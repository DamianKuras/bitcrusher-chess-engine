#ifndef BITCRUSHER_LEGAL_MOVES_GENERATOR_HPP
#define BITCRUSHER_LEGAL_MOVES_GENERATOR_HPP

#include "bishop_legal_moves.hpp"
#include "concepts.hpp"
#include "knight_legal_moves.hpp"
#include "legal_move_generators/king_legal_moves.hpp"
#include "pawn_legal_moves.hpp"
#include "queen_legal_moves.hpp"
#include "restriction_context.hpp"
#include "rook_legal_moves.hpp"

namespace bitcrusher {

/// @brief Policy controlling whether restriction context is updated during move generation.
///
/// Use UPDATE when restriction context may be stale; use LEAVE when you've already updated it.
enum class RestrictionContextUpdatePolicy : bool {
    UPDATE,
    LEAVE,
};

/// @brief Generates all legal moves for the given side, respecting restriction constraints.
///
/// Entry point for move generation. Handles both normal positions and check scenarios:
/// - Single check: Only moves that capture the checker or block are legal.
/// - Double check: Only king moves are legal (must move out of check).
/// - No check: All pieces can move freely (respecting their pin restrictions).
///
/// Optionally updates the restriction context before generation, or uses a pre-computed one.
///
/// @tparam Side The Color of the side to move(Color::WHITE or Color::BLACK).
/// @tparam MoveGenerationP Move generation scope policy. See MoveGenerationPolicy for available
/// options.
/// @tparam RestrictionContextUpdateP Whether to update restriction context before generation.
///         Set to UPDATE if context may be stale; LEAVE if you've already updated it.
/// @tparam MoveSinkT Type of the move sink that receives generated moves.
/// @param board The current board state of the position.
/// @param sink The move sink object that will store the generated capture moves.
/// @param restriction_context Contains check and pin informations.
/// @param ply Current ply (half-move count) used by some move sinks for array-based storage.
///        Defaults to 0.
template <Color                          Side,
          MoveGenerationPolicy           MoveGenerationP = MoveGenerationPolicy::FULL,
          RestrictionContextUpdatePolicy RestrictionContextUpdateP =
              RestrictionContextUpdatePolicy::UPDATE,
          MoveSink MoveSinkT>
void generateLegalMoves(const BoardState&   board,
                        MoveSinkT&          sink,
                        RestrictionContext& restriction_context,
                        int                 ply = 0) {

    if constexpr (RestrictionContextUpdateP == RestrictionContextUpdatePolicy::UPDATE) {
        updateRestrictionContext<Side>(board, restriction_context);
    }

    sink.setPly(ply);

    if (restriction_context.check_count < 2) { // In check or no check not all.
        generateLegalPawnMoves<Side, MoveGenerationP>(board, restriction_context, sink);
        generateLegalKnightMoves<Side, MoveGenerationP>(board, restriction_context, sink);
        generateLegalBishopMoves<Side, MoveGenerationP>(board, restriction_context, sink);
        generateLegalRookMoves<Side, MoveGenerationP>(board, restriction_context, sink);
        generateLegalQueenMoves<Side, MoveGenerationP>(board, restriction_context, sink);
        generateLegalKingMoves<Side, MoveGenerationP>(board, restriction_context, sink);
    } else { // In double check only king moves are legal.
        generateLegalKingMoves<Side, MoveGenerationP>(board, restriction_context, sink);
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_LEGAL_MOVES_GENERATOR_HPP