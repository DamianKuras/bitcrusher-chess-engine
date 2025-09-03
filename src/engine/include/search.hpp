#ifndef BITCRUSHER_SEARCH_HPP
#define BITCRUSHER_SEARCH_HPP

#include "board_state.hpp"
#include "concepts.hpp"
#include "evaluation.hpp"
#include "legal_move_generators/legal_moves_generator.hpp"
#include "move.hpp"
#include "move_processor.hpp"
#include "restriction_context.hpp"
#include "transposition_table.hpp"
#include <algorithm>
#include <climits>
#include <cstdint>
#include <stop_token>
#include <string>
#include <vector>
#include <zobrist_hasher.hpp>

namespace bitcrusher {
inline constexpr int CHECKMATE_BASE      = 1000000;
static constexpr int CHECKMATE_THRESHOLD = CHECKMATE_BASE - 1000;

struct SearchParameters {
    bool                     ponder{false};
    int                      white_time_ms{0};
    int                      black_time_ms{0};
    int                      white_increment_ms{0};
    int                      black_increment_ms{0};
    int                      moves_to_go{0};  // number of moves till next time control
    int                      max_depth{-1};   // search x plies only.
    int                      max_nodes{-1};   // search x nodes only
    int                      mate_in_x{0};    // search fo a mate in x moves
    int                      move_time_ms{0}; // search x mseconds
    bool                     infinite{false};
    std::vector<std::string> search_moves; // limit search only to selected moves

    void addSearchMove(std::string_view move) { search_moves.emplace_back(move); }
};

struct SharedSearchContext {
    std::atomic<std::uint64_t> nodes_searched{0ULL};
    TranspositionTable         tt;
};

// Alpha is minimum score that the maximizing player is assured of
// Beta is maximum score that the minimizing player is assured of
template <Color Side, MoveSink MoveSinkT>
int search(SharedSearchContext&                  search_ctx,
           BoardState&                           board,
           const SearchParameters&               search_parameters,
           int                                   alpha,
           int                                   beta,
           const std::stop_token&                st,
           std::chrono::steady_clock::time_point start_time,
           int                                   ply = 0) {

    if (st.stop_requested()) {
        return alpha;
    }
    // auto now         = std::chrono::steady_clock::now();
    // auto search_time = now - start_time;
    // if (Side == Color::WHITE && search_time >= search_parameters.white_time_ms) {
    // }
    int depth = search_parameters.max_depth - ply;
    ++search_ctx.nodes_searched;
    if (ply == search_parameters.max_depth ||
        (search_parameters.max_nodes > 0 &&
         search_ctx.nodes_searched >= search_parameters.max_nodes)) {
        return basicEval(board, Side);
    }
    // Mate distance pruning
    int mate_value = CHECKMATE_BASE - ply;
    if (mate_value < beta) {
        beta = mate_value;
        if (alpha >= beta) {
            return mate_value;
        }
    }

    mate_value = -(CHECKMATE_BASE - ply);
    if (mate_value > alpha) {
        alpha = mate_value;
        if (alpha >= beta) {
            return mate_value;
        }
    }
    uint64_t                zobrist_key  = ZobristHasher::createHash(board);
    TranspositionTableEntry stored_entry = search_ctx.tt.getEntry(zobrist_key);
    int stored_value = search_ctx.tt.getPositionEvaluation(depth, alpha, beta, zobrist_key);
    if (stored_value != NOT_FOUND_IN_TRANSPOSITION_TABLE) {
        return stored_value;
    }
    int  alpha_orig = alpha;
    int  best_score = INT_MIN;
    Move best_move  = Move::none();

    RestrictionContext restriction_context;
    updateRestrictionContext<Side>(board, restriction_context);

    MoveSinkT sink;
    generateLegalMoves<Side>(board, restriction_context, sink);

    if (sink.empty()) {
        if (restriction_context.check_count > 0) {
            return -(CHECKMATE_BASE - ply); // Lower depth mates should have higher scores
        }
        return 0; // Stalemate
    }

    MoveProcessor move_processor;

    for (int i = 0; i < sink.count; i++) {
        move_processor.applyMove(board, sink.moves[i]);
        // -search because the opponent's best score becomes your worst
        int eval = -search<! Side, MoveSinkT>(search_ctx, board, search_parameters, -beta, -alpha,
                                              st, start_time, ply + 1);
        move_processor.undoMove(board, sink.moves[i]);

        if (eval > best_score) {
            best_score = eval;
            best_move  = sink.moves[i];

            alpha = std::max(eval, alpha);
        }
        if (eval >= beta) {
            best_score = eval;
            best_move  = sink.moves[i];
            break; // Beta cutoff
        }
    }

    TranspositionTableEvaluationType flag = TranspositionTableEvaluationType::EXACT_VALUE;
    if (best_score <= alpha_orig) {
        flag = TranspositionTableEvaluationType::AT_BEST;
    } else if (best_score >= beta) {
        flag = TranspositionTableEvaluationType::AT_LEAST;
    }

    if (! best_move.isNullMove()) {
        search_ctx.tt.store(depth, best_score, flag, zobrist_key, best_move);
    }
    return best_score;
}

} // namespace bitcrusher

#endif // BITCRUSHER_SEARCH_HPP