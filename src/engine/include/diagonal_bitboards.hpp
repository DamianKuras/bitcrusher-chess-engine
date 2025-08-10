#ifndef BITCRUSHER_DIAGONAL_BITBOARDS_HPP
#define BITCRUSHER_DIAGONAL_BITBOARDS_HPP

#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"

namespace bitcrusher {

constexpr std::array BOTTOM_DIAGONALS{Diagonal::A7G1, Diagonal::A6F1, Diagonal::A5E1,
                                      Diagonal::A4D1, Diagonal::A3C1, Diagonal::A2B1,
                                      Diagonal::A1};

constexpr std::array RIGHT_DIAGONALS{Diagonal::B8H2, Diagonal::C8H3, Diagonal::D8H4, Diagonal::E8H5,
                                     Diagonal::F8H6, Diagonal::G8H7, Diagonal::H8};

consteval std::array<uint64_t, DIAGONAL_COUNT> createDiagonalBitboards() {
    std::array<uint64_t, DIAGONAL_COUNT> array{};
    array[std::to_underlying(Diagonal::A8H1)] =
        convert::toBitboard(Square::A8, Square::B7, Square::C6, Square::D5, Square::E4, Square::F3,
                            Square::G2, Square::H1);
    uint64_t prev_diagonal = array[std::to_underlying(Diagonal::A8H1)];
    for (Diagonal diag : BOTTOM_DIAGONALS) {
        array[std::to_underlying(diag)] =
            offset::shiftBitboardNoWrap<Direction::BOTTOM>(prev_diagonal);
        prev_diagonal = array[std::to_underlying(diag)];
    }
    prev_diagonal = array[std::to_underlying(Diagonal::A8H1)];
    for (auto diag : RIGHT_DIAGONALS) {
        array[std::to_underlying(diag)] =
            offset::shiftBitboardNoWrap<Direction::RIGHT>(prev_diagonal);

        prev_diagonal = array[std::to_underlying(diag)];
    }
    return array;
}

// Diagonal bitboards lookup indexed using Diagonal enum
constexpr std::array<uint64_t, DIAGONAL_COUNT> PRECOMPUTED_DIAGONAL_BITBOARDS =
    createDiagonalBitboards();

consteval std::array<uint64_t, DIAGONAL_COUNT> createCounterDiagonalBitboards() {
    std::array<uint64_t, DIAGONAL_COUNT> array{};
    array[std::to_underlying(CounterDiagonal::A1H8)] =
        convert::toBitboard(Square::A1, Square::B2, Square::C3, Square::D4, Square::E5, Square::F6,
                            Square::G7, Square::H8);
    uint64_t prev_counter_diagonal = array[std::to_underlying(CounterDiagonal::A1H8)];
    for (CounterDiagonal counter_diag :
         {CounterDiagonal::A2G8, CounterDiagonal::A3F8, CounterDiagonal::A4E8,
          CounterDiagonal::A5D8, CounterDiagonal::A6C8, CounterDiagonal::A7B8,
          CounterDiagonal::A8}) {
        array[std::to_underlying(counter_diag)] =
            offset::shiftBitboardNoWrap<Direction::TOP>(prev_counter_diagonal);
        prev_counter_diagonal = array[std::to_underlying(counter_diag)];
    }
    prev_counter_diagonal = array[std::to_underlying(CounterDiagonal::A1H8)];

    for (CounterDiagonal counter_diag :
         {CounterDiagonal::B1H7, CounterDiagonal::C1H6, CounterDiagonal::D1H5,
          CounterDiagonal::E1H4, CounterDiagonal::F1H3, CounterDiagonal::G1H2,
          CounterDiagonal::H1}) {
        array[std::to_underlying(counter_diag)] =
            offset::shiftBitboardNoWrap<Direction::RIGHT>(prev_counter_diagonal);
        prev_counter_diagonal = array[std::to_underlying(counter_diag)];
    }
    return array;
}

// Counter diagonal bitboards lookup indexed using CounterDiagonal enum
constexpr std::array<uint64_t, DIAGONAL_COUNT> PRECOMPUTED_COUNTER_DIAGONAL_BITBOARDS =
    createCounterDiagonalBitboards();

consteval std::array<uint64_t, SQUARE_COUNT> createSquareToDiagonalBitboard() {
    std::array<uint64_t, SQUARE_COUNT> array{};
    for (Square square = Square::A8; square <= Square::H1; square += 1) {
        Diagonal diagonal = convert::toDiagonal(square);

        array[std::to_underlying(square)] =
            PRECOMPUTED_DIAGONAL_BITBOARDS[std::to_underlying(diagonal)];
    }
    return array;
}

constexpr std::array<uint64_t, SQUARE_COUNT> SQUARE_TO_DIAGONAL_BITBOARD =
    createSquareToDiagonalBitboard();

consteval std::array<uint64_t, SQUARE_COUNT> createSquareToCounterDiagonalBitboard() {
    std::array<uint64_t, SQUARE_COUNT> array{};
    for (Square square = Square::A8; square <= Square::H1; square += 1) {
        CounterDiagonal counter_diagonal = convert::toCounterDiagonal(square);

        array[std::to_underlying(square)] =
            PRECOMPUTED_COUNTER_DIAGONAL_BITBOARDS[std::to_underlying(counter_diagonal)];
    }
    return array;
}

const std::array<uint64_t, SQUARE_COUNT> SQUARE_TO_COUNTER_DIAGONAL_BITBOARD =
    createSquareToCounterDiagonalBitboard();

} // namespace bitcrusher

#endif // BITCRUSHER_DIAGONAL_BITBOARDS_HPP