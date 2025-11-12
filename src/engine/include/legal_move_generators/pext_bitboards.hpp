#ifndef BITCRUSHER_PEXT_BITBOARDS_HPP
#define BITCRUSHER_PEXT_BITBOARDS_HPP

#if defined(HAS_BMI2)
#    include "./attack_generators/diagonal_slider_attacks.hpp"
#    include "./attack_generators/horizontal_vertical_slider_attacks.hpp"
#    include "bitboard_conversions.hpp"
#    include "bitboard_enums.hpp"
#    include "file_rank_bitboards.hpp"
#    include <bit>
#    include <cstdint>
#    include <immintrin.h>

namespace bitcrusher {

inline constexpr int PEXT_SIZE = 107648;

/// @brief Pre-computed magic bitboard lookup tables for fast O(1) sliding piece attacks.
///
/// Uses the PEXT (Parallel Bits Extract) BMI2 CPU instruction with magic bitboards to generate
/// rook and bishop attacks in O(1) time. Leverages the PEXT instruction to convert occupancy
/// bitboards into indices for pre-computed attack lookup tables.
///
/// **Initialization:**
/// Attack tables are initialized once via static initializer.
class PextBitboards {
public:
    inline static std::array<uint64_t, SQUARE_COUNT> rook_masks{};
    inline static std::array<uint64_t, SQUARE_COUNT> bishop_masks{};
    inline static std::array<uint64_t, SQUARE_COUNT> rook_index{};
    inline static std::array<uint64_t, SQUARE_COUNT> bishop_index{};
    inline static std::array<uint64_t, PEXT_SIZE>    attack_table{};

private:
    static uint64_t getEdgeFilter(Square sq) {
        uint64_t mask = (RANK_BITBOARDS[std::to_underlying(Rank::R_1)] |
                         RANK_BITBOARDS[std::to_underlying(Rank::R_8)]) &
                        ~RANK_BITBOARDS[std::to_underlying(convert::toRank(sq))];
        mask |= (FILE_BITBOARDS[std::to_underlying(File::A)] |
                 FILE_BITBOARDS[std::to_underlying(File::H)]) &
                ~FILE_BITBOARDS[std::to_underlying(convert::toFile(sq))];
        return ~mask;
    }

    struct Initializer {
        Initializer() { PextBitboards::initPextBitboards(); }
    };

    inline static Initializer initializer;

    static inline void initPextBitboards() {
        uint32_t current_index = 0;

        // Initialize rooks.
        for (int square_index = 0; square_index < SQUARE_COUNT; square_index++) {
            auto sq                  = static_cast<Square>(square_index);
            rook_index[square_index] = current_index;
            rook_masks[square_index] =
                generateHorizontalVerticalAttacks(convert::toBitboard(sq), EMPTY_BITBOARD) &
                getEdgeFilter(sq);

            uint64_t mask = rook_masks[square_index];
            uint64_t max  = 1ULL << std::popcount(mask);
            for (int i = 0; i < max; i++) {
                uint64_t blockers = _pdep_u64(i, mask);
                attack_table[current_index++] =
                    generateHorizontalVerticalAttacks(convert::toBitboard(sq), blockers);
            }
        }

        // Initialize bishops
        for (int square_index = 0; square_index < SQUARE_COUNT; square_index++) {
            auto sq = static_cast<Square>(square_index);

            bishop_index[square_index] = current_index;
            bishop_masks[square_index] =
                generateDiagonalAttacks(convert::toBitboard(sq), EMPTY_BITBOARD) &
                getEdgeFilter(sq);
            uint64_t mask = bishop_masks[square_index];
            uint64_t max  = 1ULL << std::popcount(mask);
            for (int i = 0; i < max; i++) {
                uint64_t blockers = _pdep_u64(i, mask);
                attack_table[current_index++] =
                    generateDiagonalAttacks(convert::toBitboard(sq), blockers);
            }
        }
    }
};

} // namespace bitcrusher
#endif // defined(HAS_BMI2)
#endif // BITCRUSHER_PEXT_BITBOARDS_HPP
