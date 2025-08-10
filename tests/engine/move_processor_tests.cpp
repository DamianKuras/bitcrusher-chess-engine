#include "bitboard_enums.hpp"
#include "bitboard_utils.hpp"
#include "board_state.hpp"
#include "move.hpp"
#include "move_processor.hpp"
#include <gtest/gtest.h>

using bitcrusher::BoardState;
using bitcrusher::Color;
using bitcrusher::Move;
using bitcrusher::MoveProcessor;
using bitcrusher::Piece;
using bitcrusher::PieceType;
using bitcrusher::Side;
using bitcrusher::Square;
using bitcrusher::utils::isSquareSet;

class MoveProcessorTestFixture : public ::testing::Test {
protected:
    BoardState    board{};
    MoveProcessor move_processor{};
    BoardState    pre_move_state{};
};

TEST_F(MoveProcessorTestFixture, QuietMove) {

    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare(Piece::WHITE_KNIGHT, Square::G1);
    const Move move = Move::createQuietMove(Square::G1, Square::F3, PieceType::KNIGHT);
    pre_move_state  = board;

    // test apply move
    move_processor.applyMove(board, move);

    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_KNIGHT, Square::F3));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_KNIGHT, Square::G1));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_KNIGHT, Square::G1));
    EXPECT_EQ(board.getHalfmoveClock(), 1);
    EXPECT_FALSE(board.isWhiteMove());

    // test undo move
    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, PawnDoublePush) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare(Piece::WHITE_PAWN, Square::E2);
    const auto move           = Move::createDoublePawnPushMove(Square::E2, Square::E4);
    BoardState pre_move_state = board;

    move_processor.applyMove(board, move);

    EXPECT_TRUE(isSquareSet(board.getBitboard(Piece::WHITE_PAWN), Square::E4));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::E3));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::E2));
    EXPECT_EQ(board.getEnPassantSquare(), Square::E3);
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, EnPassantCapture) {
    BoardState board;
    board.addPieceToSquare(Piece::WHITE_PAWN, Square::E5);
    board.addPieceToSquare(Piece::BLACK_PAWN, Square::F5);
    board.setSideToMove(Color::WHITE);
    board.setEnPassantSquare(Square::F6);
    const auto move           = Move::createEnPassantMove(Square::E5, Square::F6);
    BoardState pre_move_state = board;

    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::E5));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::F6));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::BLACK_PAWN, Square::F5));

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, CaptureMove) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare(Piece::WHITE_BISHOP, Square::C1);
    board.addPieceToSquare(Piece::BLACK_KNIGHT, Square::F4);
    BoardState pre_move_state = board;
    const auto move =
        Move::createCaptureMove(Square::C1, Square::F4, PieceType::BISHOP, PieceType::KNIGHT);

    move_processor.applyMove(board, move);
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_BISHOP, Square::F4));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_BISHOP, Square::C1));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::BLACK_KNIGHT, Square::F4));
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, WhiteKingsideCastling) {
    BoardState board;
    board.addPieceToSquare(Piece::WHITE_KING, Square::E1);
    board.addPieceToSquare(Piece::WHITE_ROOK, Square::H1);
    const auto move           = Move::createCastlingMove<Color::WHITE, Side::KINGSIDE>();
    BoardState pre_move_state = board;

    move_processor.applyMove(board, move);

    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_KING, Square::G1));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_KING, Square::E1));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_ROOK, Square::F1));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_ROOK, Square::H1));
    EXPECT_FALSE(board.hasWhiteKingsideCastlingRight());
    EXPECT_FALSE(board.hasWhiteQueensideCastlingRight());

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, BlackQueensideCastling) {
    BoardState board;
    board.setSideToMove(Color::BLACK);
    board.addPieceToSquare(Piece::BLACK_KING, Square::E8);
    board.addPieceToSquare(Piece::BLACK_ROOK, Square::A8);
    const auto move           = Move::createCastlingMove<Color::BLACK, Side::QUEENSIDE>();
    BoardState pre_move_state = board;

    move_processor.applyMove(board, move);

    // King should now be on C8
    EXPECT_TRUE(board.isPieceOnSquare(Piece::BLACK_KING, Square::C8));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::BLACK_KING, Square::E8));
    // Rook should have moved from A8 to D8
    EXPECT_TRUE(board.isPieceOnSquare(Piece::BLACK_ROOK, Square::D8));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::BLACK_ROOK, Square::A8));

    // Castling rights for black should be removed
    EXPECT_FALSE(board.hasBlackKingsideCastlingRight());
    EXPECT_FALSE(board.hasBlackQueensideCastlingRight());

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, PawnPromotionQueen) {
    BoardState board;
    board.addPieceToSquare(Piece::WHITE_PAWN, Square::A7);
    const auto move           = Move::createPromotionMove(Square::A7, Square::A8, PieceType::QUEEN);
    BoardState pre_move_state = board;

    move_processor.applyMove(board, move);

    // The pawn should be removed from E8 and replaced with a queen
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::A8));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_QUEEN, Square::A8));

    // Pawn promotion resets the halfmove clock
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, UpdateCastlingRightsOnKingMove) {
    BoardState board;
    board.addWhiteKingsideCastlingRight();
    board.addWhiteQueensideCastlingRight();
    board.addPieceToSquare(Piece::WHITE_KING, Square::E1);
    const auto move           = Move::createQuietMove(Square::E1, Square::F2, PieceType::KING);
    BoardState pre_move_state = board;

    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.hasWhiteKingsideCastlingRight());
    EXPECT_FALSE(board.hasWhiteQueensideCastlingRight());

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, UpdateCastlingRightsOnRookMove) {
    BoardState board;
    board.addPieceToSquare(Piece::WHITE_ROOK, Square::H1);
    board.setSideToMove(Color::WHITE);
    const auto move           = Move::createQuietMove(Square::H1, Square::H5, PieceType::ROOK);
    BoardState pre_move_state = board;

    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.hasWhiteKingsideCastlingRight());
    EXPECT_FALSE(board.hasWhiteQueensideCastlingRight());

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, FullmoveNumberIncrementBlackMove) {
    BoardState board;
    board.setSideToMove(Color::BLACK);
    const int fullmove_number = 5;
    board.setFullmoveNumber(fullmove_number);
    board.addPieceToSquare(Piece::BLACK_PAWN, Square::E7);
    const auto move           = Move::createDoublePawnPushMove(Square::E7, Square::E5);
    BoardState pre_move_state = board;

    move_processor.applyMove(board, move);

    // After Black's move, fullmove number should have incremented
    EXPECT_EQ(board.getFullmoveNumber(), 6);
    EXPECT_TRUE(board.isWhiteMove());

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, PawnPromotionKnightCapture) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare(Piece::WHITE_PAWN, Square::B7);
    board.addPieceToSquare(Piece::BLACK_ROOK, Square::C8);

    const auto move = Move::createPromotionCaptureMove(Square::B7, Square::C8, PieceType::KNIGHT,
                                                       PieceType::ROOK);
    BoardState pre_move_state = board;
    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::C8));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_KNIGHT, Square::C8));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::BLACK_ROOK, Square::C8));
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, PawnPromotionRookPromotion) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare(Piece::WHITE_PAWN, Square::D7);
    const auto move           = Move::createPromotionMove(Square::D7, Square::D8, PieceType::ROOK);
    BoardState pre_move_state = board;
    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::D8));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_ROOK, Square::D8));
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorTestFixture, PawnPromotionBishopPromotion) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare(Piece::WHITE_PAWN, Square::F7);
    const auto move = Move::createPromotionMove(Square::F7, Square::F8, PieceType::BISHOP);
    BoardState pre_move_state = board;
    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::F8));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_BISHOP, Square::F8));
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);
    EXPECT_EQ(board, pre_move_state);
}
