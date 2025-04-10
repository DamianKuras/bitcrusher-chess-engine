#ifndef BITCRUSHER_KNIGHT_LEGAL_MOVES_HPP
#define BITCRUSHER_KNIGHT_LEGAL_MOVES_HPP

#include "attacks/knight_attacks.hpp"
#include "concepts.hpp"

namespace bitcrusher {

template <MoveSink MoveSinkT, Color Side>
void generateLegalKnightMoves(const BoardState& board, MoveSinkT& sink) {
    uint64_t knights_not_pinned = board.getNonRestrictedBitboard<PieceType::KNIGHT, Side>();
    while (knights_not_pinned) {
        Square   knight_square = utils::popFirstSetSquare(knights_not_pinned);
        uint64_t knight_moves  = generateKnightsAttacks(convert::toBitboard(knight_square)) &
                                board.restriction_context.checkmask &
                                ~board.getOwnOccupancy<Side>();
        uint64_t knight_captures     = knight_moves & board.getOpponentOccupancy<Side>();
        uint64_t knight_non_captures = knight_moves & board.getEmptySquares();
        createMovesFromBitboard<MoveType::QUIET, PieceType::KNIGHT>(sink, knight_non_captures,
                                                                    knight_square, board);
        createMovesFromBitboard<MoveType::CAPTURE, PieceType::KNIGHT>(sink, knight_captures,
                                                                      knight_square, board);
    }
}

} // namespace bitcrusher

#endif // BITCRUSHER_KNIGHT_LEGAL_MOVES_HPP