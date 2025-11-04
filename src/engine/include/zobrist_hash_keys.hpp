#ifndef BITCRUSHER_ZOBRIST_HASH_KEYS_HPP
#define BITCRUSHER_ZOBRIST_HASH_KEYS_HPP

#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "random"
#include <cassert>
#include <cstdint>

namespace bitcrusher {

class ZobristKeys {
    inline static std::array<std::array<uint64_t, PIECE_COUNT>, SQUARE_COUNT> zobrist_piece_table;
    inline static uint64_t                                                    is_black_move_zobrist;
    inline static std::array<uint64_t, CASTLING_RIGHTS_COUNT> zobrist_castling_rights;
    inline static std::array<uint64_t, BOARD_DIMENSION>       zobrist_en_passant_file;
    inline static bool                                        initialized = false;

public:
    static void init(uint64_t seed) {
        if (initialized) {
            return;
        }

        initialized = true;
        std::mt19937_64 rng(seed);
        for (int i = 0; i < SQUARE_COUNT; i++) {
            for (int j = 0; j < PIECE_COUNT; j++) {
                zobrist_piece_table[i][j] = rng();
            }
        }
        is_black_move_zobrist = rng();
        for (auto& key : zobrist_castling_rights) {
            key = rng();
        }
        for (auto& key : zobrist_en_passant_file) {
            key = rng();
        }
    }

    static constexpr uint64_t getPieceSquareKey(Piece piece, Square square) {
        return zobrist_piece_table[static_cast<int>(square)][static_cast<int>(piece)];
    }

    static constexpr uint64_t getIsBlackMoveKey() { return is_black_move_zobrist; }

    template <CastlingRights Right>
        requires SingularCastlingRight<Right>
    static constexpr uint64_t getCastlingRightsKey() {
        if constexpr (Right == CastlingRights::WHITE_KINGSIDE) {
            return zobrist_castling_rights[0];
        }
        if constexpr (Right == CastlingRights::WHITE_QUEENSIDE) {
            return zobrist_castling_rights[1];
        }
        if constexpr (Right == CastlingRights::BLACK_KINGSIDE) {
            return zobrist_castling_rights[2];
        }
        if constexpr (Right == CastlingRights::BLACK_QUEENSIDE) {
            return zobrist_castling_rights[3];
        }
    }

    static constexpr uint64_t getEnPassantKey(Square ep_square) {
        if (ep_square == Square::NULL_SQUARE) {
            return 0;
        }
        int en_passant_file = static_cast<int>(convert::toFile(ep_square));
        return zobrist_en_passant_file[en_passant_file];
    }
};
} // namespace bitcrusher

#endif // BITCRUSHER_ZOBRIST_HASH_KEYS_HPP