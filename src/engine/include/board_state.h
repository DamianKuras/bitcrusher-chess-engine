

#ifndef BITCRUSHER_BOARD_STATE_H_
#define BITCRUSHER_BOARD_STATE_H_

#include "bitboard.h"
#include <array>
#include <cstdint>

namespace bitcrusher {

/*
Each of the castling right has its own bit
*/
enum class CastlingRights : uint8_t {
    WHITE_KINGSIDE  = 1 << 0,
    WHITE_QUEENSIDE = 1 << 1,
    BLACK_KINGSIDE  = 1 << 2,
    BLACK_QUEENSIDE = 1 << 3,
};

enum class Piece : uint8_t {
    WHITE_PAWNS,
    WHITE_KNIGHTS,
    WHITE_BISHOPS,
    WHITE_ROOKS,
    WHITE_QUEENS,
    WHITE_KING,

    BLACK_PAWNS,
    BLACK_KNIGHTS,
    BLACK_BISHOPS,
    BLACK_ROOKS,
    BLACK_QUEENS,
    BLACK_KING,
};

struct BoardState {
    std::array<uint64_t, 12> bitboards{};
    Square                   en_passant_square = Square::NULL_SQUARE;
    uint8_t                  castling_rights   = 0; // Bitmask using CastlingRights values
    bool                     is_white_move     = true;
    uint8_t                  halfmove_clock    = 0;
    uint16_t                 fullmove_number   = 0;
};

} // namespace bitcrusher

#endif // BITCRUSHER_BOARD_STATE_H_
