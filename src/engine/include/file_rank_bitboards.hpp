#ifndef BITCRUSHER_FILE_RANK_BITBOARDS_HPP
#define BITCRUSHER_FILE_RANK_BITBOARDS_HPP

#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "bitboard_utils.hpp"
#include <array>

namespace bitcrusher {
consteval std::array<uint64_t, BOARD_DIMENSION> createFileBitboards() {
    std::array<uint64_t, BOARD_DIMENSION> array{};
    array[std::to_underlying(File::A)] =
        convert::toBitboard(Square::A1, Square::A2, Square::A3, Square::A4, Square::A5, Square::A6,
                            Square::A7, Square::A8);
    uint64_t prev_file = array[std::to_underlying(File::A)];
    for (File file : {File::B, File::C, File::D, File::E, File::F, File::G, File::H}) {
        array[std::to_underlying(file)] = utils::shift(prev_file, Direction::RIGHT);
        prev_file                       = array[std::to_underlying(file)];
    }
    return array;
}

// File bitboards lookup indexed using File enum
constexpr std::array<uint64_t, BOARD_DIMENSION> FILE_BITBOARDS = createFileBitboards();

consteval std::array<uint64_t, BOARD_DIMENSION> createRankBitboards() {
    std::array<uint64_t, BOARD_DIMENSION> array{};
    array[std::to_underlying(Rank::R_1)] =
        convert::toBitboard(Square::A1, Square::B1, Square::C1, Square::D1, Square::E1, Square::F1,
                            Square::G1, Square::H1);
    uint64_t prev_rank = array[std::to_underlying(Rank::R_1)];
    for (Rank rank : {
             Rank::R_2,
             Rank::R_3,
             Rank::R_4,
             Rank::R_5,
             Rank::R_6,
             Rank::R_7,
             Rank::R_8,
         }) {
        array[std::to_underlying(rank)] = utils::shift(prev_rank, Direction::TOP);
        prev_rank                       = array[std::to_underlying(rank)];
    }
    return array;
}

// Rank bitboards lookup indexed using Rank enum
static constexpr std::array<uint64_t, BOARD_DIMENSION> RANK_BITBOARDS = createRankBitboards();

} // namespace bitcrusher

#endif // BITCRUSHER_FILE_RANK_BITBOARDS_HPP
