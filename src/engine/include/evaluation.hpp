#ifndef BITCRUSHER_EVALUATION_HPP
#define BITCRUSHER_EVALUATION_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include <bit>

namespace bitcrusher {
inline constexpr int PAWN_CENTIPAWN_VALUE   = 100;
inline constexpr int KNIGHT_CENTIPAWN_VALUE = 300;
inline constexpr int BISHOP_CENTIPAWN_VALUE = 300;
inline constexpr int ROOK_CENTIPAWN_VALUE   = 500;
inline constexpr int QUEEN_CENTIPAWN_VALUE  = 900;

template <PieceType PieceT>
constexpr int materialCountDifference(const BoardState& board) noexcept {
    const auto white_count = std::popcount(board.getBitboard<PieceT, Color::WHITE>());
    const auto black_count = std::popcount(board.getBitboard<PieceT, Color::BLACK>());
    return (white_count - black_count);
}

// Returns evaluation relative to side to move.
[[nodiscard]] constexpr int basicEval(const BoardState& board, Color side) {
    int eval = 0;
    eval += PAWN_CENTIPAWN_VALUE * materialCountDifference<PieceType::PAWN>(board);
    eval += KNIGHT_CENTIPAWN_VALUE * materialCountDifference<PieceType::KNIGHT>(board);
    eval += BISHOP_CENTIPAWN_VALUE * materialCountDifference<PieceType::BISHOP>(board);
    eval += ROOK_CENTIPAWN_VALUE * materialCountDifference<PieceType::ROOK>(board);
    eval += QUEEN_CENTIPAWN_VALUE * materialCountDifference<PieceType::QUEEN>(board);
    return side == Color::WHITE ? eval : -eval;
}

} // namespace bitcrusher

#endif // BITCRUSHER_EVALUATION_HPP