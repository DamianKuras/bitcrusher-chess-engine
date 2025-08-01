#ifndef BITCRUSHER_KNIGHT_LEGAL_MOVES_HPP
#define BITCRUSHER_KNIGHT_LEGAL_MOVES_HPP

#include "attack_generators/knight_attacks.hpp"
#include "concepts.hpp"
#include "move_generation_from_bitboard.hpp"
#include "restriction_context.hpp"

namespace bitcrusher {

template <MoveSink MoveSinkT, Color Side>
void generateLegalKnightMoves(const BoardState&         board,
                              const RestrictionContext& restriction_context,
                              MoveSinkT&                sink) {
    uint64_t knights_not_pinned =
        restriction_context.nonRestricted(board.getBitboard<PieceType::KNIGHT, Side>());
    while (knights_not_pinned) {
        Square   knight_square = utils::popFirstSetSquare(knights_not_pinned);
        uint64_t knight_moves  = generateKnightsAttacks(convert::toBitboard(knight_square)) &
                                restriction_context.checkmask;
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