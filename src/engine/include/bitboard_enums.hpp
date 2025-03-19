#ifndef BITCRUSHER_BITBOARD_ENUMS_HPP
#define BITCRUSHER_BITBOARD_ENUMS_HPP

#include <cstdint>
#include <utility>

namespace bitcrusher {

inline constexpr int BOARD_DIMENSION = 8;

const uint64_t EMPTY_BITBOARD = 0;

const uint64_t FULL_BITBOARD = ~EMPTY_BITBOARD;

// Represents chess board squares using 0-based indexing from A8 (0) to H1 (63)
//
// Null square is equal to 64 outside of the 0-63 board range.
// clang-format off
enum class Square : uint8_t {
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1,
    NULL_SQUARE,
};

const int SQUARE_COUNT=64;

enum class Rank: uint8_t {
    R_8,
    R_7,
    R_6,
    R_5,
    R_4,
    R_3,
    R_2,    
    R_1,
};

enum class File:uint8_t {
    A, B, C, D, E, F, G, H,
};
// clang-format on

enum class Color : bool { WHITE, BLACK };
enum class Side : bool { KINGSIDE, QUEENSIDE };

enum class Diagonal : uint8_t {
    H8,
    G8H7,
    F8H6,
    E8H5,
    D8H4,
    C8H3,
    B8H2,
    A8H1, // longest diagonal
    A7G1,
    A6F1,
    A5E1,
    A4D1,
    A3C1,
    A2B1,
    A1,
};

const int DIAGONAL_COUNT = 15;

enum class CounterDiagonal : uint8_t {
    A8,
    A7B8,
    A6C8,
    A5D8,
    A4E8,
    A3F8,
    A2G8,
    A1H8, // longest counter diagonal
    B1H7,
    C1H6,
    D1H5,
    E1H4,
    F1H3,
    G1H2,
    H1
};

enum class Direction : uint8_t {
    TOP,
    LEFT,
    RIGHT,
    BOTTOM,
};

// Square operators
[[nodiscard]] static constexpr Square operator+(Square square,
                                                int offset) noexcept {
    return static_cast<Square>(std::to_underlying(square) + offset);
}

static constexpr void operator+=(Square &square, int offset) noexcept {
    square = square + offset;
}

[[nodiscard]] static constexpr Square operator-(Square square,
                                                int offset) noexcept {
    return static_cast<Square>(std::to_underlying(square) - offset);
}

static constexpr Square operator-=(Square square, int offset) noexcept {
    return square - offset;
}

// File operators
[[nodiscard]] static constexpr File operator+(File file, int offset) noexcept {
    return static_cast<File>(std::to_underlying(file) + offset);
}

static constexpr void operator+=(File &file, int offset) noexcept {
    file = file + offset;
}

[[nodiscard]] static constexpr File operator-(File file, int offset) noexcept {
    return static_cast<File>(std::to_underlying(file) - offset);
}

// Rank operators
[[nodiscard]] static constexpr Rank operator+(Rank rank, int offset) noexcept {
    return static_cast<Rank>(std::to_underlying(rank) - offset);
}

static constexpr void operator+=(Rank &rank, int offset) noexcept {
    rank = rank + offset;
}

[[nodiscard]] static constexpr Rank operator-(Rank rank, int offset) noexcept {
    return static_cast<Rank>(std::to_underlying(rank) + offset);
}

} // namespace bitcrusher

#endif // BITCRUSHER_BITBOARD_ENUMS_HPP