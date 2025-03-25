#include "bitboard_conversions.hpp"
#include "bitboard_enums.hpp"
#include "bitboard_offsets.hpp"
#include "file_rank_bitboards.hpp"
#include <gtest/gtest.h>
#include <utility>

TEST(BitcrusherOffsetTest, CalculateOffsetSingleDirection) {
    int offset = bitcrusher::offset::calculateOffset<bitcrusher::Direction::TOP>();
    EXPECT_EQ(offset, bitcrusher::convert::toDelta(bitcrusher::Direction::TOP));
}

TEST(BitcrusherOffsetTest, CalculateOffsetMultipleDirections) {
    int offset = bitcrusher::offset::calculateOffset<bitcrusher::Direction::TOP,
                                                     bitcrusher::Direction::RIGHT>();
    EXPECT_EQ(offset, bitcrusher::convert::toDelta(bitcrusher::Direction::TOP) +
                          bitcrusher::convert::toDelta(bitcrusher::Direction::RIGHT));
}

TEST(BitcrusherOffsetTest, CalculateOffsetRepeatedDirection) {
    int offset = bitcrusher::offset::calculateOffset<bitcrusher::offset::RepeatedDirection{
        bitcrusher::Direction::TOP, 2}>();
    EXPECT_EQ(offset, 2 * bitcrusher::convert::toDelta(bitcrusher::Direction::TOP));
}

TEST(BitcrusherOffsetTest, ComputeWrapPreventionMaskSingleLeft) {
    uint64_t mask = bitcrusher::offset::computeWrapPreventionMask<bitcrusher::Direction::LEFT>();

    EXPECT_EQ(mask, ~bitcrusher::FILE_BITBOARDS[std::to_underlying(bitcrusher::File::A)]);
}

TEST(BitcrusherOffsetTest, ShiftBitboardNoWrapSingleDirectionFallofLeft) {
    const uint64_t bitboard = bitcrusher::convert::toBitboard(bitcrusher::Square::A1);

    const uint64_t shifted_bitboard =
        bitcrusher::offset::shiftBitboardNoWrap<bitcrusher::Direction::LEFT>(bitboard);

    EXPECT_EQ(shifted_bitboard, bitcrusher::EMPTY_BITBOARD);
}

TEST(BitcrusherOffsetTest, ShiftBitboardNoWrapRepeatedDirectionFallofRight) {
    const uint64_t bitboard = bitcrusher::convert::toBitboard(bitcrusher::Square::D3);

    const uint64_t shifted_bitboard =
        bitcrusher::offset::shiftBitboardNoWrap<bitcrusher::offset::RepeatedDirection{
            bitcrusher::Direction::RIGHT, 5}>(bitboard);

    EXPECT_EQ(shifted_bitboard, bitcrusher::EMPTY_BITBOARD);
}

TEST(BitcrusherOffsetTest, ShiftBitboardNoWrapSingleDirectionCorrect) {
    uint64_t bitboard = bitcrusher::convert::toBitboard(bitcrusher::Square::B2);

    uint64_t shifted_bitboard =
        bitcrusher::offset::shiftBitboardNoWrap<bitcrusher::Direction::LEFT>(bitboard);

    EXPECT_EQ(shifted_bitboard, bitcrusher::convert::toBitboard(bitcrusher::Square::A2));
}

TEST(BitcrusherOffsetTest, ShiftBitboardNoWrapRepeatedDirectionCorrect) {
    const uint64_t bitboard = bitcrusher::convert::toBitboard(bitcrusher::Square::H1);

    const uint64_t shifted_bitboard = bitcrusher::offset::shiftBitboardNoWrap<
        bitcrusher::offset::RepeatedDirection{bitcrusher::Direction::TOP, 7},
        bitcrusher::offset::RepeatedDirection{bitcrusher::Direction::LEFT, 7}>(bitboard);

    EXPECT_EQ(shifted_bitboard, bitcrusher::convert::toBitboard(bitcrusher::Square::A8));
}

TEST(BitcrusherOffsetTest, ShiftBitboardNoWrapMixedDirectionsCorrect) {
    const uint64_t bitboard = bitcrusher::convert::toBitboard(bitcrusher::Square::H1);

    const uint64_t shifted_bitboard =
        bitcrusher::offset::shiftBitboardNoWrap<bitcrusher::offset::RepeatedDirection{
                                                    bitcrusher::Direction::TOP, 7},
                                                bitcrusher::Direction::LEFT>(bitboard);

    EXPECT_EQ(shifted_bitboard, bitcrusher::convert::toBitboard(bitcrusher::Square::G8));
}

TEST(BitcrusherOffsetTest, ShiftBitboardNoWrapShiftUpSingle) {
    const uint64_t initial  = bitcrusher::convert::toBitboard(bitcrusher::Square::B2);
    const uint64_t expected = bitcrusher::convert::toBitboard(bitcrusher::Square::B3);

    uint64_t shifted_bitboard =
        bitcrusher::offset::shiftBitboardNoWrap<bitcrusher::Direction::TOP>(initial);

    EXPECT_EQ(shifted_bitboard, expected);
}

TEST(BitcrusherOffsetTest, ShiftBitboardNoWrapShiftUpFallof) {
    const uint64_t initial = bitcrusher::convert::toBitboard(bitcrusher::Square::B8);

    uint64_t shifted_bitboard =
        bitcrusher::offset::shiftBitboardNoWrap<bitcrusher::Direction::TOP>(initial);

    EXPECT_EQ(shifted_bitboard, bitcrusher::EMPTY_BITBOARD);
}

TEST(BitcrusherOffsetTest, ShiftBitboardNoWrapShiftBottomSingle) {
    const uint64_t initial  = bitcrusher::convert::toBitboard(bitcrusher::Square::B3);
    const uint64_t expected = bitcrusher::convert::toBitboard(bitcrusher::Square::B2);

    uint64_t shifted_bitboard =
        bitcrusher::offset::shiftBitboardNoWrap<bitcrusher::Direction::BOTTOM>(initial);

    EXPECT_EQ(shifted_bitboard, expected);
}

TEST(BitcrusherOffsetTest, ShiftBitboardNoWrapShiftBottomFallof) {
    const uint64_t initial = bitcrusher::convert::toBitboard(bitcrusher::Square::B1);

    uint64_t shifted_bitboard =
        bitcrusher::offset::shiftBitboardNoWrap<bitcrusher::Direction::BOTTOM>(initial);

    EXPECT_EQ(shifted_bitboard, bitcrusher::EMPTY_BITBOARD);
}