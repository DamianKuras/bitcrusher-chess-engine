#ifndef BITCRUSHER_KING_ATTACKS_HPP
#define BITCRUSHER_KING_ATTACKS_HPP

#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"

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

} // namespace bitcrusher

#endif // BITCRUSHER_KING_ATTACKS_HPP