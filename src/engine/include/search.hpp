#ifndef BITCRUSHER_SEARCH_HPP
#define BITCRUSHER_SEARCH_HPP

#include "board_state.hpp"
#include "concepts.hpp"
#include "evaluation.hpp"
#include "legal_move_generators/legal_moves_generator.hpp"
#include "legal_move_generators/shared_move_generation.hpp"
#include "move.hpp"
#include "move_processor.hpp"
#include "restriction_context.hpp"
#include "transposition_table.hpp"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <stop_token>
#include <string>
#include <vector>

namespace bitcrusher {

inline constexpr int SEARCH_INTERRUPTED  = 987654321;
inline constexpr int CHECKMATE_BASE      = 1000000;
inline constexpr int CHECKMATE_THRESHOLD = CHECKMATE_BASE - 1000;
inline constexpr int NODE_CHECK_INTERVAL = 1023;

struct SearchParameters {
    bool ponder{false};
    int  white_time_ms{0};
    int  black_time_ms{0};
    int  white_increment_ms{0};
    int  black_increment_ms{0};
    int  moves_to_go{0};  // Number of moves till next time control.
    int  max_depth{300};  // Search max depth.
    int  max_nodes{-1};   // Search x nodes only.
    int  mate_in_x{0};    // Search fo a mate in x moves.
    int  move_time_ms{0}; // Search x mseconds.
    bool infinite{false};

    std::vector<std::string> search_moves; // Limit search only to selected moves.

