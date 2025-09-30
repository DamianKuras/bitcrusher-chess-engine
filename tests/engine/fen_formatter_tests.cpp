
#include "bitboard_enums.hpp"
#include "fen_formatter.hpp"
#include "move.hpp"
#include <board_state.hpp>
#include <gtest/gtest.h>
#include <utility>

using bitcrusher::BoardState;
using bitcrusher::CastlingRights;
using bitcrusher::Color;
using bitcrusher::EMPTY_BITBOARD;
using bitcrusher::Move;
using bitcrusher::parseFEN;
using bitcrusher::Piece;
using bitcrusher::PieceType;
using bitcrusher::Square;

TEST(parseFENTest, StartingPosition) {
    // Inital chess position FEN
    std::string_view fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    BoardState       board_state;

    parseFEN(fen, board_state);

    // White pieces
    EXPECT_EQ(board_state.getBitboard(Piece::WHITE_PAWN),
              bitcrusher::convert::toBitboard(Square::A2, Square::B2, Square::C2, Square::D2,
                                              Square::E2, Square::F2, Square::G2, Square::H2));

    EXPECT_EQ(board_state.getBitboard(Piece::WHITE_KNIGHT),
              bitcrusher::convert::toBitboard(Square::B1, Square::G1));

    EXPECT_EQ(board_state.getBitboard(Piece::WHITE_BISHOP),
              bitcrusher::convert::toBitboard(Square::C1, Square::F1));
    EXPECT_EQ(board_state.getBitboard(Piece::WHITE_ROOK),
              bitcrusher::convert::toBitboard(Square::A1, Square::H1));
    EXPECT_EQ(board_state.getBitboard(Piece::WHITE_QUEEN),
              bitcrusher::convert::toBitboard(Square::D1));
    EXPECT_EQ(board_state.getBitboard(Piece::WHITE_KING),
              bitcrusher::convert::toBitboard(Square::E1));
    // Black pieces
    EXPECT_EQ(board_state.getBitboard(Piece::BLACK_PAWN),
              bitcrusher::convert::toBitboard(Square::A7, Square::B7, Square::C7, Square::D7,
                                              Square::E7, Square::F7, Square::G7, Square::H7));
    EXPECT_EQ(board_state.getBitboard(Piece::BLACK_KNIGHT),
              bitcrusher::convert::toBitboard(Square::B8, Square::G8));
    EXPECT_EQ(board_state.getBitboard(Piece::BLACK_BISHOP),
              bitcrusher::convert::toBitboard(Square::C8, Square::F8));
    EXPECT_EQ(board_state.getBitboard(Piece::BLACK_ROOK),
              bitcrusher::convert::toBitboard(Square::A8, Square::H8));
    EXPECT_EQ(board_state.getBitboard(Piece::BLACK_QUEEN),
              bitcrusher::convert::toBitboard(Square::D8));
    EXPECT_EQ(board_state.getBitboard(Piece::BLACK_KING),
              bitcrusher::convert::toBitboard(Square::E8));
    // State
    EXPECT_TRUE(board_state.isWhiteMove());
    EXPECT_TRUE(board_state.hasCastlingRights<CastlingRights::ALL_CASTLING_RIGHTS>());
    EXPECT_EQ(board_state.getEnPassantSquare(), Square::NULL_SQUARE);
    EXPECT_EQ(board_state.getHalfmoveClock(), 0);
    EXPECT_EQ(board_state.getFullmoveNumber(), 1);
}

TEST(parseFENTest, EmptyBoard) {
    // FEN for an empty board
    std::string_view fen = "8/8/8/8/8/8/8/8 w - - 0 1";
    BoardState       board_state;
    bitcrusher::parseFEN(fen, board_state);

    for (int i = 0; i < std::to_underlying(Piece::COUNT); i++) {
        EXPECT_EQ(board_state.getBitboard(static_cast<Piece>(i)), EMPTY_BITBOARD);
    }

    EXPECT_TRUE(board_state.isWhiteMove());
    EXPECT_FALSE(board_state.hasAnyCastlingRights());
    EXPECT_EQ(board_state.getEnPassantSquare(), Square::NULL_SQUARE);
    EXPECT_EQ(board_state.getHalfmoveClock(), 0);
    EXPECT_EQ(board_state.getFullmoveNumber(), 1);
}

