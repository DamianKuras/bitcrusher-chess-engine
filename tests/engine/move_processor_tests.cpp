#include "bitboard_enums.hpp"
#include "bitboard_utils.hpp"
#include "board_state.hpp"
#include "fen_formatter.hpp"
#include "move.hpp"
#include "move_processor.hpp"
#include "zobrist_hash_keys.hpp"
#include <gtest/gtest.h>
#include <utility>

using bitcrusher::BoardState;
using bitcrusher::CastlingRights;
using bitcrusher::Color;
using bitcrusher::Move;
using bitcrusher::MoveProcessor;
using bitcrusher::Piece;
using bitcrusher::PieceType;
using bitcrusher::Side;
using bitcrusher::Square;
using bitcrusher::ZobristKeys;
using bitcrusher::utils::isSquareSet;

class MoveProcessorFixture : public ::testing::Test {
protected:
    BoardState    board{};
    MoveProcessor move_processor{};
    BoardState    pre_move_state{};

    void SetUp() override {
        board          = BoardState{};
        move_processor = MoveProcessor{};
        pre_move_state = BoardState{};
    }

    static void SetUpTestSuite() { ZobristKeys::init(12345); }

    MoveProcessorFixture() { ZobristKeys::init(12345); }
};

TEST_F(MoveProcessorFixture, QuietMove) {
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::KNIGHT, Square::G1);
    const Move move = Move::createQuietMove(Square::G1, Square::F3, PieceType::KNIGHT);
    pre_move_state  = board;

    // test apply move
    move_processor.applyMove(board, move);

    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_KNIGHT, Square::F3));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_KNIGHT, Square::G1));
    EXPECT_EQ(board.getHalfmoveClock(), 1);
    EXPECT_FALSE(board.isWhiteMove());

    // test undo move
    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, PawnDoublePush) {
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::E2);
    const Move move = Move::createDoublePawnPushMove(Square::E2, Square::E4);
    pre_move_state  = board;

    move_processor.applyMove(board, move);

    EXPECT_TRUE(isSquareSet(board.getBitboard(Piece::WHITE_PAWN), Square::E4));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::E3));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::E2));
    EXPECT_EQ(board.getEnPassantSquare(), Square::E3);
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, EnPassantCapture) {
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::E5);
    board.addPieceToSquare<Color::BLACK>(PieceType::PAWN, Square::F5);
    board.setSideToMove(Color::WHITE);
    board.setEnPassantSquare(Square::F6);
    const Move move = Move::createEnPassantMove(Square::E5, Square::F6);
    pre_move_state  = board;

    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::E5));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::F6));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::BLACK_PAWN, Square::F5));

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, CaptureMove) {
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::BISHOP, Square::C1);
    board.addPieceToSquare<Color::BLACK>(PieceType::KNIGHT, Square::F4);
    pre_move_state = board;
    const Move move =
        Move::createCaptureMove(Square::C1, Square::F4, PieceType::BISHOP, PieceType::KNIGHT);

    move_processor.applyMove(board, move);

    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_BISHOP, Square::F4));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_BISHOP, Square::C1));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::BLACK_KNIGHT, Square::F4));
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, WhiteKingsideCastling) {
    board.addPieceToSquare<Color::WHITE>(PieceType::KING, Square::E1);
    board.addPieceToSquare<Color::WHITE>(PieceType::ROOK, Square::H1);
    const Move move = Move::createCastlingMove<Color::WHITE, Side::KINGSIDE>();
    board.addCastlingRights<CastlingRights::WHITE_KINGSIDE>();
    pre_move_state = board;

    move_processor.applyMove(board, move);

    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_KING, Square::G1));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_KING, Square::E1));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_ROOK, Square::F1));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_ROOK, Square::H1));
    EXPECT_FALSE(board.hasCastlingRights<CastlingRights::WHITE_KINGSIDE>());

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, BlackQueensideCastling) {
    board.setSideToMove(Color::BLACK);
    board.addPieceToSquare<Color::BLACK>(PieceType::KING, Square::E8);
    board.addPieceToSquare<Color::BLACK>(PieceType::ROOK, Square::A8);
    const Move move = Move::createCastlingMove<Color::BLACK, Side::QUEENSIDE>();
    pre_move_state  = board;

    move_processor.applyMove(board, move);

    // King should now be on C8
    EXPECT_TRUE(board.isPieceOnSquare(Piece::BLACK_KING, Square::C8));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::BLACK_KING, Square::E8));
    // Rook should have moved from A8 to D8
    EXPECT_TRUE(board.isPieceOnSquare(Piece::BLACK_ROOK, Square::D8));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::BLACK_ROOK, Square::A8));

    // Castling rights for black should be removed
    EXPECT_FALSE(board.hasCastlingRights<CastlingRights::BLACK_CASTLING_RIGHTS>());

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, PawnPromotionQueen) {
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::A7);
    const Move move = Move::createPromotionMove(Square::A7, Square::A8, PieceType::QUEEN);
    pre_move_state  = board;

    move_processor.applyMove(board, move);

    // The pawn should be removed from A8 and replaced with a queen
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::A7));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::A8));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_QUEEN, Square::A8));

    // Pawn promotion resets the halfmove clock
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, UpdateCastlingRightsOnKingMove) {
    board.addCastlingRights<CastlingRights::WHITE_CASTLING_RIGHTS>();
    board.addPieceToSquare<Color::WHITE>(PieceType::KING, Square::E1);
    const Move move = Move::createQuietMove(Square::E1, Square::F2, PieceType::KING);
    pre_move_state  = board;

    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.hasCastlingRights<CastlingRights::WHITE_CASTLING_RIGHTS>());

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, UpdateCastlingRightsOnRookMove) {
    board.addPieceToSquare<Color::WHITE>(PieceType::ROOK, Square::H1);
    board.setSideToMove(Color::WHITE);
    const Move move = Move::createQuietMove(Square::H1, Square::H5, PieceType::ROOK);
    pre_move_state  = board;

    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.hasCastlingRights<CastlingRights::WHITE_CASTLING_RIGHTS>());

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, FullmoveNumberIncrementBlackMove) {
    board.setSideToMove(Color::BLACK);
    const int fullmove_number = 5;
    board.setFullmoveNumber(fullmove_number);
    board.addPieceToSquare<Color::BLACK>(PieceType::PAWN, Square::E7);
    const Move move = Move::createDoublePawnPushMove(Square::E7, Square::E5);
    pre_move_state  = board;

    move_processor.applyMove(board, move);

    // After Black's move, fullmove number should have incremented
    EXPECT_EQ(board.getFullmoveNumber(), 6);
    EXPECT_TRUE(board.isWhiteMove());

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, PawnPromotionKnightCapture) {
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::B7);
    board.addPieceToSquare<Color::BLACK>(PieceType::ROOK, Square::C8);

    const Move move = Move::createPromotionCaptureMove(Square::B7, Square::C8, PieceType::KNIGHT,
                                                       PieceType::ROOK);
    pre_move_state  = board;

    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::C8));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_KNIGHT, Square::C8));
    EXPECT_FALSE(board.isPieceOnSquare(Piece::BLACK_ROOK, Square::C8));
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, PawnPromotionRookPromotion) {
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::D7);
    const Move move = Move::createPromotionMove(Square::D7, Square::D8, PieceType::ROOK);
    pre_move_state  = board;

    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::D8));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_ROOK, Square::D8));
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, PawnPromotionBishopPromotion) {
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::F7);
    const Move move = Move::createPromotionMove(Square::F7, Square::F8, PieceType::BISHOP);
    pre_move_state  = board;

    move_processor.applyMove(board, move);

    EXPECT_FALSE(board.isPieceOnSquare(Piece::WHITE_PAWN, Square::F8));
    EXPECT_TRUE(board.isPieceOnSquare(Piece::WHITE_BISHOP, Square::F8));
    EXPECT_EQ(board.getHalfmoveClock(), 0);

    move_processor.undoMove(board, move);

    EXPECT_EQ(board, pre_move_state);
}