    void addSearchMove(std::string_view move) { search_moves.emplace_back(move); }
};

struct SharedSearchContext {
    std::atomic<std::uint64_t> nodes_searched{0ULL};
    TranspositionTable         tt;

#ifdef DEBUG
    std::atomic<int> beta_cutoffs{0};
    std::atomic<int> tt_cutoffs{0};
#endif
};

inline bool shouldStopSearching(const std::stop_token&                st,
                                std::chrono::steady_clock::time_point start_time,
                                int                                   max_search_time_ms,
                                uint64_t                              nodes_searched) {
    if (st.stop_requested()) {
        return true;
    }

    if ((nodes_searched & NODE_CHECK_INTERVAL) == 0) {
        auto now = std::chrono::steady_clock::now();
        auto search_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
        return search_time > max_search_time_ms;
    }
    return false;
}

template <Color Side, MoveSink MoveSinkT>
int quiescenceSearch(SharedSearchContext&                  search_ctx,
                     BoardState&                           board,
                     MoveProcessor&                        move_processor,
                     RestrictionContext&                   restriction_context,
                     int                                   alpha,
                     int                                   beta,
                     const std::stop_token&                st,
                     std::chrono::steady_clock::time_point start_time,
                     int                                   max_search_time_ms,
                     MoveSinkT&                            sink,
                     int                                   ply) {
    if (move_processor.hasCurrentPositionRepeated3Times()) {
        return 0;
    }
    if (shouldStopSearching(st, start_time, max_search_time_ms, search_ctx.nodes_searched.load())) {
        return SEARCH_INTERRUPTED;
    }

    updateRestrictionContext<Side>(board, restriction_context);

    // Generate appropriate moves based on check state.
    if (restriction_context.check_count > 0) { // In check.
        generateLegalMoves<Side, MoveGenerationPolicy::FULL, RestrictionContextPolicy::LEAVE>(
            board, sink, restriction_context, ply);
        if (sink.count[ply] == 0) { // No legal moves and in check.
            return -CHECKMATE_BASE;
        }
    } else {
        generateLegalMoves<Side, MoveGenerationPolicy::CAPTURES_ONLY,
                           RestrictionContextPolicy::LEAVE>(board, sink, restriction_context, ply);
    }

    int static_eval = basicEval(board, Side);

    if (sink.count[ply] == 0) { // No legal Captures or max depth.
        return static_eval;
    }

    int best_score = static_eval;

    // Stand Pat.
    if (best_score >= beta) {
        return best_score;
    }
    alpha = std::max(best_score, alpha);

    for (int i = 0; i < sink.count[ply]; i++) {
        Move move = sink.moves[ply][i];
        move_processor.applyMove(board, move);

        assert((board.getOwnOccupancy<Side>() ^ board.getOpponentOccupancy<Side>()) ==
               board.getAllOccupancy());
        // -Search because the opponent's best score becomes your worst.
        int eval = -quiescenceSearch<! Side, MoveSinkT>(
            search_ctx, board, move_processor, restriction_context, -beta, -alpha, st, start_time,
            max_search_time_ms, sink, ply + 1);

        move_processor.undoMove(board, move);
        if (abs(eval) == SEARCH_INTERRUPTED) {
            return SEARCH_INTERRUPTED;
        }
        if (eval > best_score) {
            best_score = eval;
            alpha      = std::max(eval, alpha);
        }
        if (alpha >= beta) {
            break; // Beta cutoff.
        }
    }
    return best_score;
}

// Alpha is minimum score that the maximizing player is assured of.
// Beta is maximum score that the minimizing player is assured of.
template <Color Side, MoveSink MoveSinkT>
int search(SharedSearchContext&                  search_ctx,
           BoardState&                           board,
           MoveProcessor&                        move_processor,
           const SearchParameters&               search_parameters,
           RestrictionContext&                   restriction_context,
           int                                   depth,
           int                                   alpha,
           int                                   beta,
           std::stop_token&                      st,
           std::chrono::steady_clock::time_point start_time,
           int                                   max_search_time_ms,
           MoveSinkT&                            sink,
           int                                   ply       = 0,
           bool                                  exclusive = false) {
    int alpha_orig = alpha;
    if (move_processor.hasCurrentPositionRepeated3Times()) {
        return 0; // Draw.
    }

    if (shouldStopSearching(st, start_time, max_search_time_ms, search_ctx.nodes_searched.load())) {
        return SEARCH_INTERRUPTED;
    }
    // Transposition table cutoff.
    uint64_t                zobrist_key  = board.getZobristHash();
    TranspositionTableEntry stored_entry = search_ctx.tt.getEntry(zobrist_key);
    if (stored_entry.key == zobrist_key && search_ctx.tt.isSearched(zobrist_key) && exclusive) {
        return ON_EVALUATION;
    }
    if (stored_entry.key == zobrist_key && stored_entry.depth >= depth) {

        if (stored_entry.evaluation_type == TranspositionTableEvaluationType::EXACT_VALUE) {
#ifdef DEBUG
            search_ctx.tt_cutoffs.fetch_add(1, std::memory_order_relaxed);
#endif
            return stored_entry.value;
        }
        if (stored_entry.evaluation_type == TranspositionTableEvaluationType::LOWERBOUND &&
            stored_entry.value >= beta) {
#ifdef DEBUG
            search_ctx.tt_cutoffs.fetch_add(1, std::memory_order_relaxed);
#endif
            return stored_entry.value;
        }
        if (stored_entry.evaluation_type == TranspositionTableEvaluationType::UPPERBOUND &&
            stored_entry.value <= alpha) {
#ifdef DEBUG
            search_ctx.tt_cutoffs.fetch_add(1, std::memory_order_relaxed);
#endif
            return stored_entry.value;
        }
    }

    // Mate distance pruning.
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

    generateLegalMoves<Side>(board, sink, restriction_context, ply);

    // Check if side to move is mated.
    if (sink.count[ply] == 0) { // No legal moves.
        if (restriction_context.check_count > 0) {

            int mate_score = CHECKMATE_BASE - ply; // Lower depth mates should have higher scores.
            return -mate_score;
        }
        return 0; // Stalemate no legal moves and not in check.
    }

    if (depth == 0 || (search_parameters.max_nodes > 0 &&
                       search_ctx.nodes_searched >= search_parameters.max_nodes)) {
        int eval = quiescenceSearch<Side, MoveSinkT>(search_ctx, board, move_processor,
                                                     restriction_context, -beta, -alpha, st,
                                                     start_time, max_search_time_ms, sink, ply + 1);
        return eval;
    }

    // Move ordering.
    if (stored_entry.key == zobrist_key && stored_entry.depth > 0) {
        for (int i = 0; i < sink.count[ply]; ++i) {
            if (sink.moves[ply][i] == stored_entry.best_move) {
                std::swap(sink.moves[ply][0], sink.moves[ply][i]);
                break;
            }
        }
    }

    // Search the node.
    search_ctx.tt.addSearched(zobrist_key);

    search_ctx.nodes_searched.fetch_add(1, std::memory_order_relaxed);
    int               best_score = -CHECKMATE_BASE;
    Move              best_move  = Move::none();
    std::vector<bool> needs_search(sink.count[ply], true);
    bool              all_done = false;
    for (int iteration = 0; iteration < 2 && ! all_done; iteration++) {
        all_done    = true;
        int start_i = 0;
        int end_i   = sink.count[ply];
        for (int i = start_i; i < end_i; i++) {
            if (! needs_search[i]) {
                // Skip moves already fully evaluated in a prior pass.
                continue;
            }

            Move move = sink.moves[ply][i];
            move_processor.applyMove(board, move);
            bool exclusive = iteration == 0 && i != 0;
            int  eval      = -search<! Side, MoveSinkT>(search_ctx, board, move_processor,
                                                        search_parameters, restriction_context, depth - 1,
                                                        -beta, -alpha, st, start_time, max_search_time_ms,
                                                        sink, ply + 1, exclusive);
            move_processor.undoMove(board, move);
            if (abs(eval) == SEARCH_INTERRUPTED) {
                return SEARCH_INTERRUPTED;
            }
            if (abs(eval) == ON_EVALUATION) {
                // Other thead is working on this move already.
                all_done = false;
                continue;
            }
            // Mark this move as done for subsequent passes.
            needs_search[i] = false;

            if (eval > best_score) {
                best_score = eval;
                best_move  = move;
                alpha      = std::max(eval, alpha);
            }
            if (alpha >= beta) {
#ifdef DEBUG
                search_ctx.beta_cutoffs.fetch_add(1, std::memory_order_relaxed);
#endif
                all_done = true; // No need to search more nodes.
                break;           // Beta cutoff.
            }
        }
    }

    search_ctx.tt.removeSearched(zobrist_key);
    stored_entry.value     = best_score;
    stored_entry.best_move = best_move;
    stored_entry.depth     = depth;
    stored_entry.key       = zobrist_key;
    if (best_score <= alpha_orig) {
        stored_entry.evaluation_type = TranspositionTableEvaluationType::UPPERBOUND;
    } else if (best_score >= beta) {
        stored_entry.evaluation_type = TranspositionTableEvaluationType::LOWERBOUND;
    } else {
        stored_entry.evaluation_type = TranspositionTableEvaluationType::EXACT_VALUE;
    }
    search_ctx.tt.store(zobrist_key, stored_entry);
    return best_score;
}

} // namespace bitcrusher

#endif // BITCRUSHER_SEARCH_HPP