TEST(parseFENTest, SideToMoveBlack) {
    BoardState  board_state;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";

    bitcrusher::parseFEN(fen, board_state);

    EXPECT_FALSE(board_state.isWhiteMove());
}

TEST(parseFENTest, EnPassantSquareSet) {
    BoardState  board_state;
    std::string fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq e3 0 1";

    bitcrusher::parseFEN(fen, board_state);

    EXPECT_EQ(board_state.getEnPassantSquare(), Square::E3);
}

TEST(parseFENTest, HalfmoveAndFullmoveClock) {
    BoardState  board_state;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 5 42";

    bitcrusher::parseFEN(fen, board_state);

    EXPECT_EQ(board_state.getHalfmoveClock(), 5);
    EXPECT_EQ(board_state.getFullmoveNumber(), 42);
}

TEST(parseFENTest, KingsOnlyPosition) {
    BoardState  board_state;
    std::string fen = "4k3/8/8/8/8/8/8/4K3 w - - 0 1";

    bitcrusher::parseFEN(fen, board_state);

    EXPECT_EQ(board_state.getBitboard(Piece::WHITE_KING),
              bitcrusher::convert::toBitboard(Square::E1));
    EXPECT_EQ(board_state.getBitboard(Piece::BLACK_KING),
              bitcrusher::convert::toBitboard(Square::E8));
    EXPECT_TRUE(board_state.getBitboard(Piece::WHITE_PAWN) == EMPTY_BITBOARD);
    EXPECT_TRUE(board_state.getBitboard(Piece::BLACK_PAWN) == EMPTY_BITBOARD);
}

TEST(parseFENTest, MixedPosition) {
    BoardState  board_state;
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";

    bitcrusher::parseFEN(fen, board_state);

    EXPECT_TRUE(board_state.isPieceOnSquare(Piece::WHITE_QUEEN, Square::F3));
    EXPECT_TRUE(board_state.isPieceOnSquare(Piece::WHITE_ROOK, Square::H1));
    EXPECT_TRUE(board_state.isPieceOnSquare(Piece::WHITE_ROOK, Square::A1));
    EXPECT_TRUE(board_state.isPieceOnSquare(Piece::BLACK_KNIGHT, Square::B6));
    EXPECT_TRUE(board_state.isPieceOnSquare(Piece::BLACK_KNIGHT, Square::F6));
    EXPECT_EQ(board_state.getHalfmoveClock(), 0);
    EXPECT_EQ(board_state.getFullmoveNumber(), 1);
}

TEST(MoveFromUci, QuietMove) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::E2);

    Move m = moveFromUci("e2e3", board);

    EXPECT_EQ(m.fromSquare(), Square::E2);
    EXPECT_EQ(m.toSquare(), Square::E3);
    EXPECT_TRUE(m.isQuiet());
}

TEST(MoveFromUci, DoublePawnPush) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::E2);

    Move m = moveFromUci("e2e4", board);

    EXPECT_EQ(m.fromSquare(), Square::E2);
    EXPECT_EQ(m.toSquare(), Square::E4);
    EXPECT_TRUE(m.isPawnDoublePush());
}

TEST(MoveFromUci, CaptureMove) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::E4);
    board.addPieceToSquare<Color::BLACK>(PieceType::PAWN, Square::D5);

    Move m = moveFromUci("e4d5", board);

    EXPECT_EQ(m.fromSquare(), Square::E4);
    EXPECT_EQ(m.toSquare(), Square::D5);
    EXPECT_TRUE(m.isCapture());
    EXPECT_EQ(m.capturedPiece(), PieceType::PAWN);
}

