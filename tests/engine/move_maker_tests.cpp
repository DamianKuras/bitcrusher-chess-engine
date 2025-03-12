#include "bitboard.hpp"
#include "board_state.hpp"
#include "move.hpp"
#include "move_maker.hpp"
#include <gtest/gtest.h>

TEST(makeMoveTest, QuietMove) {
    bitcrusher::BoardState board_state;
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::WHITE_KNIGHT>(),
                          bitcrusher::Square::G1);
    const auto move = bitcrusher::Move::createQuietMove(
        bitcrusher::Square::G1, bitcrusher::Square::F3, bitcrusher::Piece::WHITE_KNIGHT);

    bitcrusher::makeMove(board_state, move);

    // White knight has moved to G1
    EXPECT_TRUE(bitcrusher::isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_KNIGHT>(),
                                        bitcrusher::Square::F3));
    EXPECT_FALSE(bitcrusher::isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_KNIGHT>(),
                                         bitcrusher::Square::G1));
    // No pawn move or capture should increment halfmove clock
    EXPECT_EQ(board_state.getHalfmoveClock(), 1);
    // Side to move should have toggled
    EXPECT_FALSE(board_state.isWhiteMove());
}

TEST(makeMoveTest, PawnDoublePush) {
    bitcrusher::BoardState board_state;
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::WHITE_PAWN>(),
                          bitcrusher::Square::E2);
    const auto move = bitcrusher::Move::createDoublePawnPushMove(
        bitcrusher::Square::E2, bitcrusher::Square::E4, bitcrusher::Piece::WHITE_PAWN);

    bitcrusher::makeMove(board_state, move);

    // White pawn has moved to E4
    EXPECT_TRUE(bitcrusher::isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_PAWN>(),
                                        bitcrusher::Square::E4));
    EXPECT_FALSE(bitcrusher::isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_PAWN>(),
                                         bitcrusher::Square::E2));
    // En passant set correctly
    EXPECT_EQ(board_state.getEnPassantSquare(), bitcrusher::Square::E3);

    // Pawn moves reset the halfmove clock.
    EXPECT_EQ(board_state.getHalfmoveClock(), 0);
}

TEST(makeMoveTest, EnPassantCapture) {
    bitcrusher::BoardState board_state;
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::WHITE_PAWN>(),
                          bitcrusher::Square::E5);
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::BLACK_PAWN>(),
                          bitcrusher::Square::F5);
    board_state.setEnPassantSquare(bitcrusher::Square::F6);
    const auto move = bitcrusher::Move::createEnPassantMove(
        bitcrusher::Square::E5, bitcrusher::Square::F6, bitcrusher::Piece::WHITE_PAWN);

    bitcrusher::makeMove(board_state, move);

    // White pawn has moved to F6
    EXPECT_FALSE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_PAWN>(),
                             bitcrusher::Square::E5));
    EXPECT_TRUE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_PAWN>(),
                            bitcrusher::Square::F6));
    // Black pawn is removed from F5
    EXPECT_FALSE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::BLACK_PAWN>(),
                             bitcrusher::Square::F5));
}

TEST(makeMoveTest, CaptureMove) {
    bitcrusher::BoardState board_state;
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::WHITE_BISHOP>(),
                          bitcrusher::Square::C1);
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::BLACK_KNIGHT>(),
                          bitcrusher::Square::F4);
    const auto move = bitcrusher::Move::createCaptureMove(
        bitcrusher::Square::C1, bitcrusher::Square::F4, bitcrusher::Piece::WHITE_BISHOP,
        bitcrusher::Piece::BLACK_KNIGHT);

    bitcrusher::makeMove(board_state, move);

    // Bishop moved to F4
    EXPECT_TRUE(bitcrusher::isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_BISHOP>(),
                                        bitcrusher::Square::F4));
    EXPECT_FALSE(bitcrusher::isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_BISHOP>(),
                                         bitcrusher::Square::C1));
    // Knight is captured
    EXPECT_FALSE(bitcrusher::isSquareSet(board_state.getBitboard<bitcrusher::Piece::BLACK_KNIGHT>(),
                                         bitcrusher::Square::F4));
    // Capture resets halfmove clock
    EXPECT_EQ(board_state.getHalfmoveClock(), 0);
}