TEST_F(MoveProcessorFixture, ThreeTimeRepetition) {
    parseFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", board);
    const Move white_knight_jump_forward =
        Move::createQuietMove(Square::G1, Square::F3, PieceType::KNIGHT);
    const Move white_knight_jump_backward =
        Move::createQuietMove(Square::F3, Square::G1, PieceType::KNIGHT);
    const Move black_knight_jump_forward =
        Move::createQuietMove(Square::G8, Square::F6, PieceType::KNIGHT);
    const Move black_knight_jump_backward =
        Move::createQuietMove(Square::F6, Square::G8, PieceType::KNIGHT);

    move_processor.applyMove(board, white_knight_jump_forward);
    move_processor.applyMove(board, black_knight_jump_forward);
    move_processor.applyMove(board, white_knight_jump_backward);
    move_processor.applyMove(board, black_knight_jump_backward);
    move_processor.applyMove(board, white_knight_jump_forward);
    move_processor.applyMove(board, black_knight_jump_forward);

    EXPECT_FALSE(move_processor.hasCurrentPositionRepeated3Times());

    move_processor.applyMove(board, white_knight_jump_backward);
    move_processor.applyMove(board, black_knight_jump_backward);

    EXPECT_TRUE(move_processor.hasCurrentPositionRepeated3Times());
}

