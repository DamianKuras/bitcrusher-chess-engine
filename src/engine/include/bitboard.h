#ifndef BITCRUSHER_BITBOARD_H_
#define BITCRUSHER_BITBOARD_H_

#include <cstdint>

namespace bitcrusher {

// Represents chess board squares using 0-based indexing from A1 (0) to H8 (63)
//
// Null square is equal to 64 outside of the 0-63 board range.
enum class Square : uint8_t {
    A1 = 0,
    B1,
    C1,
    D1,
    E1,
    F1,
    G1,
    H1,
    A2,
    B2,
    C2,
    D2,
    E2,
    F2,
    G2,
    H2,
    A3,
    B3,
    C3,
    D3,
    E3,
    F3,
    G3,
    H3,
    A4,
    B4,
    C4,
    D4,
    E4,
    F4,
    G4,
    H4,
    A5,
    B5,
    C5,
    D5,
    E5,
    F5,
    G5,
    H5,
    A6,
    B6,
    C6,
    D6,
    E6,
    F6,
    G6,
    H6,
    A7,
    B7,
    C7,
    D7,
    E7,
    F7,
    G7,
    H7,
    A8,
    B8,
    C8,
    D8,
    E8,
    F8,
    G8,
    H8,
    NULL_SQUARE
};

constexpr uint64_t bitboardFromSquare(Square square) noexcept {
    return 1ULL << static_cast<uint8_t>(square);
}

constexpr bool isSquareSet(uint64_t bitboard, Square square) noexcept {
    return (bitboard & bitboardFromSquare(square)) != 0;
}

constexpr void setSquare(uint64_t& bitboard, Square square) noexcept {
    bitboard |= bitboardFromSquare(square);
}

constexpr void clearSquare(uint64_t& bitboard, Square square) noexcept {
    bitboard &= ~bitboardFromSquare(square);
}

// pops the first set square (LSB to MSB)
constexpr Square popFirstSetSquare(uint64_t& bitboard) noexcept {
    int index = __builtin_ctzll(bitboard);
    bitboard &= bitboard - 1;
    return static_cast<Square>(index);
}

// Variadic template to create a bitboard from multiple squares.
//
// Example:
//
// uint64_t initalWhiteRookBitboard = bitboardFromSquares(square::A1,square::H1)
//
// or
//
// uint64_t file_A = bitboardFromSquares(square::A1, square::A2, square::A3,
// square::A4, square::A5, square::A6, square::A7, square::A8);
template <typename... Args> constexpr uint64_t bitboardFromSquares(Args... args) {
    uint64_t bitboard = 0;
    for (uint8_t square_index : {args...}) {
        bitboard |= bitboardFromSquare(square_index);
    }
    return bitboard;
}

} // namespace bitcrusher

#endif // BITCRUSHER_BITBOARD_H_