#ifndef BITCRUSHER_KNIGHT_ATTACKS_HPP
#define BITCRUSHER_KNIGHT_ATTACKS_HPP

#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"
#include <cstdint>

namespace bitcrusher {

constexpr uint64_t generateKnightsAttacks(uint64_t knights_bitboard) {
    uint64_t attacks = 0ULL;
    attacks |= offset::safeShift<
        offset::makeOffset<offset::RepeatedDirection{Direction::TOP, 2}, Direction::LEFT>()>(
        knights_bitboard);
    attacks |= offset::safeShift<
        offset::makeOffset<offset::RepeatedDirection{Direction::TOP, 2}, Direction::RIGHT>()>(
        knights_bitboard);
    attacks |= offset::safeShift<
        offset::makeOffset<offset::RepeatedDirection{Direction::BOTTOM, 2}, Direction::LEFT>()>(
        knights_bitboard);
    attacks |= offset::safeShift<
        offset::makeOffset<offset::RepeatedDirection{Direction::BOTTOM, 2}, Direction::RIGHT>()>(
        knights_bitboard);
    attacks |= offset::safeShift<
        offset::makeOffset<offset::RepeatedDirection{Direction::LEFT, 2}, Direction::TOP>()>(
        knights_bitboard);
    attacks |= offset::safeShift<
        offset::makeOffset<offset::RepeatedDirection{Direction::LEFT, 2}, Direction::BOTTOM>()>(
        knights_bitboard);
    attacks |= offset::safeShift<
        offset::makeOffset<offset::RepeatedDirection{Direction::RIGHT, 2}, Direction::TOP>()>(
        knights_bitboard);
    attacks |= offset::safeShift<
        offset::makeOffset<offset::RepeatedDirection{Direction::RIGHT, 2}, Direction::BOTTOM>()>(
        knights_bitboard);
    return attacks;
}

consteval std::array<uint64_t, SQUARE_COUNT> generatePrecomputedKnightAttacks() {
    std::array<uint64_t, SQUARE_COUNT> attacks{};
    for (int i = 0; i < SQUARE_COUNT; i++) {
        uint64_t knights_bitboard = convert::toBitboard(static_cast<Square>(i));
        attacks[i]                = generateKnightsAttacks(knights_bitboard);
    }
    return attacks;
}

inline constinit std::array<uint64_t, SQUARE_COUNT> precomputed_knight_attacks =
    generatePrecomputedKnightAttacks();

inline constexpr uint64_t generateKnightAttacks(Square knight_square) {
    return precomputed_knight_attacks[static_cast<int>(knight_square)];
}

} // namespace bitcrusher

#endif // BITCRUSHER_KNIGHT_ATTACKS_HPP