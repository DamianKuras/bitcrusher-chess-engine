#ifndef BITCRUSHER_SQUARES_ATTACKED_HPP
#define BITCRUSHER_SQUARES_ATTACKED_HPP

#include "bitboard_enums.hpp"
#include "bitboard_utils.hpp"
#include "board_state.hpp"
#include "diagonal_slider_attacks.hpp"
#include "horizontal_vertical_slider_attacks.hpp"
#include "knight_attacks.hpp"
#include "pawn_attacks.hpp"
#include "pext_bitboards.hpp"

namespace bitcrusher {

template <Color Side> inline std::uint64_t generateSquaresAttacked(const BoardState& board) {
    std::uint64_t attacked_squares = EMPTY_BITBOARD;

    attacked_squares |= generatePawnsAttacks<Side>(board.getBitboard<PieceType::PAWN, Side>());
    attacked_squares |= generateKnightsAttacks(board.getBitboard<PieceType::KNIGHT, Side>());
    attacked_squares |= generateKingAttacks(board.getBitboard<PieceType::KING, Side>());

#if defined(HAS_BMI2)
    uint64_t diag = board.getDiagonalSliders<Side>();
    while (diag) {
        attacked_squares |=
            getDiagonalAttacks(utils::popFirstSetSquare(diag), board.getAllOccupancy());
    }
    uint64_t hv = board.getHorizontalVerticalSliders<Side>();
    while (hv) {
        attacked_squares |=
            getHorizontalVerticalAttacks(utils::popFirstSetSquare(hv), board.getAllOccupancy());
    }
#else
    attacked_squares |=
        generateDiagonalAttacks(board.getDiagonalSliders<Side>(), board.getAllOccupancy());
    attacked_squares |= generateHorizontalVerticalAttacks(
        board.getHorizontalVerticalSliders<Side>(), board.getAllOccupancy());
#endif

    return attacked_squares;
}

template <Color Side>
inline std::uint64_t generateSquaresAttackedXRayingOpponentKing(const BoardState& board) {
    std::uint64_t  attacked_squares       = EMPTY_BITBOARD;
    std::uint64_t  opponent_king_bitboard = board.getBitboard<PieceType::KING, ! Side>();
    const uint64_t occupancy              = board.getAllOccupancy() ^ opponent_king_bitboard;

    attacked_squares |= generatePawnsAttacks<Side>(board.getBitboard<PieceType::PAWN, Side>());
    attacked_squares |= generateKnightsAttacks(board.getBitboard<PieceType::KNIGHT, Side>());
    attacked_squares |= generateKingAttacks(board.getBitboard<PieceType::KING, Side>());

#if defined(HAS_BMI2)
    uint64_t diag = board.getDiagonalSliders<Side>();
    while (diag) {
        attacked_squares |= getDiagonalAttacks(utils::popFirstSetSquare(diag), occupancy);
    }
    uint64_t hv = board.getHorizontalVerticalSliders<Side>();
    while (hv) {
        attacked_squares |= getHorizontalVerticalAttacks(utils::popFirstSetSquare(hv), occupancy);
    }
#else
    attacked_squares |= generateDiagonalAttacks(board.getDiagonalSliders<Side>(), occupancy);
    attacked_squares |=
        generateHorizontalVerticalAttacks(board.getHorizontalVerticalSliders<Side>(), occupancy);
#endif

    return attacked_squares;
}

} // namespace bitcrusher

#endif // BITCRUSHER_SQUARES_ATTACKED_HPP