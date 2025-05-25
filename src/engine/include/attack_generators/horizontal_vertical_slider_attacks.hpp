#ifndef BITCRUSHER_HORIZONTAL_VERTICAL_SLIDER_ATTACKS_HPP
#define BITCRUSHER_HORIZONTAL_VERTICAL_SLIDER_ATTACKS_HPP

#include "shared_slider_attacks.hpp"

namespace bitcrusher {

template <Direction Dir> consteval SliderOffsets makeHVOffsets() {
    return SliderOffsets{.offset_x1 = offset::makeOffset<Dir>(),
                         .offset_x2 = offset::makeOffset<offset::RepeatedDirection{Dir, 2}>(),
                         .offset_x4 = offset::makeOffset<offset::RepeatedDirection{Dir, 4}>()};
}

[[nodiscard]] inline uint64_t
generateBottomAttacks(uint64_t square_bitboard, uint64_t occ_file, uint64_t file_mask) {
    return (occ_file ^ (occ_file - 2 * square_bitboard)) & file_mask;
}

[[nodiscard]] inline uint64_t
generateTopAttacks(uint64_t square_bitboard, uint64_t occ_file, uint64_t file_mask) {
    uint64_t rev_occ    = std::byteswap(occ_file);
    uint64_t rev_slider = std::byteswap(square_bitboard);
    return std::byteswap(rev_occ ^ (rev_occ - 2 * rev_slider)) & file_mask;
}

[[nodiscard]] inline uint64_t
generateRightAttacks(uint64_t square_bitboard, uint64_t occ_rank, uint64_t rank_mask) {
    return (occ_rank ^ (occ_rank - 2 * square_bitboard)) & rank_mask;
}

[[nodiscard]] inline uint64_t
generateLeftAttacks(uint64_t square_bitboard, uint64_t occ_rank, uint64_t /*_*/) {

    const uint64_t o = occ_rank & ~square_bitboard; // Exclude slider from occupancy

    const uint64_t west_occupancy = o & (square_bitboard - 1); // Squares west of slider

    // Find closest blocker using portable MSB detection
    const uint64_t msb = (west_occupancy != 0ULL) ? 63 - std::countl_zero(west_occupancy) : 64;
    const uint64_t cbn = (msb < 64) ? 1ULL << msb : 0;
    const uint64_t masked_occ = west_occupancy | square_bitboard;
    // SBAMG formula with edge wrapping protection
    return ((masked_occ ^ (masked_occ - 3 * cbn)) & (square_bitboard - 1));
}

// Generates all horizontal and vertical attacks for multiple pieces
[[nodiscard]] inline uint64_t generateHorizontalVerticalAttacks(const uint64_t sliders,
                                                                uint64_t       occupancy) {
    uint64_t       attacks   = 0ULL;
    constexpr auto OFFSETS_1 = makeHVOffsets<Direction::TOP>();
    const uint64_t fill_1    = computeOccludedFill<OFFSETS_1>(sliders, ~occupancy);
    attacks |= offset::safeShift<OFFSETS_1.offset_x1>(fill_1);

    constexpr auto OFFSETS_2 = makeHVOffsets<Direction::LEFT>();
    const uint64_t fill_2    = computeOccludedFill<OFFSETS_2>(sliders, ~occupancy);
    attacks |= offset::safeShift<OFFSETS_2.offset_x1>(fill_2);

    constexpr auto OFFSETS_3 = makeHVOffsets<Direction::RIGHT>();
    const uint64_t fill_3    = computeOccludedFill<OFFSETS_3>(sliders, ~occupancy);
    attacks |= offset::safeShift<OFFSETS_3.offset_x1>(fill_3);

    constexpr auto OFFSETS_4 = makeHVOffsets<Direction::BOTTOM>();
    const uint64_t fill_4    = computeOccludedFill<OFFSETS_4>(sliders, ~occupancy);
    attacks |= offset::safeShift<OFFSETS_4.offset_x1>(fill_4);

    return attacks;
}

} // namespace bitcrusher

#endif // BITCRUSHER_HORIZONTAL_VERTICAL_SLIDER_ATTACKS_HPP