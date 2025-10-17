#ifndef BITCRUSHER_KING_ATTACKS_HPP
#define BITCRUSHER_KING_ATTACKS_HPP

#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"
#include <cstdint>

namespace bitcrusher {

constexpr uint64_t generateKingAttacks(uint64_t king_bb) {
    uint64_t attacks = 0ULL;

    attacks |= offset::safeShift<offset::makeOffset<Direction::TOP>()>(king_bb);
    attacks |= offset::safeShift<offset::makeOffset<Direction::TOP, Direction::LEFT>()>(king_bb);
    attacks |= offset::safeShift<offset::makeOffset<Direction::TOP, Direction::RIGHT>()>(king_bb);
    attacks |= offset::safeShift<offset::makeOffset<Direction::LEFT>()>(king_bb);
    attacks |= offset::safeShift<offset::makeOffset<Direction::RIGHT>()>(king_bb);
    attacks |= offset::safeShift<offset::makeOffset<Direction::BOTTOM, Direction::LEFT>()>(king_bb);
    attacks |=
        offset::safeShift<offset::makeOffset<Direction::BOTTOM, Direction::RIGHT>()>(king_bb);
    attacks |= offset::safeShift<offset::makeOffset<Direction::BOTTOM>()>(king_bb);
    return attacks;
}

consteval std::array<uint64_t, SQUARE_COUNT> generatePrecomputedKingAttacks() {
    std::array<uint64_t, SQUARE_COUNT> king_attacks{};
    for (int i = 0; i < SQUARE_COUNT; i++) {
        king_attacks[i] = generateKingAttacks(convert::toBitboard(static_cast<Square>(i)));
    }
    return king_attacks;
}

inline constinit std::array<uint64_t, SQUARE_COUNT> precomputed_king_attacks =
    generatePrecomputedKingAttacks();

constexpr uint64_t generateKingAttacks(Square sq) {
    return precomputed_king_attacks[static_cast<int>(sq)];
}

} // namespace bitcrusher

#endif // BITCRUSHER_KING_ATTACKS_HPP