TEST(MoveFromUci, EnPassant) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::E5);
    board.addPieceToSquare<Color::BLACK>(PieceType::PAWN, Square::D5);
    board.setEnPassantSquare(Square::D6);

    Move m = moveFromUci("e5d6", board);

    EXPECT_EQ(m.fromSquare(), Square::E5);
    EXPECT_EQ(m.toSquare(), Square::D6);
    EXPECT_TRUE(m.isEnPassant());
}

TEST(MoveFromUci, PromotionMove) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::E7);

    Move m = moveFromUci("e7e8q", board);

    EXPECT_EQ(m.fromSquare(), Square::E7);
    EXPECT_EQ(m.toSquare(), Square::E8);
    EXPECT_TRUE(m.isPromotion());
    EXPECT_EQ(m.promotionPiece(), PieceType::QUEEN);
}

TEST(MoveFromUci, PromotionCaptureMove) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::E7);
    board.addPieceToSquare<Color::BLACK>(PieceType::KNIGHT, Square::D8);

    Move m = moveFromUci("e7d8n", board);

    EXPECT_EQ(m.fromSquare(), Square::E7);
    EXPECT_EQ(m.toSquare(), Square::D8);
    EXPECT_TRUE(m.isPromotionCapture());
    EXPECT_EQ(m.promotionPiece(), PieceType::KNIGHT);
    EXPECT_EQ(m.capturedPiece(), PieceType::KNIGHT);
}

TEST(MoveFromUci, WhiteKingsideCastling) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::KING, Square::E1);
    board.addPieceToSquare<Color::WHITE>(PieceType::ROOK, Square::H1);
    board.addCastlingRights<CastlingRights::WHITE_CASTLING_RIGHTS>();

    Move m = moveFromUci("e1g1", board);

    EXPECT_TRUE(m.isKingsideCastle());
}

TEST(MoveFromUci, WhiteQueensideCastling) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::KING, Square::E1);
    board.addPieceToSquare<Color::WHITE>(PieceType::ROOK, Square::A1);
    board.addCastlingRights<CastlingRights::WHITE_QUEENSIDE>();

    Move m = moveFromUci("e1c1", board);

    EXPECT_TRUE(m.isQueensideCastle());
}

TEST(MoveFromUci, BlackKingsideCastling) {
    BoardState board;
    board.setSideToMove(Color::BLACK);
    board.addPieceToSquare<Color::BLACK>(PieceType::KING, Square::E8);
    board.addPieceToSquare<Color::BLACK>(PieceType::ROOK, Square::H8);
    board.addCastlingRights<CastlingRights::BLACK_KINGSIDE>();

    Move m = moveFromUci("e8g8", board);

    EXPECT_TRUE(m.isKingsideCastle());
}

TEST(MoveFromUci, BlackQueensideCastling) {
    BoardState board;
    board.setSideToMove(Color::BLACK);
    board.addPieceToSquare<Color::BLACK>(PieceType::KING, Square::E8);
    board.addPieceToSquare<Color::BLACK>(PieceType::ROOK, Square::A8);
    board.addCastlingRights<CastlingRights::BLACK_QUEENSIDE>();

    Move m = moveFromUci("e8c8", board);

    EXPECT_TRUE(m.isQueensideCastle());
}

TEST(MoveFromUci, FallbackQuiet) {
    BoardState board;
    board.setSideToMove(Color::WHITE);
    board.addPieceToSquare<Color::WHITE>(PieceType::KNIGHT, Square::B1);

    Move m = moveFromUci("b1a3", board);

    EXPECT_TRUE(m.isQuiet());
}

TEST(MoveFromUci, BlackCapture) {
    BoardState board;
    board.setSideToMove(Color::BLACK);
    board.addPieceToSquare<Color::BLACK>(PieceType::PAWN, Square::D5);
    board.addPieceToSquare<Color::WHITE>(PieceType::PAWN, Square::C4);

    Move m = moveFromUci("d5c4", board);

    EXPECT_TRUE(m.isCapture());
    EXPECT_EQ(m.capturedPiece(), PieceType::PAWN);
}