TEST(makeMoveTest, WhiteKingsideCastling) {
    bitcrusher::BoardState board_state;
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::WHITE_KING>(),
                          bitcrusher::Square::E1);
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::WHITE_ROOK>(),
                          bitcrusher::Square::H1);
    const auto move = bitcrusher::Move::createCastlingMove<bitcrusher::SideToMove::WHITE,
                                                           bitcrusher::CastleSide::KINGSIDE>();

    bitcrusher::makeMove(board_state, move);

    // King should now be on G1
    EXPECT_TRUE(bitcrusher::isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_KING>(),
                                        bitcrusher::Square::G1));
    EXPECT_FALSE(bitcrusher::isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_KING>(),
                                         bitcrusher::Square::E1));
    // Rook should have moved from H1 to F1
    EXPECT_TRUE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_ROOK>(),
                            bitcrusher::Square::F1));
    EXPECT_FALSE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_ROOK>(),
                             bitcrusher::Square::H1));

    // Castling rights for white should be removed
    EXPECT_FALSE(board_state.hasWhiteKingsideCastlingRight());
    EXPECT_FALSE(board_state.hasWhiteQueensideCastlingRight());
}

TEST(makeMoveTest, BlackQueensideCastling) {
    bitcrusher::BoardState board_state;
    board_state.setIsWhiteMove(false);
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::BLACK_KING>(),
                          bitcrusher::Square::E8);
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::BLACK_ROOK>(),
                          bitcrusher::Square::A8);
    const auto move = bitcrusher::Move::createCastlingMove<bitcrusher::SideToMove::BLACK,
                                                           bitcrusher::CastleSide::QUEENSIDE>();

    bitcrusher::makeMove(board_state, move);

    // King should now be on C8
    EXPECT_TRUE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::BLACK_KING>(),
                            bitcrusher::Square::C8));
    EXPECT_FALSE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::BLACK_KING>(),
                             bitcrusher::Square::E8));
    // Rook should have moved from A8 to D8
    EXPECT_TRUE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::BLACK_ROOK>(),
                            bitcrusher::Square::D8));
    EXPECT_FALSE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::BLACK_ROOK>(),
                             bitcrusher::Square::A8));

    // Castling rights for black should be removed
    EXPECT_FALSE(board_state.hasBlackKingsideCastlingRight());
    EXPECT_FALSE(board_state.hasBlackQueensideCastlingRight());
}

TEST(makeMoveTest, PawnPromotionQueen) {
    bitcrusher::BoardState board_state;
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::WHITE_PAWN>(),
                          bitcrusher::Square::A7);
    const auto move = bitcrusher::Move::createPromotionMove(
        bitcrusher::Square::A7, bitcrusher::Square::A8, bitcrusher::Piece::WHITE_QUEEN);

    bitcrusher::makeMove(board_state, move);

    // The pawn should be removed from E8 and replaced with a queen
    EXPECT_FALSE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_PAWN>(),
                             bitcrusher::Square::A8));
    EXPECT_TRUE(isSquareSet(board_state.getBitboard<bitcrusher::Piece::WHITE_QUEEN>(),
                            bitcrusher::Square::A8));

    // Pawn promotion resets the halfmove clock
    EXPECT_EQ(board_state.getHalfmoveClock(), 0);
}

TEST(makeMoveTest, UpdateCastlingRightsOnKingMove) {
    bitcrusher::BoardState board_state;
    board_state.addWhiteKingsideCastlingRight();
    board_state.addWhiteQueensideCastlingRight();
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::WHITE_KING>(),
                          bitcrusher::Square::E1);
    const auto move = bitcrusher::Move::createQuietMove(
        bitcrusher::Square::E1, bitcrusher::Square::F2, bitcrusher::Piece::WHITE_KING);

    bitcrusher::makeMove(board_state, move);

    EXPECT_FALSE(board_state.hasWhiteKingsideCastlingRight());
    EXPECT_FALSE(board_state.hasWhiteQueensideCastlingRight());
}

TEST(makeMoveTest, UpdateCastlingRightsOnRookMove) {
    bitcrusher::BoardState board_state;
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::WHITE_ROOK>(),
                          bitcrusher::Square::H1);

    const auto move = bitcrusher::Move::createQuietMove(
        bitcrusher::Square::H1, bitcrusher::Square::H5, bitcrusher::Piece::WHITE_ROOK);

    bitcrusher::makeMove(board_state, move);

    EXPECT_FALSE(board_state.hasWhiteKingsideCastlingRight());
    EXPECT_FALSE(board_state.hasWhiteQueensideCastlingRight());
}

TEST(makeMoveTest, FullmoveNumberIncrementBlackMove) {
    bitcrusher::BoardState board_state;
    board_state.setIsWhiteMove(false);
    board_state.setFullmoveNumber(5);
    bitcrusher::setSquare(board_state.getBitboard<bitcrusher::Piece::BLACK_PAWN>(),
                          bitcrusher::Square::E7);
    const auto move = bitcrusher::Move::createDoublePawnPushMove(
        bitcrusher::Square::E7, bitcrusher::Square::E5, bitcrusher::Piece::BLACK_PAWN);

    bitcrusher::makeMove(board_state, move);

    // After Black's move, fullmove number should have incremented
    EXPECT_EQ(board_state.getFullmoveNumber(), 6);
    EXPECT_TRUE(board_state.isWhiteMove());
}
