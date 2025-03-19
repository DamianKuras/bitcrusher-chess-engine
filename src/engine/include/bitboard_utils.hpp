#ifndef BITCRUSHER_BITBOARD_UTILS_HPP
#define BITCRUSHER_BITBOARD_UTILS_HPP

#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include <bit>
#include <cassert>
#include <cstdint>

namespace bitcrusher::utils {

[[nodiscard]] static constexpr bool isSquareSet(uint64_t bitboard,
                                                Square square) noexcept {
    return (bitboard & convert::toBitboard(square)) != 0;
}

static constexpr void setSquare(uint64_t &bitboard, Square square) noexcept {
    bitboard |= convert::toBitboard(square);
}

static constexpr void clearSquare(uint64_t &bitboard, Square square) noexcept {
    bitboard &= ~convert::toBitboard(square);
}

// pops the first set square (LSB to MSB)
[[nodiscard]] static constexpr Square
popFirstSetSquare(uint64_t &bitboard) noexcept {
    auto const index = static_cast<Square>(std::countr_zero(bitboard));
    bitboard &= bitboard - 1;
    return index;
}

// Toggle bits at the source and destination squares
// Preconditions: the source square is set and the destination square is clear
static constexpr void toggleSquares(uint64_t &bitboard, Square source,
                                    Square destination) {
    bitboard ^=
        (convert::toBitboard(source) | convert::toBitboard(destination));
}

// Shifts bitboard towards direction without validation
static constexpr uint64_t shift(uint64_t bitboard, Direction direction) {
    if (direction == Direction::TOP) {
        return bitboard >> BOARD_DIMENSION;
    }
    if (direction == Direction::BOTTOM) {
        return bitboard << BOARD_DIMENSION;
    }
    if (direction == Direction::LEFT) {
        return bitboard >> 1;
    }
    // direction == Direction::RIGHT
    return bitboard << 1;
}

} // namespace bitcrusher::utils

#endif // BITCRUSHER_BITBOARD_UTILS_HPP