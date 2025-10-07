#ifndef BITCRUSHER_SEARCH_MANAGER_HPP
#define BITCRUSHER_SEARCH_MANAGER_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "concepts.hpp"
#include "fen_formatter.hpp"
#include "move.hpp"
#include "move_processor.hpp"
#include "move_sink.hpp"
#include "restriction_context.hpp"
#include "search.hpp"
#include "transposition_table.hpp"
#include <chrono>
#include <climits>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>

namespace bitcrusher {

class SearchManager {
public:
    SearchManager() {
        ZobristKeys::init(12345);

        // Create persistent main worker thread.
        main_search_thread_ = std::thread([this]() { this->workerThreadMain(); });
    }

    SearchManager(const SearchManager&)            = delete;
    SearchManager(SearchManager&&)                 = delete;
    SearchManager& operator=(const SearchManager&) = delete;
    SearchManager& operator=(SearchManager&&)      = delete;

    ~SearchManager() {
        {
            shutdown_ = true;
            condition_.notify_all();
        }

        if (main_search_thread_.joinable()) {
            main_search_thread_.join();
        }
        for (auto& thread : workers_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    template <MoveSink MoveSinkT> void startSearch(const SearchParameters& search_parameters) {

        if (search_active_.load()) {
            stopSearch();
            waitUntilSearchFinished();
        }
        search_ctx_.tt.clear();
        // Update search parameters and state with lock.
        {
            std::lock_guard<std::mutex> lock(mutex_);
            search_options_            = search_parameters;
            start_time_                = std::chrono::steady_clock::now();
            search_ctx_.nodes_searched = 0;
            stop_source_               = std::stop_source();
            search_active_             = true;
            // Calculate move time allocation.
            if (board_.isWhiteMove()) {
                search_time_ms_ = calculateMoveTimeAllocation<Color::WHITE>(search_parameters);

            } else {
                search_time_ms_ = calculateMoveTimeAllocation<Color::BLACK>(search_parameters);
            }
        }
        condition_.notify_all(); // Notifies main and all worker threads.
    }

    void waitUntilSearchFinished() {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return ! search_active_.load(); });
    }

    void stopSearch() {
        if (! search_active_.load()) {
            return;
        }
        stop_source_.request_stop();
    }

    [[nodiscard]] uint64_t getNodeCount() const { return search_ctx_.nodes_searched.load(); }

    [[nodiscard]] std::chrono::time_point<std::chrono::steady_clock> getSearchStartTime() const {
        return start_time_;
    }

    template <Color Side>
    int calculateMoveTimeAllocation(const SearchParameters& search_parameters) {
        if (search_parameters.move_time_ms > 0) {
            return search_parameters.move_time_ms;
        }
        if (search_parameters.infinite ||
            (search_parameters.move_time_ms == 0 && search_parameters.white_time_ms == 0 &&
             search_parameters.black_time_ms == 0)) {
            return INT_MAX;
        }

        int time_available = (Side == Color::WHITE) ? search_parameters.white_time_ms
                                                    : search_parameters.black_time_ms;
        int increment      = (Side == Color::WHITE) ? search_parameters.white_increment_ms
                                                    : search_parameters.black_increment_ms;

        const int time_divisor      = 20;
        const int increment_divisor = 2;
        int time_allocated = (time_available / time_divisor) + (increment / increment_divisor);

        return std::max(1, time_allocated);
    }

    void setOnSearchFinished(const std::function<void()>& callback) {
        onSearchFinished_ = callback;
    }

    void setOnDepthCompleted(const std::function<void(int)>& callback) {
        onDepthCompleted_ = callback;
    }

    constexpr void setPosToStartpos() { parseFEN(INITIAL_POSITION_FEN, board_); }

    inline void setHashMBSize(int size) { search_ctx_.tt.setMBSize(size); }

    void setPos(std::string_view fen) { parseFEN(fen, board_); }

    void applyUciMove(std::string_view move_uci) {
        move_processor_.applyMove(board_, moveFromUci(move_uci, board_));
    }

    std::string bestMoveUci() { return toUci(best_move_); }

    std::string getPrincipalVariation(int depth) {
        BoardState  pv_board = board_;
        std::string ans      = toUci(best_move_);

        MoveProcessor mp;

        mp.applyMove(pv_board, best_move_);
        int max_remaining_moves_in_pv = (depth * 2) - 1;
        while (max_remaining_moves_in_pv-- > 0) {
            uint64_t hash            = pv_board.getZobristHash();
            auto     best_move_entry = search_ctx_.tt.getEntry(hash);
            if (best_move_entry.value == NOT_FOUND_IN_TRANSPOSITION_TABLE ||
                best_move_entry.best_move.isNullMove() || best_move_entry.key != hash ||
                best_move_entry.depth < 0) {
                break;
            }

            ans += " " + toUci(best_move_entry.best_move);
            mp.applyMove(pv_board, best_move_entry.best_move);
        }
        return ans;
    }

    std::string getScore() const {
        if (std::abs(score_) >= CHECKMATE_THRESHOLD) {
            int mate_distance = CHECKMATE_BASE - std::abs(score_);
            int moves_to_mate = (mate_distance + 1) / 2;
            return "mate " + std::to_string(moves_to_mate);
        }

        return "cp " + std::to_string(score_);
    }

    void resetMoveProcessor() { move_processor_.resetHistory(); }

    void newGame() {
        move_processor_.resetHistory();
        search_ctx_.tt.clear();
    }
#ifdef DEBUG
    int getBetaCutoffs() { return search_ctx_.beta_cutoffs.load(); }

