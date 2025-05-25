#ifndef BITCRUSHER_DIAGONAL_SLIDER_ATTACKS_HPP
#define BITCRUSHER_DIAGONAL_SLIDER_ATTACKS_HPP

#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"
#include "shared_slider_attacks.hpp"

namespace bitcrusher {
template <Direction First, Direction Second> consteval SliderOffsets makeDiagonalOffset() {
    return SliderOffsets{.offset_x1 = offset::makeOffset<First, Second>(),
                         .offset_x2 = offset::makeOffset<offset::RepeatedDirection{First, 2},
                                                         offset::RepeatedDirection{Second, 2}>(),
                         .offset_x4 = offset::makeOffset<offset::RepeatedDirection{First, 4},
                                                         offset::RepeatedDirection{Second, 4}>()};
}

[[nodiscard]] inline uint64_t
generateUpperLeftDiagonalAttacks(uint64_t square_bitboard, uint64_t occ_diag, uint64_t diag_mask) {
    uint64_t reverse_occupied = std::byteswap(occ_diag);
    uint64_t reverse_slider   = std::byteswap(square_bitboard);
    return std::byteswap(reverse_occupied ^ (reverse_occupied - 2 * reverse_slider)) & diag_mask;
}

[[nodiscard]] inline uint64_t generateBottomRightDiagonalAttacks(uint64_t square_bitboard,
                                                                 uint64_t occ_diag,
                                                                 uint64_t diag_mask) {
    return (occ_diag ^ (occ_diag - 2 * square_bitboard)) & diag_mask;
}

[[nodiscard]] inline uint64_t generateUpperRightDiagonalAttacks(uint64_t square_bitboard,
                                                                uint64_t occ_counter_diag,
                                                                uint64_t counter_diag_mask) {
    uint64_t reverse_occupied = std::byteswap(occ_counter_diag);
    uint64_t reverse_slider   = std::byteswap(square_bitboard);
    return std::byteswap(reverse_occupied ^ (reverse_occupied - 2 * reverse_slider)) &
           counter_diag_mask;
}

[[nodiscard]] inline uint64_t generateBottomLeftDiagonalAttacks(uint64_t square_bitboard,
                                                                uint64_t occ_counter_diag,
                                                                uint64_t counter_diag_mask) {
    return (occ_counter_diag ^ (occ_counter_diag - 2 * square_bitboard)) & counter_diag_mask;
}

// Generates all diagonal attacks for multiple pieces
[[nodiscard]] inline uint64_t generateDiagonalAttacks(uint64_t sliders, uint64_t occupancy) {
    uint64_t       attacks = 0ULL;
    constexpr auto OFFSETS = makeDiagonalOffset<Direction::TOP, Direction::LEFT>();
    uint64_t       fill    = computeOccludedFill<OFFSETS>(sliders, ~occupancy);
    attacks |= offset::safeShift<OFFSETS.offset_x1>(fill);

    constexpr auto OFFSETS_2 = makeDiagonalOffset<Direction::TOP, Direction::RIGHT>();
    uint64_t       fill_2    = computeOccludedFill<OFFSETS_2>(sliders, ~occupancy);
    attacks |= offset::safeShift<OFFSETS_2.offset_x1>(fill_2);

    constexpr auto OFFSETS_3 = makeDiagonalOffset<Direction::BOTTOM, Direction::LEFT>();
    uint64_t       fill_3    = computeOccludedFill<OFFSETS_3>(sliders, ~occupancy);
    attacks |= offset::safeShift<OFFSETS_3.offset_x1>(fill_3);

    constexpr auto OFFSETS_4 = makeDiagonalOffset<Direction::BOTTOM, Direction::RIGHT>();
    uint64_t       fill_4    = computeOccludedFill<OFFSETS_4>(sliders, ~occupancy);
    attacks |= offset::safeShift<OFFSETS_4.offset_x1>(fill_4);

    return attacks;
}

} // namespace bitcrusher

#endif // BITCRUSHER_DIAGONAL_SLIDER_ATTACKS_HPP