TEST_F(MoveProcessorFixture, ThreeTimeRepetitionBrokenByPawnMove) {
    parseFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", board);
    const Move white_knight_jump_forward =
        Move::createQuietMove(Square::G1, Square::F3, PieceType::KNIGHT);
    const Move white_knight_jump_backward =
        Move::createQuietMove(Square::F3, Square::G1, PieceType::KNIGHT);
    const Move black_knight_jump_forward =
        Move::createQuietMove(Square::G8, Square::F6, PieceType::KNIGHT);
    const Move black_knight_jump_backward =
        Move::createQuietMove(Square::F6, Square::G8, PieceType::KNIGHT);

    const Move white_pawn_forward = Move::createQuietMove(Square::D2, Square::D3, PieceType::PAWN);

    move_processor.applyMove(board, white_knight_jump_forward);
    move_processor.applyMove(board, black_knight_jump_forward);
    move_processor.applyMove(board, white_knight_jump_backward);
    move_processor.applyMove(board, black_knight_jump_backward);
    move_processor.applyMove(board, white_knight_jump_forward);
    move_processor.applyMove(board, black_knight_jump_forward);

    EXPECT_FALSE(move_processor.hasCurrentPositionRepeated3Times());

    move_processor.applyMove(board, white_pawn_forward);
    move_processor.applyMove(board, black_knight_jump_backward);
    move_processor.applyMove(board, white_knight_jump_backward);

    EXPECT_FALSE(move_processor.hasCurrentPositionRepeated3Times());
}

TEST_F(MoveProcessorFixture, ThreeTimeRepetitionBrokenByCastlingRights) {
    parseFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", board);
    const Move white_king_forward  = Move::createQuietMove(Square::E1, Square::E2, PieceType::KING);
    const Move white_king_backward = Move::createQuietMove(Square::E2, Square::E1, PieceType::KING);
    const Move black_king_forward  = Move::createQuietMove(Square::E8, Square::E7, PieceType::KING);
    const Move black_king_backward = Move::createQuietMove(Square::E7, Square::E8, PieceType::KING);

    Move white_pawn_forward = Move::createQuietMove(Square::D2, Square::D3, PieceType::PAWN);

    move_processor.applyMove(board, white_king_forward); // Removes white castling rights.
    move_processor.applyMove(board, black_king_forward); // Removes black castling rights.
    move_processor.applyMove(board, white_king_backward);
    move_processor.applyMove(board, black_king_backward);
    move_processor.applyMove(board, white_king_forward);
    move_processor.applyMove(board, black_king_forward);

    EXPECT_FALSE(move_processor.hasCurrentPositionRepeated3Times());

    move_processor.applyMove(board, white_king_backward);
    move_processor.applyMove(board, black_king_backward);

    EXPECT_FALSE(move_processor.hasCurrentPositionRepeated3Times());
}

TEST_F(MoveProcessorFixture, RepetitionWithDifferentMoveOrder) {
    parseFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", board);
    const Move white_knight_jump_forward =
        Move::createQuietMove(Square::G1, Square::F3, PieceType::KNIGHT);
    const Move white_knight_jump_backward =
        Move::createQuietMove(Square::F3, Square::G1, PieceType::KNIGHT);
    const Move black_knight_jump_forward =
        Move::createQuietMove(Square::G8, Square::F6, PieceType::KNIGHT);
    const Move black_knight_jump_backward =
        Move::createQuietMove(Square::F6, Square::G8, PieceType::KNIGHT);

    const Move white_bishop_forward =
        Move::createQuietMove(Square::F1, Square::C4, PieceType::BISHOP);
    const Move white_bishop_backward =
        Move::createQuietMove(Square::C4, Square::F1, PieceType::BISHOP);
    const Move black_bishop_forward =
        Move::createQuietMove(Square::F8, Square::C5, PieceType::BISHOP);
    const Move black_bishop_backward =
        Move::createQuietMove(Square::C5, Square::F8, PieceType::BISHOP);

    move_processor.applyMove(board, white_knight_jump_forward);
    move_processor.applyMove(board, black_knight_jump_forward);

    // Move bishops forward and back for both colors to break move order
    move_processor.applyMove(board, white_bishop_forward);
    move_processor.applyMove(board, black_bishop_forward);

    move_processor.applyMove(board, white_bishop_backward);
    move_processor.applyMove(board, black_bishop_backward);

    move_processor.applyMove(board, white_bishop_forward);
    move_processor.applyMove(board, black_bishop_forward);

    move_processor.applyMove(board, white_knight_jump_backward);
    move_processor.applyMove(board, black_knight_jump_backward);

    EXPECT_FALSE(move_processor.hasCurrentPositionRepeated3Times());

    move_processor.applyMove(board, white_knight_jump_forward);
    move_processor.applyMove(board, black_knight_jump_forward);

    move_processor.applyMove(board, white_knight_jump_backward);
    move_processor.applyMove(board, black_knight_jump_backward);

    move_processor.applyMove(board, white_knight_jump_forward);
    move_processor.applyMove(board, black_knight_jump_forward);

    move_processor.applyMove(board, white_knight_jump_backward);
    move_processor.applyMove(board, black_knight_jump_backward);
    // Third Repetition

    EXPECT_TRUE(move_processor.hasCurrentPositionRepeated3Times());
}