#ifndef BITCRUSHER_BITBOARD_OFFSETS_HPP
#define BITCRUSHER_BITBOARD_OFFSETS_HPP

#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "file_rank_bitboards.hpp"
#include <utility>

namespace bitcrusher::offset {

template <typename... Directions>
    requires(std::same_as<Directions, Direction> && ...)
[[nodiscard]] consteval int calculateOffset(Directions... directions) {
    int offset = 0;
    for (Direction direction : {directions...}) {
        offset += convert::toDelta(direction);
    }
    return offset;
}

// Shifts the bitboard without wrapping bits.
[[nodiscard]] constexpr uint64_t shiftBitboardNoWrap(uint64_t bitboard,
                                                     int offset) noexcept {
    if (offset > 0) {
        int unreachable_files_count = offset % BOARD_DIMENSION;
        uint64_t wrapped_files = EMPTY_BITBOARD;
        for (File f = File::A; std::to_underlying(f) < unreachable_files_count;
             f += 1) {
            wrapped_files |= FILE_BITBOARDS[std::to_underlying(f)];
        }
        return (bitboard & ~wrapped_files) << offset;
    }
    if (offset < 0) {
        offset = -offset; // get absolute value of negative offset
        int unreachable_files_count = offset % BOARD_DIMENSION;
        uint64_t wrapped_files = EMPTY_BITBOARD;
        for (int i = 0; i < unreachable_files_count; i++) {
            wrapped_files |= FILE_BITBOARDS[BOARD_DIMENSION - 1 - i];
        }
        return (bitboard & ~wrapped_files) >> offset;
    }
    return bitboard; // shift by 0
}

} // namespace bitcrusher::offset

#endif // BITCRUSHER_BITBOARD_OFFSETS_HPP