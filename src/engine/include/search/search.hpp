#ifndef BITCRUSHER_SEARCH_HPP
#define BITCRUSHER_SEARCH_HPP

#include "board_state.hpp"
#include "concepts.hpp"
#include "evaluation.hpp"
#include "heuristics/heuristics.hpp"
#include "legal_move_generators/legal_moves_generator.hpp"
#include "legal_move_generators/shared_move_generation.hpp"
#include "move.hpp"
#include "move_processor.hpp"
#include "restriction_context.hpp"
#include "transposition_table.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_set>
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
    int  max_ply{300};    // Search max depth.
    int  max_nodes{-1};   // Search x nodes only.
    int  mate_in_x{0};    // Search fo a mate in x moves.
    int  move_time_ms{0}; // Search x mseconds.
    bool infinite{false};
    bool use_quiescence_search{true};

    std::unordered_set<std::string> search_moves; // Limit search only to selected moves.

    void addSearchMove(const std::string& move) { search_moves.insert(move); }
};

struct SharedSearchContext {
    std::atomic<std::uint64_t> nodes_searched{0ULL};
    TranspositionTable         tt;

    std::atomic<bool>    is_pondering{false};
    std::atomic<int64_t> time_limit_start_ms{0};
    std::atomic<int>     max_search_time_ms{0};

    // Best move found at the root so far. Written only by the main thread
    // (IsRoot=true). Guarantees a legal move is always available even if
    // iterative deepening is interrupted before depth 1 completes.
    Move root_best_move{Move::none()};

#ifdef DEBUG
    std::atomic<int> beta_cutoffs{0};
    std::atomic<int> tt_cutoffs{0};
#endif
};

template <typename CtxT>
inline bool shouldStopSearching(const std::stop_token& st, CtxT& search_ctx) {
    if (st.stop_requested()) {
        return true;
    }

    if ((search_ctx.nodes_searched.load() & NODE_CHECK_INTERVAL) == 0 &&
        ! search_ctx.is_pondering.load()) {
        auto now = std::chrono::steady_clock::now();
        auto current_time_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        const int max_time = search_ctx.max_search_time_ms.load();

        if (max_time > 0 && (current_time_ms - search_ctx.time_limit_start_ms.load()) > max_time) {
            return true;
        }
    }
    return false;
}

template <Color Side, SearchConfig Config = DEFAULT_CONFIG, MoveSink MoveSinkT, typename CtxT>
int quiescenceSearch(CtxT&                  search_ctx,
                     BoardState&            board,
                     MoveProcessor&         move_processor,
                     RestrictionContext&    restriction_context,
                     int                    alpha,
                     int                    beta,
                     const std::stop_token& st,
                     MoveSinkT&             sink,
                     int                    ply) {
    if (move_processor.hasPositionRepeatedOnPath()) {
        return 0;
    }
    if (board.getHalfmoveClock() >= 100) {
        return 0; // Fifty-move rule draw.
    }
    if (shouldStopSearching(st, search_ctx)) {
        return SEARCH_INTERRUPTED;
    }
    search_ctx.nodes_searched.fetch_add(1, std::memory_order::relaxed);

    updateRestrictionContext<Side>(board, restriction_context);

    // Generate appropriate moves based on check state.
    if (restriction_context.check_count > 0) { // In check.
        generateLegalMoves<Side, MoveGenerationPolicy::COMPETITIVE_FULL,
                           RestrictionContextUpdatePolicy::LEAVE>(board, sink, restriction_context,
                                                                  ply);
        if (sink.count[ply] == 0) { // No legal moves and in check.
            return -CHECKMATE_BASE;
        }
    } else { // Not in check.
        generateLegalMoves<Side, MoveGenerationPolicy::COMPETITIVE_CAPTURES_ONLY,
                           RestrictionContextUpdatePolicy::LEAVE>(board, sink, restriction_context,
                                                                  ply);
    }

    int static_eval = eval(board, Side);

    if (sink.count[ply] == 0) { // No legal captures or max depth.
        return static_eval;
    }

    int best_score = static_eval;

    // Stand pat.
    if (best_score >= beta) {
        return best_score;
    }
    alpha = std::max(best_score, alpha);

    heuristics::scoreAndSortQuiescence<Config>(sink, ply);

    for (int i = 0; i < sink.count[ply]; i++) {
        Move move = sink.moves[ply][i];
        move_processor.applyMove(board, move);

        assert((board.getOwnOccupancy<Side>() ^ board.getOpponentOccupancy<Side>()) ==
               board.getAllOccupancy());
        // -Search because the opponent's best score becomes your worst.
        int score = -quiescenceSearch<! Side, Config>(search_ctx, board, move_processor,
                                                      restriction_context, -beta, -alpha, st, sink,
                                                      ply + 1);

        move_processor.undoMove(board, move);
        if (abs(score) == SEARCH_INTERRUPTED) {
            return SEARCH_INTERRUPTED;
        }
        if (score > best_score) {
            best_score = score;
            alpha      = std::max(score, alpha);
        }
        if (alpha >= beta) {
            break; // Beta cutoff.
        }
    }
    return best_score;
}

/// @brief Alpha is minimum score that the maximizing player is assured of.
/// Beta is maximum score that the minimizing player is assured of.
template <Color        Side,
          SearchConfig Config             = DEFAULT_CONFIG,
          bool         IsRoot             = false,
          bool         PauseAfterRootSort = false,
          MoveSink     MoveSinkT,
          typename CtxT>
