#include "bitboard_concepts.hpp"
#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "fen_formatter.hpp"
#include "search.hpp"
#include "zobrist_hasher.hpp"
#include <chrono>
#include <climits>
#include <cstdint>
#include <functional>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

namespace bitcrusher {

constexpr inline int MINIMAL_TT_SIZE = 1024;

class SearchManager {

public:
    SearchManager() {
        ZobristHasher::init(std::time(nullptr));
        search_ctx_.tt.setSize(MINIMAL_TT_SIZE);
    }

    template <MoveSink MoveSinkT> void startSearch(const SearchParameters& search_options) {
        if (is_searching_) {
            stopSearch();
        }
        start_time_                = std::chrono::steady_clock::now();
        search_ctx_.nodes_searched = 0;
        stop_source_               = std::stop_source();
        is_searching_              = true;
        for (int i = 0; i < max_cores_; i++) {
            BoardState thread_board = board_; // Copy for thread safety
            active_threads_.fetch_add(1);
            workers_.emplace_back([this, search_options, thread_board]() {
                this->go<MoveSinkT>(search_options, search_ctx_, thread_board,
                                    stop_source_.get_token());
            });
        }
    }

    void changeMaxCores(int cores_limit) { max_cores_ = cores_limit; }

    void stopSearch() {
        stop_source_.request_stop();
        waitUntilSearchFinished();
        handleFinish();
    }

    void handleFinish() {
        is_searching_ = false;
        onSearchFinished_();
    }

    uint64_t getNodeCount() const { return search_ctx_.nodes_searched.load(); };

    template <MoveSink MoveSinkT>
    void go(const SearchParameters& search_parameters,
            SharedSearchContext&    search_ctx,
            BoardState              board,
            std::stop_token         st) {
        if (board.isWhiteMove()) {
            bitcrusher::search<bitcrusher::Color::WHITE, MoveSinkT>(
                search_ctx, board, search_parameters.max_depth, INT_MIN, INT_MAX, st);
        } else {
            bitcrusher::search<bitcrusher::Color::BLACK, MoveSinkT>(
                search_ctx, board, search_parameters.max_depth, INT_MIN, INT_MAX, st);
        }

        if (active_threads_.fetch_sub(1) == 1) { // Last thread completes

            if (! st.stop_requested()) {
                handleFinish();
            }
        }
    }

    void setOnSearchFinished(std::function<void()> callback) {
        onSearchFinished_ = std::move(callback);
    }

    void waitUntilSearchFinished() {
        for (auto& search_thread : workers_) {
            search_thread.join();
        }
    }

    constexpr void setPosToStartpos() { parseFEN(INITIAL_POSITION_FEN, board_); }

    void setPos(std::string_view fen) { parseFEN(fen, board_); }

    void applyUciMove(std::string_view move_uci) {
        move_processor_.applyMove(board_, moveFromUci(move_uci, board_));
    }

    std::string bestMoveUci() const {
        uint64_t start_pos_hash = ZobristHasher::createHash(board_);
        auto     entry          = search_ctx_.tt.getEntry(start_pos_hash);
        return toUci(entry.best_move);
    }

private:
    std::vector<std::jthread>                          workers_;
    std::stop_source                                   stopSignal_;
    int                                                max_cores_ = 2;
    std::atomic<int>                                   active_threads_{0};
    std::stop_source                                   stop_source_;
    std::function<void()>                              onSearchFinished_;
    SharedSearchContext                                search_ctx_;
    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    BoardState                                         board_;
    MoveProcessor                                      move_processor_;
    bool                                               is_searching_ = false;
};

} // namespace bitcrusher
