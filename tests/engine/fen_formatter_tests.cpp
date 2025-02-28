#include "bitboard.h"
#include "fen_formatter.h"
#include <board_state.h>
#include <gtest/gtest.h>

TEST(FENParserTest, StartingPosition) {
    // Inital chess position FEN
    std::string_view       fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    bitcrusher::BoardState board_state;
    bitcrusher::parseFEN(fen, board_state);

    // White pieces
    EXPECT_EQ(board_state[bitcrusher::Piece::WHITE_PAWNS],
              bitboardFromSquares(bitcrusher::Square::A2, bitcrusher::Square::B2, bitcrusher::Square::C2,
                                  bitcrusher::Square::D2, bitcrusher::Square::E2, bitcrusher::Square::F2,
                                  bitcrusher::Square::G2, bitcrusher::Square::H2));

    EXPECT_EQ(board_state[bitcrusher::Piece::WHITE_KNIGHTS],
              bitcrusher::bitboardFromSquares(bitcrusher::Square::B1, bitcrusher::Square::G1));

    EXPECT_EQ(board_state[bitcrusher::Piece::WHITE_BISHOPS],
              bitcrusher::bitboardFromSquares(bitcrusher::Square::C1, bitcrusher::Square::F1));
    EXPECT_EQ(board_state[bitcrusher::Piece::WHITE_ROOKS],
              bitcrusher::bitboardFromSquares(bitcrusher::Square::A1, bitcrusher::Square::H1));
    EXPECT_EQ(board_state[bitcrusher::Piece::WHITE_QUEENS], bitcrusher::bitboardFromSquare(bitcrusher::Square::D1));
    EXPECT_EQ(board_state[bitcrusher::Piece::WHITE_KING], bitcrusher::bitboardFromSquare(bitcrusher::Square::E1));

    // Black pieces
    EXPECT_EQ(board_state[bitcrusher::Piece::BLACK_PAWNS],
              bitboardFromSquares(bitcrusher::Square::A7, bitcrusher::Square::B7, bitcrusher::Square::C7,
                                  bitcrusher::Square::D7, bitcrusher::Square::E7, bitcrusher::Square::F7,
                                  bitcrusher::Square::G7, bitcrusher::Square::H7));
    EXPECT_EQ(board_state[bitcrusher::Piece::BLACK_KNIGHTS],
              bitcrusher::bitboardFromSquares(bitcrusher::Square::B8, bitcrusher::Square::G8));
    EXPECT_EQ(board_state[bitcrusher::Piece::BLACK_BISHOPS],
              bitcrusher::bitboardFromSquares(bitcrusher::Square::C8, bitcrusher::Square::F8));
    EXPECT_EQ(board_state[bitcrusher::Piece::BLACK_ROOKS],
              bitcrusher::bitboardFromSquares(bitcrusher::Square::A8, bitcrusher::Square::H8));
    EXPECT_EQ(board_state[bitcrusher::Piece::BLACK_QUEENS], bitcrusher::bitboardFromSquare(bitcrusher::Square::D8));
    EXPECT_EQ(board_state[bitcrusher::Piece::BLACK_KING], bitcrusher::bitboardFromSquare(bitcrusher::Square::E8));

    const bitcrusher::CastlingRight expected_castling_rights =
        bitcrusher::CastlingRight::WHITE_KINGSIDE | bitcrusher::CastlingRight::WHITE_QUEENSIDE |
        bitcrusher::CastlingRight::BLACK_KINGSIDE | bitcrusher::CastlingRight::BLACK_QUEENSIDE;
    // State
    EXPECT_TRUE(board_state.is_white_move);
    EXPECT_EQ(board_state.castling_rights, expected_castling_rights);
    EXPECT_EQ(board_state.en_passant_square, bitcrusher::Square::NULL_SQUARE);
    EXPECT_EQ(board_state.halfmove_clock, 0);
    EXPECT_EQ(board_state.fullmove_number, 1);
}

