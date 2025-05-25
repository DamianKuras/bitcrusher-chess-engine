#ifndef BITCRUSHER_LEGAL_MOVES_GENERATOR_HPP
#define BITCRUSHER_LEGAL_MOVES_GENERATOR_HPP

#include "bishop_legal_moves.hpp"
#include "bitboard_concepts.hpp"
#include "checks_pins_detection.hpp"
#include "knight_legal_moves.hpp"
#include "legal_move_generators/king_legal_moves.hpp"
#include "pawn_legal_moves.hpp"
#include "queen_legal_moves.hpp"
#include "restriction_context.hpp"
#include "rook_legal_moves.hpp"

namespace bitcrusher {

template <Color Side, MoveSink MoveSinkT>
void generateLegalMoves(const BoardState&        board,
                        const RestrictionContext restriction_context,
                        MoveSinkT&               sink) {
    if (restriction_context.check_count < 2) {
        generateLegalPawnMoves<MoveSinkT, Side>(board, restriction_context, sink);
        generateLegalKnightMoves<MoveSinkT, Side>(board, restriction_context, sink);
        generateLegalBishopMoves<MoveSinkT, Side>(board, restriction_context, sink);
        generateLegalRookMoves<MoveSinkT, Side>(board, restriction_context, sink);
        generateLegalQueenMoves<MoveSinkT, Side>(board, restriction_context, sink);
        generateLegalKingMoves<MoveSinkT, Side>(board, restriction_context, sink);
    } else { // if in double check only king moves are legal
        generateLegalKingMoves<MoveSinkT, Side>(board, restriction_context, sink);
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_LEGAL_MOVES_GENERATOR_HPP