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

template <Color Side,
          MoveGenerationPolicy MoveGenerationP = MoveGenerationPolicy::FULL,
          MoveSink             MoveSinkT>
void generateLegalMoves(const BoardState&        board,
                        const RestrictionContext restriction_context,
                        MoveSinkT&               sink) {

    if (restriction_context.check_count < 2) { // In check or no check not all.
        generateLegalQueenMoves< Side, MoveGenerationP>(board, restriction_context, sink);
        generateLegalRookMoves< Side, MoveGenerationP>(board, restriction_context, sink);
        generateLegalBishopMoves< Side, MoveGenerationP>(board, restriction_context,
                                                                   sink);
        generateLegalKnightMoves< Side, MoveGenerationP>(board, restriction_context,
                                                                   sink);
        generateLegalPawnMoves< Side, MoveGenerationP>(board, restriction_context, sink);
        generateLegalKingMoves< Side, MoveGenerationP>(board, restriction_context, sink);
    } else { // In double check only king moves are legal.
        generateLegalKingMoves<Side, MoveGenerationP>(board, restriction_context, sink);
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_LEGAL_MOVES_GENERATOR_HPP