// Test case for an empty board FEN
TEST(FENParserTest, EmptyBoard) {
    // FEN for an empty board
    std::string_view       fen = "8/8/8/8/8/8/8/8 w - - 0 1";
    bitcrusher::BoardState board_state;
    bitcrusher::parseFEN(fen, board_state);

    for (auto bitboard : board_state.bitboards) {
        EXPECT_EQ(bitboard, 0ULL);
    }

    EXPECT_TRUE(board_state.is_white_move);
    EXPECT_EQ(board_state.castling_rights, bitcrusher::CastlingRight::NONE);
    EXPECT_EQ(board_state.en_passant_square, bitcrusher::Square::NULL_SQUARE);
    EXPECT_EQ(board_state.halfmove_clock, 0);
    EXPECT_EQ(board_state.fullmove_number, 1);
}

TEST(FENParserTest, SideToMoveBlack) {
    bitcrusher::BoardState board_state;
    std::string            fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";
    bitcrusher::parseFEN(fen, board_state);
    EXPECT_FALSE(board_state.is_white_move);
}

TEST(FENParserTest, CastlingRightsNone) {
    bitcrusher::BoardState board_state;
    std::string            fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1";
    bitcrusher::parseFEN(fen, board_state);
    EXPECT_EQ(board_state.castling_rights, bitcrusher::CastlingRight::NONE);
}

TEST(FENParserTest, WhiteCastlingRights) {
    bitcrusher::BoardState board_state;
    std::string            fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1";
    bitcrusher::parseFEN(fen, board_state);
    EXPECT_EQ(board_state.castling_rights,
              bitcrusher::CastlingRight::WHITE_KINGSIDE | bitcrusher::CastlingRight::WHITE_QUEENSIDE);
}

TEST(FENParserTest, EnPassantSquareSet) {
    bitcrusher::BoardState board_state;
    std::string            fen = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq e3 0 1";
    bitcrusher::parseFEN(fen, board_state);
    EXPECT_EQ(board_state.en_passant_square, bitcrusher::Square::E3);
}

TEST(FenParserTest, HalfmoveAndFullmove) {
    bitcrusher::BoardState board_state;
    std::string            fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 5 42";
    bitcrusher::parseFEN(fen, board_state);
    EXPECT_EQ(board_state.halfmove_clock, 5);
    EXPECT_EQ(board_state.fullmove_number, 42);
}

TEST(FenParserTest, KingsOnlyPosition) {
    bitcrusher::BoardState board_state;
    std::string            fen = "4k3/8/8/8/8/8/8/4K3 w - - 0 1";
    bitcrusher::parseFEN(fen, board_state);

    EXPECT_EQ(board_state[bitcrusher::Piece::WHITE_KING], bitcrusher::bitboardFromSquare(bitcrusher::Square::E1));
    EXPECT_EQ(board_state[bitcrusher::Piece::BLACK_KING], bitcrusher::bitboardFromSquare(bitcrusher::Square::E8));
    EXPECT_TRUE(board_state[bitcrusher::Piece::WHITE_PAWNS] == 0);
    EXPECT_TRUE(board_state[bitcrusher::Piece::BLACK_PAWNS] == 0);
}

TEST(FenParserTest, MixedPosition) {
    bitcrusher::BoardState board_state;
    std::string            fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    bitcrusher::parseFEN(fen, board_state);

    EXPECT_TRUE(board_state[bitcrusher::Piece::WHITE_QUEENS] & bitcrusher::bitboardFromSquare(bitcrusher::Square::F3));
    EXPECT_TRUE(board_state[bitcrusher::Piece::WHITE_ROOKS] & bitcrusher::bitboardFromSquare(bitcrusher::Square::H1));
    EXPECT_TRUE(board_state[bitcrusher::Piece::WHITE_ROOKS] & bitcrusher::bitboardFromSquare(bitcrusher::Square::A1));
    EXPECT_TRUE(board_state[bitcrusher::Piece::BLACK_KNIGHTS] & bitcrusher::bitboardFromSquare(bitcrusher::Square::B6));
    EXPECT_TRUE(board_state[bitcrusher::Piece::BLACK_KNIGHTS] & bitcrusher::bitboardFromSquare(bitcrusher::Square::F6));
    EXPECT_EQ(board_state.halfmove_clock, 0);
    EXPECT_EQ(board_state.fullmove_number, 1);
}