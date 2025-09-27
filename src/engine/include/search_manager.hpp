#ifndef BITCRUSHER_SEARCH_MANAGER_HPP
#define BITCRUSHER_SEARCH_MANAGER_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "concepts.hpp"
#include "fen_formatter.hpp"
#include "move.hpp"
#include "move_processor.hpp"
#include "move_sink.hpp"
#include "search.hpp"
#include "transposition_table.hpp"
#include "zobrist_hasher.hpp"
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
        ZobristHasher::init(std::time(nullptr));

        // Create persistent main worker thread.
        worker_ = std::thread([this]() { this->workerThreadMain(); });
    }

    SearchManager(const SearchManager&)            = delete;
    SearchManager(SearchManager&&)                 = delete;
    SearchManager& operator=(const SearchManager&) = delete;
    SearchManager& operator=(SearchManager&&)      = delete;

    ~SearchManager() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
            condition_.notify_one();
        }

        if (worker_.joinable()) {
            worker_.join();
        }
    }

    template <MoveSink MoveSinkT> void startSearch(const SearchParameters& search_parameters) {
        search_ctx_.tt.clear();

        if (search_active_.load()) {
            stopSearch();
        }

        // Update search parameters and state with lock.
        {
            std::lock_guard<std::mutex> lock(mutex_);
            search_options_            = search_parameters;
            start_time_                = std::chrono::steady_clock::now();
            search_ctx_.nodes_searched = 0;
            stop_source_               = std::stop_source();
            search_active_             = true;
        }
        // Calculate move time allocation.
        if (board_.isWhiteMove()) {
            search_time_ms_ = calculateMoveTimeAllocation<Color::WHITE>(search_parameters);

        } else {
            search_time_ms_ = calculateMoveTimeAllocation<Color::BLACK>(search_parameters);
        }

        condition_.notify_one(); // Wake search thread
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

        const int time_divisor      = 10;
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

    inline void setPosToStartpos() { parseFEN(INITIAL_POSITION_FEN, board_); }

    inline void setHashMBSize(int size) { search_ctx_.tt.setMBSize(size); }

    void setPos(std::string_view fen) { parseFEN(fen, board_); }

    void applyUciMove(std::string_view move_uci) {
        move_processor_.applyMove(board_, moveFromUci(move_uci, board_));
    }

    std::string bestMoveUci() { return toUci(best_move_); }

    std::string getPrincipalVariation(int depth) {
        BoardState  pv_board = board_;
        std::string ans;

        MoveProcessor mp;
        int           max_moves_in_pv = depth * 2;
        ans += " " + toUci(best_move_);
        mp.applyMove(pv_board, best_move_);

        while (max_moves_in_pv-- > 0) {
            uint64_t hash            = ZobristHasher::createHash(pv_board);
            auto     best_move_entry = search_ctx_.tt.getEntry(hash);
            if (best_move_entry.value == NOT_FOUND_IN_TRANSPOSITION_TABLE ||
                best_move_entry.best_move.isNullMove() || best_move_entry.key != hash) {
                break;
            }

            ans += " " + toUci(best_move_entry.best_move);
            mp.applyMove(pv_board, best_move_entry.best_move);
        }
        return ans;
    }

    std::string getScore() {
        uint64_t start_pos_hash = ZobristHasher::createHash(board_);
        auto     entry          = search_ctx_.tt.getEntry(start_pos_hash);
        int      score          = entry.value;

        if (std::abs(score) >= CHECKMATE_THRESHOLD) {
            int mate_distance = CHECKMATE_BASE - std::abs(score);
            int moves_to_mate = (mate_distance + 1) / 2;
            return "mate " + std::to_string(moves_to_mate);
        }

        return std::to_string(score);
    }

    void resetMoveProcessor() { move_processor_.resetHistory(); }
#ifdef DEBUG
    int getBetaCutoffs() { return search_ctx_.beta_cutoffs.load(); }

    int getTTCutoffs() { return search_ctx_.tt_cutoffs.load(); }
#endif
private:
    void workerThreadMain() {
        while (true) {
            // Wait for work or shutdown signal
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] { return search_active_.load() || shutdown_; });

            if (shutdown_) {
                break; // Exit thread
            }

            // Copy necessary data for thread-safe access
            SearchParameters local_options = search_options_;
            BoardState       thread_board  = board_;
            auto             stop_token    = stop_source_.get_token();
            lock.unlock();

            performSearch<FastMoveSink>(local_options, thread_board, stop_token, true);
            handleSearchFinished();
        }
    }

    template <MoveSink MoveSinkT>
    void performSearch(const SearchParameters& search_parameters,
                       BoardState              board,
                       const std::stop_token&  st,
                       bool                    is_main_thread = false) {

        for (int depth = 1; depth <= search_parameters.max_depth; depth++) {

            int score = 0;
            if (board.isWhiteMove()) {
                score = bitcrusher::search<Color::WHITE, MoveSinkT>(
                    search_ctx_, board, search_parameters, depth * 2, -CHECKMATE_BASE,
                    CHECKMATE_BASE, st, start_time_, search_time_ms_);
            } else {
                score = bitcrusher::search<Color::BLACK, MoveSinkT>(
                    search_ctx_, board, search_parameters, depth * 2, -CHECKMATE_BASE,
                    CHECKMATE_BASE, st, start_time_, search_time_ms_);
            }

            if (is_main_thread) {
                auto now = std::chrono::steady_clock::now();
                auto search_time =
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_)
                        .count();
                if (search_time > search_time_ms_) {
                    stop_source_.request_stop();
                    return;
                }
                if (st.stop_requested()) {
                    return;
                }
                uint64_t root_hash       = ZobristHasher::createHash(board_);
                auto     root_move_entry = search_ctx_.tt.getEntry(root_hash);
                best_move_               = root_move_entry.best_move;
                if (onDepthCompleted_) {
                    onDepthCompleted_(depth);
                }
            }
        }
    }

    void handleSearchFinished() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            search_active_ = false;
            condition_.notify_one();
        }

        if (onSearchFinished_) {
            onSearchFinished_();
        }
    }

    std::thread       worker_;
    std::atomic<bool> shutdown_{false};
    std::atomic<bool> search_active_{false};

    std::mutex              mutex_;
    std::condition_variable condition_;

    std::stop_source                                   stop_source_;
    SearchParameters                                   search_options_;
    SharedSearchContext                                search_ctx_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    int                                                search_time_ms_{};

    BoardState    board_;
    MoveProcessor move_processor_;

    std::function<void()>    onSearchFinished_;
    std::function<void(int)> onDepthCompleted_;

    Move best_move_ = Move::none();
};

} // namespace bitcrusher

#endif // BITCRUSHER_SEARCH_MANAGER_HPP
