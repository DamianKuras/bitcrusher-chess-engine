#ifndef BITCRUSHER_KNIGHT_ATTACKS_HPP
#define BITCRUSHER_KNIGHT_ATTACKS_HPP

#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"

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

} // namespace bitcrusher

#endif // BITCRUSHER_KNIGHT_ATTACKS_HPP