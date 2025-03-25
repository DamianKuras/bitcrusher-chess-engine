#ifndef BITCRUSHER_BITBOARD_OFFSETS_HPP
#define BITCRUSHER_BITBOARD_OFFSETS_HPP

#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "file_rank_bitboards.hpp"

namespace bitcrusher::offset {

// Represents a direction repeated a certain number of times
struct RepeatedDirection {
    Direction   dir{};
    std::size_t count{};

    consteval RepeatedDirection(Direction dir, std::size_t count) : dir(dir), count(count) {}
};

// Encapsulates both the wrap prevention mask and the computed shift delta
struct BitboardOffset {
    uint64_t wrap_prevention_mask;
    int      shift_value;

    consteval BitboardOffset(uint64_t mask, int shift)
        : wrap_prevention_mask(mask), shift_value(shift) {}
};

template <typename T>
concept DirectionVariant = std::is_same_v<T, Direction> || std::is_same_v<T, RepeatedDirection>;

// Computes the total shift offset from a parameter pack of Directions/RepeatedDirection
template <auto... Directions>
    requires(DirectionVariant<decltype(Directions)> && ...)
[[nodiscard]] consteval int calculateOffset() noexcept {
    int offset_delta = 0;
    // Use a lambda to process each parameter without requiring a common type
    auto accumulate_offset = [&](auto direction) {
        if constexpr (std::is_same_v<decltype(direction), Direction>) {
            offset_delta += convert::toDelta(direction);
        } else {
            // Assume direction is of type RepeatedDirection
            offset_delta += convert::toDelta(direction.dir) * direction.count;
        }
    };
    (accumulate_offset(Directions), ...); 
    return offset_delta;
}

// Computes a wrap prevention mask to avoid file wrap-around on left/right shifts
template <auto... Directions>
    requires(DirectionVariant<decltype(Directions)> && ...)
[[nodiscard]] consteval uint64_t computeWrapPreventionMask() noexcept {
    int  left_file_count  = 0;
    int  right_file_count = 0;
    auto count_files      = [&](auto direction) {
        if constexpr (std::is_same_v<decltype(direction), Direction>) {
            if (direction == Direction::LEFT) {
                ++left_file_count;
            } else if (direction == Direction::RIGHT) {
                ++right_file_count;
            }
        } else {
            // Assume direction is of type RepeatedDirection
            if (direction.dir == Direction::LEFT) {
                left_file_count += direction.count;
            } else if (direction.dir == Direction::RIGHT) {
                right_file_count += direction.count;
            }
        }
    };
    (count_files(Directions), ...);

    uint64_t prevention_mask = 0ULL;
    if (left_file_count > 0) {
        for (int i = 0; i < left_file_count; ++i) {
            prevention_mask |= FILE_BITBOARDS[i];
        }
    }
    if (right_file_count > 0) {
        for (int i = 0; i < right_file_count; ++i) {
            prevention_mask |= FILE_BITBOARDS[BOARD_DIMENSION - 1 - i];
        }
    }
    return ~prevention_mask;
}

// Performs a safe shift on the bitboard, applying the wrap prevention mask first
template <BitboardOffset Offset>
[[nodiscard]] constexpr uint64_t safeShift(uint64_t bitboard) noexcept {

    if constexpr (Offset.shift_value > 0) {
        return (bitboard & Offset.wrap_prevention_mask) << Offset.shift_value;
    } else if constexpr (Offset.shift_value < 0) {
        return (bitboard & Offset.wrap_prevention_mask) >> -Offset.shift_value;
    }

    return bitboard; // No shift needed when offset is zero
}

// Constructs a BitboardOffset from a parameter pack of directions
template <auto... Directions>
    requires(DirectionVariant<decltype(Directions)> && ...)
consteval BitboardOffset makeOffset() noexcept {
    uint64_t mask  = computeWrapPreventionMask<Directions...>();
    int      shift = calculateOffset<Directions...>();
    return BitboardOffset{mask, shift};
}

// Shifts the bitboard safely without wrapping bits between files
template <auto... Directions>
    requires(DirectionVariant<decltype(Directions)> && ...)
constexpr uint64_t shiftBitboardNoWrap(uint64_t bitboard) noexcept {
    constexpr BitboardOffset OFFSET = makeOffset<Directions...>();
    return safeShift<OFFSET>(bitboard);
}

} // namespace bitcrusher::offset

#endif // BITCRUSHER_BITBOARD_OFFSETS_HPP