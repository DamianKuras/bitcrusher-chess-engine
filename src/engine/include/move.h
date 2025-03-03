#ifndef BITCRUSHER_MOVE_H
#define BITCRUSHER_MOVE_H

#include "bitboard.h"
#include "board_state.h"

namespace bitcrusher {

// The encoding leverages bit patterns:
// Promotions have the fourth bit set (binary 1xxx).
// Captures have the third bit set (binary x1xx).
enum class MoveFlag : uint8_t {
    QUIET                    = 0,
    DOUBLE_PAWN_PUSH         = 1,
    KINGSIDE_CASTLE          = 2,
    QUEENSIDE_CASTLE         = 3,
    CAPTURE                  = 4,
    EN_PASSANT_CAPTURE       = 5,
    KNIGHT_PROMOTION         = 8,
    BISHOP_PROMOTION         = 9,
    ROOK_PROMOTION           = 10,
    QUEEN_PROMOTION          = 11,
    KNIGHT_PROMOTION_CAPTURE = 12,
    BISHOP_PROMOTION_CAPTURE = 13,
    ROOK_PROMOTION_CAPTURE   = 14,
    QUEEN_PROMOTION_CAPTURE  = 15
};

struct Move {
    Square   from = Square::NULL_SQUARE;
    Square   to   = Square::NULL_SQUARE;
    MoveFlag flags;
    Piece    moved_piece;
};

constexpr bool isCapture(const Move& move) noexcept {
    return (static_cast<uint8_t>(move.flags) & 4) != 0;
}

constexpr bool isPromotion(const Move& move) noexcept {
    return (static_cast<uint8_t>(move.flags) & 8) != 0;
}

} // namespace bitcrusher

#endif // BITCRUSHER_MOVE_H