int search(CtxT&                   search_ctx,
           BoardState&             board,
           MoveProcessor&          move_processor,
           const SearchParameters& search_parameters,
           RestrictionContext&     restriction_context,
           int                     depth,
           int                     alpha,
           int                     beta,
           std::stop_token&        st,
           MoveSinkT&              sink,
           int                     ply       = 0,
           bool                    exclusive = false) {
    int alpha_orig = alpha;
    if constexpr (IsRoot) {
        if (move_processor.hasCurrentPositionRepeated3Times()) {
            return 0; // Draw.
        }
    } else {
        if (move_processor.hasPositionRepeatedOnPath()) {
            return 0; // Draw.
        }
    }
    if (board.getHalfmoveClock() >= 100) {
        return 0; // Fifty-move rule draw.
    }

    // For the test hook path (IsRoot && PauseAfterRootSort), skip the early stop
    // check so root moves are always generated and sorted before stopping.
    if constexpr (! (IsRoot && PauseAfterRootSort)) {
        if (shouldStopSearching(st, search_ctx)) {
            return SEARCH_INTERRUPTED;
        }
    }

    // Transposition table cutoff.
    uint64_t                zobrist_key  = board.getZobristHash();
    TranspositionTableEntry stored_entry = search_ctx.tt.getEntry(zobrist_key);
    if (stored_entry.key == zobrist_key && search_ctx.tt.isSearched(zobrist_key) && exclusive) {
        return ON_EVALUATION;
    }
    if (stored_entry.key == zobrist_key && stored_entry.depth >= depth) {
        // At root, only cut off when the TT move is valid (piece on from-square).
        // An invalid move signals a hash collision: fall through to the full search so
        // root_best_move is always set to a legal move rather than Move::none().
        bool do_tt_cutoff = true;
        if constexpr (IsRoot) {
            bool tt_move_valid = ! stored_entry.best_move.isNullMove() &&
                                 (board.getOwnOccupancy<Side>() &
                                  (1ULL << static_cast<int>(stored_entry.best_move.fromSquare())));
            if (tt_move_valid && search_parameters.search_moves.empty()) {
                search_ctx.root_best_move = stored_entry.best_move;
            }
            do_tt_cutoff = tt_move_valid;
        }
        if (do_tt_cutoff) {
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
    }

    // Mate distance pruning.
    if (auto score = heuristics::applyMateDistancePruning(CHECKMATE_BASE, ply, alpha, beta)) {
        return *score;
    }

    generateLegalMoves<Side, MoveGenerationPolicy::COMPETITIVE_FULL>(board, sink,
                                                                     restriction_context, ply);

    // Check if side to move is mated or stalemated.
    if (sink.count[ply] == 0) {
        if (restriction_context.check_count > 0) {
            return -(CHECKMATE_BASE - ply); // Lower depth mates have higher scores.
        }
        return 0; // Stalemate.
    }

    // At leaf or node budget exhausted.
    // Unlike SEARCH_INTERRUPTED, this is a normal termination, the returned score
    // is real and contributes to the search result.
    bool at_leaf_or_node_budget_exhausted =
        depth == 0 || (search_parameters.max_nodes > 0 &&
                       search_ctx.nodes_searched >= search_parameters.max_nodes);

    if (at_leaf_or_node_budget_exhausted) {
        if constexpr (Config.quiescence.enabled) {
            return quiescenceSearch<Side, Config>(search_ctx, board, move_processor,
                                                  restriction_context, alpha, beta, st, sink,
                                                  ply + 1);
        }
        return eval(board, Side);
    }

    Move tt_move = (stored_entry.key == zobrist_key && stored_entry.depth > 0)
                       ? stored_entry.best_move
                       : Move::none();

    heuristics::scoreAndSort<Config>(sink, tt_move, ply);

    // Before searching, record the first sorted move as a fallback so the
    // engine always has a legal move even if the search is interrupted immediately.
    // Skip for constrained searches: if all search_moves are illegal the engine
    // should signal no move rather than silently picking a different one.
    if constexpr (IsRoot) {
        if (search_parameters.search_moves.empty()) {
            search_ctx.root_best_move = sink.moves[0][0];
        }
    }

    // Test hook: pause here so tests can stop the search after sort but before
    // any recursive call returns, exercising the fallback path deterministically.
    if constexpr (IsRoot && PauseAfterRootSort) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
            Move move = sink.moves[ply][i];
            if (ply == 0 && search_parameters.search_moves.size() > 0 &&
                ! search_parameters.search_moves.contains(toUci(move))) {
                needs_search[i] = false;
            }
            if (! needs_search[i]) {
                continue;
            }

            move_processor.applyMove(board, move);
            search_ctx.tt.prefetch(board.getZobristHash());
            bool is_exclusive = iteration == 0 && i != 0;
            int  score        = -search<! Side, Config>(search_ctx, board, move_processor,
                                                        search_parameters, restriction_context, depth - 1,
                                                        -beta, -alpha, st, sink, ply + 1, is_exclusive);
            move_processor.undoMove(board, move);
            if (abs(score) == SEARCH_INTERRUPTED) {
                return SEARCH_INTERRUPTED;
            }
            if (abs(score) == ON_EVALUATION) {
                all_done = false;
                continue;
            }
            needs_search[i] = false;

            if (score > best_score) {
                best_score = score;
                best_move  = move;
                if constexpr (IsRoot) {
                    search_ctx.root_best_move = move;
                }
                alpha = std::max(score, alpha);
            }
            if (alpha >= beta) {
#ifdef DEBUG
                search_ctx.beta_cutoffs.fetch_add(1, std::memory_order_relaxed);
#endif
                all_done = true;
                break;
            }
        }
    }

    search_ctx.tt.removeSearched(zobrist_key);
    search_ctx.tt.storeBounded(zobrist_key, best_score, best_move, depth, alpha_orig, beta);
    return best_score;
}

} // namespace bitcrusher

#endif // BITCRUSHER_SEARCH_HPP
