#ifndef BITCRUSHER_SQUARES_ATTACKED_HPP
#define BITCRUSHER_SQUARES_ATTACKED_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "diagonal_slider_attacks.hpp"
#include "horizontal_vertical_slider_attacks.hpp"
#include "knight_attacks.hpp"
#include "pawn_attacks.hpp"

namespace bitcrusher {

template <Color Side> inline std::uint64_t generateSquaresAttacked(const BoardState& board) {
    std::uint64_t attacked_squares = EMPTY_BITBOARD;

    attacked_squares |= generatePawnsAttacks<Side>(board.getBitboard<PieceType::PAWN, Side>());
    attacked_squares |= generateKnightsAttacks(board.getBitboard<PieceType::KNIGHT, Side>());
    attacked_squares |=
        generateDiagonalAttacks(board.getDiagonalSliders<Side>(), board.getAllOccupancy());
    attacked_squares |= generateHorizontalVerticalAttacks(
        board.getHorizontalVerticalSliders<Side>(), board.getAllOccupancy());
    attacked_squares |= generateKingAttacks(board.getBitboard<PieceType::KING, Side>());

    return attacked_squares;
}

template <Color Side>
inline std::uint64_t generateSquaresAttackedXRayingOpponentKing(const BoardState& board) {
    std::uint64_t attacked_squares       = EMPTY_BITBOARD;
    std::uint64_t opponent_king_bitboard = board.getBitboard<PieceType::KING, ! Side>();
    attacked_squares |= generatePawnsAttacks<Side>(board.getBitboard<PieceType::PAWN, Side>());
    attacked_squares |= generateKnightsAttacks(board.getBitboard<PieceType::KNIGHT, Side>());
    attacked_squares |= generateDiagonalAttacks(board.getDiagonalSliders<Side>(),
                                                (board.getAllOccupancy() ^ opponent_king_bitboard));

    attacked_squares |=
        generateHorizontalVerticalAttacks(board.getHorizontalVerticalSliders<Side>(),
                                          (board.getAllOccupancy() ^ opponent_king_bitboard));
    attacked_squares |= generateKingAttacks(board.getBitboard<PieceType::KING, Side>());
    return attacked_squares;
}

} // namespace bitcrusher

#endif // BITCRUSHER_SQUARES_ATTACKED_HPP