#ifndef BITCRUSHER_BITBOARD_CONVERSIONS_HPP
#define BITCRUSHER_BITBOARD_CONVERSIONS_HPP

#include "bitboard_enums.hpp"
#include <utility>

namespace bitcrusher::convert {

[[nodiscard]] constexpr int toDigit(const char c) noexcept { return c - '0'; }

[[nodiscard]] constexpr Square toSquare(File file, Rank rank) noexcept {
    auto square =
        static_cast<Square>((std::to_underlying(rank) * BOARD_DIMENSION) +
                            std::to_underlying(file));
    ;
    return square;
}

[[nodiscard]] constexpr File toFile(char c) noexcept {
    return static_cast<File>(c - 'a');
}

[[nodiscard]] constexpr Rank toRank(char c) {
    return static_cast<Rank>(BOARD_DIMENSION - convert::toDigit(c));
}

// Convert a single Square to its bitboard representation.
[[nodiscard]]
static constexpr uint64_t toBitboard(Square square) noexcept {
    return 1ULL << std::to_underlying(square);
}

// Variadic template to create a bitboard from multiple squares.
//
// Example:
//
//  uint64_t initalWhiteRookBitboard =
//  toBitboard(Square::A1,Square::H1)
//
// or
//
// uint64_t file_A = toBitboard(Square::A1, Square::A2, Square::A3,
// Square::A4, Square::A5, Square::A6, Square::A7, Square::A8);
template <typename... Squares>
    requires(std::same_as<Squares, Square> && ...)
[[nodiscard]] consteval uint64_t toBitboard(Squares... args) {
    uint64_t bitboard = 0ULL;
    for (Square square : {args...}) {
        bitboard |= convert::toBitboard(square);
    }
    return bitboard;
}

constexpr Rank toRank(Square square) {
    return static_cast<Rank>((std::to_underlying(square) / BOARD_DIMENSION));
}

constexpr File toFile(Square square) {
    return static_cast<File>((std::to_underlying(square) % BOARD_DIMENSION));
}

[[nodiscard]] constexpr int toDelta(Direction d) noexcept {
    switch (d) {
    case Direction::TOP:
        return -static_cast<int8_t>(BOARD_DIMENSION);
    case Direction::BOTTOM:
        return BOARD_DIMENSION;
    case Direction::LEFT:
        return -1;
    case Direction::RIGHT:
        return 1;
    default:
        std::unreachable(); 
    }
}

} // namespace bitcrusher::convert

#endif