    int getTTCutoffs() { return search_ctx_.tt_cutoffs.load(); }

    double getTTUsage() const { return search_ctx_.tt.getUsedPercentage(); }
#endif
    void setDebug(bool value) { debug_ = value; }

    void setMaxCores(int cores) {
        max_cores_ = cores;
        shutdownWorkers();
        for (int i = 0; i < max_cores_ - 1; ++i) {
            workers_.emplace_back([this]() { this->workerThread(); });
        }
    }

private:
    void workerThreadMain() {
        while (true) {
            // Wait for work or shutdown signal.
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] { return search_active_.load() || shutdown_; });

            if (shutdown_) {
                break; // Exit thread.
            }

            // Copy necessary data for thread-safe access.
            SearchParameters     local_options         = search_options_;
            BoardState           thread_board          = board_;
            auto                 stop_token            = stop_source_.get_token();
            MoveProcessor        thread_move_processor = move_processor_;
            auto                 thread_start_time     = start_time_;
            int                  thread_search_time_ms = search_time_ms_;
            SharedSearchContext& ctx                   = search_ctx_;
            lock.unlock();

            performSearch<FastMoveSink, true>(local_options, thread_board, thread_move_processor,
                                              stop_token, thread_start_time, thread_search_time_ms,
                                              ctx);
            handleSearchFinished();
        }
    }

    void workerThread() {
        while (true) {
            // Wait for work or shutdown signal.
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] {
                return search_active_.load() || shutdown_.load() || workers_shutdown_.load();
            });

            if (shutdown_ || workers_shutdown_) {
                break; // Exit thread.
            }

            // Copy necessary data for thread-safe access.
            SearchParameters     local_options         = search_options_;
            BoardState           thread_board          = board_;
            auto                 stop_token            = stop_source_.get_token();
            MoveProcessor        thread_move_processor = move_processor_;
            auto                 thread_start_time     = start_time_;
            int                  thread_search_time_ms = search_time_ms_;
            SharedSearchContext& ctx                   = search_ctx_;
            lock.unlock();

            performSearch<FastMoveSink>(local_options, thread_board, thread_move_processor,
                                        stop_token, thread_start_time, thread_search_time_ms, ctx);

            // Wait for main thread to finish searching.
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this] {
                    return ! search_active_.load() || shutdown_.load() || workers_shutdown_.load();
                });
                if (shutdown_ || workers_shutdown_) {
                    break;
                }
            }
        }
    }

    void shutdownWorkers() {
        if (workers_.size() == 0) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            workers_shutdown_ = true;
            condition_.notify_all();
        }
        for (auto& thread : workers_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        workers_.clear();
        workers_shutdown_ = false; // Reset for future searches.
    }

    template <MoveSink MoveSinkT, bool IsMainThread = false>
    void performSearch(const SearchParameters&                            search_parameters,
                       BoardState                                         board,
                       MoveProcessor                                      move_processor,
                       std::stop_token&                                   st,
                       std::chrono::time_point<std::chrono::steady_clock> start_time,
                       int                                                search_time_ms,
                       SharedSearchContext&                               search_ctx) {
        FastMoveSink       sink;
        RestrictionContext restriction_context;
        for (int depth = 1; depth <= search_parameters.max_depth; depth++) {

            int score{0};
            if (board.isWhiteMove()) {
                score = bitcrusher::search<Color::WHITE>(search_ctx, board, move_processor,
                                                         search_parameters, restriction_context,
                                                         depth * 2, -CHECKMATE_BASE, CHECKMATE_BASE,
                                                         st, start_time, search_time_ms, sink);
            } else {
                score = bitcrusher::search<Color::BLACK>(search_ctx, board, move_processor,
                                                         search_parameters, restriction_context,
                                                         depth * 2, -CHECKMATE_BASE, CHECKMATE_BASE,
                                                         st, start_time, search_time_ms, sink);
            }
            if constexpr (IsMainThread) {
                if (abs(score) != SEARCH_INTERRUPTED) {
                    assert(abs(score) != ON_EVALUATION);
                    TranspositionTableEntry root_move_entry =
                        search_ctx_.tt.getEntry(board.getZobristHash());
                    assert(! root_move_entry.best_move.isNullMove());
                    best_move_ = root_move_entry.best_move;
                    score_     = score;
                    if (onDepthCompleted_) {
                        onDepthCompleted_(depth);
                    }
                } else {
                    break;
                }
            }
        }
    }

    void handleSearchFinished() {
        stop_source_.request_stop();
        if (onSearchFinished_) {
            onSearchFinished_();
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            search_active_ = false;
            condition_.notify_all();
        }
    }

    std::thread              main_search_thread_;
    std::vector<std::thread> workers_;
    std::atomic<bool>        shutdown_{false};
    std::atomic<bool>        workers_shutdown_{false};
    std::atomic<bool>        search_active_{false};

    std::mutex              mutex_;
    std::condition_variable condition_;

    std::stop_source                                   stop_source_;
    SearchParameters                                   search_options_;
    SharedSearchContext                                search_ctx_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    int                                                search_time_ms_{};

    BoardState    board_{};
    MoveProcessor move_processor_;

    std::function<void()>    onSearchFinished_;
    std::function<void(int)> onDepthCompleted_;

    Move best_move_;
    int  score_{0};
    bool debug_{false};

    int max_cores_{1};
};

} // namespace bitcrusher

#endif // BITCRUSHER_SEARCH_MANAGER_HPP
