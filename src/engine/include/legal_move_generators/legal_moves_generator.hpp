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

enum class RestrictionContextPolicy : bool {
    UPDATE,
    LEAVE,
};

template <Color                    Side,
          MoveGenerationPolicy     MoveGenerationP     = MoveGenerationPolicy::FULL,
          RestrictionContextPolicy RestrictionContextP = RestrictionContextPolicy::UPDATE,
          MoveSink                 MoveSinkT>
void generateLegalMoves(const BoardState&   board,
                        MoveSinkT&          sink,
                        RestrictionContext& restriction_context,
                        int                 ply = 0) {

    if constexpr (RestrictionContextP == RestrictionContextPolicy::UPDATE) {
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