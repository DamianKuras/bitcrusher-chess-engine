#include "move.h"
#include <board_state.h>
#include <gtest/gtest.h>

TEST(isCaptureTest, QuietMoveIsNotCapture) {
    bitcrusher::Move move;
    move.flags = bitcrusher::MoveFlag::QUIET;
    EXPECT_FALSE(bitcrusher::isCapture(move));
}

TEST(isCaptureTest, CaptureFlagsAreCaptures) {
    bitcrusher::Move move;

    // Regular capture
    move.flags = bitcrusher::MoveFlag::CAPTURE;
    EXPECT_TRUE(bitcrusher::isCapture(move));

    // En passant capture
    move.flags = bitcrusher::MoveFlag::EN_PASSANT_CAPTURE;
    EXPECT_TRUE(bitcrusher::isCapture(move));

    // All promotion captures
    move.flags = bitcrusher::MoveFlag::KNIGHT_PROMOTION_CAPTURE;
    EXPECT_TRUE(bitcrusher::isCapture(move));

    move.flags = bitcrusher::MoveFlag::BISHOP_PROMOTION_CAPTURE;
    EXPECT_TRUE(bitcrusher::isCapture(move));

    move.flags = bitcrusher::MoveFlag::ROOK_PROMOTION_CAPTURE;
    EXPECT_TRUE(bitcrusher::isCapture(move));

    move.flags = bitcrusher::MoveFlag::QUEEN_PROMOTION_CAPTURE;
    EXPECT_TRUE(bitcrusher::isCapture(move));
}

TEST(isCaptureTest, NonCaptureMovesAreNotCaptures) {
    bitcrusher::Move move;

    move.flags = bitcrusher::MoveFlag::DOUBLE_PAWN_PUSH;
    EXPECT_FALSE(bitcrusher::isCapture(move));

    move.flags = bitcrusher::MoveFlag::KINGSIDE_CASTLE;
    EXPECT_FALSE(bitcrusher::isCapture(move));

    move.flags = bitcrusher::MoveFlag::QUEENSIDE_CASTLE;
    EXPECT_FALSE(bitcrusher::isCapture(move));

    // Non-capture promotions
    move.flags = bitcrusher::MoveFlag::KNIGHT_PROMOTION;
    EXPECT_FALSE(bitcrusher::isCapture(move));

    move.flags = bitcrusher::MoveFlag::BISHOP_PROMOTION;
    EXPECT_FALSE(bitcrusher::isCapture(move));

    move.flags = bitcrusher::MoveFlag::ROOK_PROMOTION;
    EXPECT_FALSE(bitcrusher::isCapture(move));

    move.flags = bitcrusher::MoveFlag::QUEEN_PROMOTION;
    EXPECT_FALSE(bitcrusher::isCapture(move));
}

TEST(isPromotionTest, AllPromotionFlagsArePromotions) {
    bitcrusher::Move move;

    // Non-capture promotions
    move.flags = bitcrusher::MoveFlag::KNIGHT_PROMOTION;
    EXPECT_TRUE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::BISHOP_PROMOTION;
    EXPECT_TRUE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::ROOK_PROMOTION;
    EXPECT_TRUE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::QUEEN_PROMOTION;
    EXPECT_TRUE(bitcrusher::isPromotion(move));

    // Capture promotions
    move.flags = bitcrusher::MoveFlag::KNIGHT_PROMOTION_CAPTURE;
    EXPECT_TRUE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::BISHOP_PROMOTION_CAPTURE;
    EXPECT_TRUE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::ROOK_PROMOTION_CAPTURE;
    EXPECT_TRUE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::QUEEN_PROMOTION_CAPTURE;
    EXPECT_TRUE(bitcrusher::isPromotion(move));
}

TEST(isPromotionTest, NonPromotionMovesAreNotPromotions) {
    bitcrusher::Move move;

    move.flags = bitcrusher::MoveFlag::QUIET;
    EXPECT_FALSE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::DOUBLE_PAWN_PUSH;
    EXPECT_FALSE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::KINGSIDE_CASTLE;
    EXPECT_FALSE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::QUEENSIDE_CASTLE;
    EXPECT_FALSE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::CAPTURE;
    EXPECT_FALSE(bitcrusher::isPromotion(move));

    move.flags = bitcrusher::MoveFlag::EN_PASSANT_CAPTURE;
    EXPECT_FALSE(bitcrusher::isPromotion(move));
}