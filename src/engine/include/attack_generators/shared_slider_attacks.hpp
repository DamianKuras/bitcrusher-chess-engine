#ifndef BITCRUSHER_SHARED_SLIDER_ATTACKS_HPP
#define BITCRUSHER_SHARED_SLIDER_ATTACKS_HPP
#include "bitboard_offsets.hpp"

namespace bitcrusher {

struct SliderOffsets {
    offset::BitboardOffset offset_x1;
    offset::BitboardOffset offset_x2;
    offset::BitboardOffset offset_x4;
};

// Kogge-stone
template <SliderOffsets Offset>
constexpr uint64_t computeOccludedFill(uint64_t sliders, uint64_t empty_squares) {
    sliders |= empty_squares & offset::safeShift<Offset.offset_x1>(sliders);
    empty_squares &= offset::safeShift<Offset.offset_x1>(empty_squares);

    sliders |= empty_squares & offset::safeShift<Offset.offset_x2>(sliders);
    empty_squares &= offset::safeShift<Offset.offset_x2>(empty_squares);

    sliders |= empty_squares & offset::safeShift<Offset.offset_x4>(sliders);
    return sliders;
}

} // namespace bitcrusher

#endif // BITCRUSHER_SHARED_SLIDER_ATTACKS_HPP