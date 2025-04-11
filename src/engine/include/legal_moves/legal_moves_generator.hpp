#ifndef BITCRUSHER_LEGAL_MOVES_GENERATOR_HPP
#define BITCRUSHER_LEGAL_MOVES_GENERATOR_HPP

#include "bishop_legal_moves.hpp"
#include "concepts.hpp"
#include "king_legal_moves.hpp"
#include "knight_legal_moves.hpp"
#include "pawn_legal_moves.hpp"
#include "queen_legal_moves.hpp"
#include "rook_legal_moves.hpp"

namespace bitcrusher {

template <MoveSink MoveSinkT, Color Side>
void generateLegalMoves(const BoardState& board, MoveSinkT& sink) {
    if (board.restriction_context.check_count < 2) {
        generateLegalPawnMoves<MoveSinkT, Side>(board, sink);
        generateLegalKnightMoves<MoveSinkT, Side>(board, sink);
        generateLegalBishopMoves<MoveSinkT, Side>(board, sink);
        generateLegalRookMoves<MoveSinkT, Side>(board, sink);
        generateLegalQueenMoves<MoveSinkT, Side>(board, sink);
        generateLegalKingMoves<MoveSinkT, Side>(board, sink);
    } else { // if in double check only king moves legal
        generateLegalKingMoves<MoveSinkT, Side>(board, sink);
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_LEGAL_MOVES_GENERATOR_HPP