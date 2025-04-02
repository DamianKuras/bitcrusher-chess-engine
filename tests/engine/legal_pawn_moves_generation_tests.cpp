#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "fen_formatter.hpp"
#include "legal_moves_generator.hpp"
#include "test_helpers.hpp"
#include <gtest/gtest.h>

using bitcrusher::BoardState;
using bitcrusher::Color;
using bitcrusher::generateLegalPawnMoves;
using bitcrusher::parseFEN;
using test_helpers::TestMoveSink;

TEST(generateLegalPawnMovesTest, statringPositionWhite) {
    BoardState       board;
    std::string_view starting_chess_position =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    parseFEN(starting_chess_position, board);
    TestMoveSink sink = TestMoveSink();

    generateLegalPawnMoves<TestMoveSink, Color::WHITE>(board, sink);

    // clang-format off
    const std::array<std::string, 16> expected_moves{
        "a2a3", "a2a4",
        "b2b3", "b2b4",
        "c2c3", "c2c4",
        "d2d3", "d2d4",
        "e2e3", "e2e4",
        "f2f3", "f2f4",
        "g2g3", "g2g4",
        "h2h3", "h2h4"
    };
    // clang-format on
    for (const auto& uci_move : expected_moves) {
        EXPECT_TRUE(sink.moves.contains(uci_move));
    }

    EXPECT_EQ(sink.moves.size(), expected_moves.size());
}

// Test pawn moves from the starting position for Black
TEST(generateLegalPawnMovesTest, startingPositionBlack) {
    BoardState       board;
    std::string_view starting_chess_position_black_move =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";
    parseFEN(starting_chess_position_black_move, board);
    TestMoveSink sink = TestMoveSink();

    generateLegalPawnMoves<TestMoveSink, Color::BLACK>(board, sink);

    // clang-format off
    const std::array<std::string, 16> expected_moves{
        "a7a6", "a7a5",
        "b7b6", "b7b5",
        "c7c6", "c7c5",
        "d7d6", "d7d5",
        "e7e6", "e7e5",
        "f7f6", "f7f5",
        "g7g6", "g7g5",
        "h7h6", "h7h5"
    };
    // clang-format on
    for (const auto& uci_move : expected_moves) {
        EXPECT_TRUE(sink.moves.contains(uci_move));
    }
    EXPECT_EQ(sink.moves.size(), expected_moves.size());
}

TEST(generateLegalPawnMovesTest, pawnCaptureMoves) {
    BoardState board;
    // FEN: White pawn on d4 can capture enemy pawns on c5 and e5 or push forward.
    std::string_view fen = "k7/8/8/2p1p3/3P4/8/8/K7 w - - 0 1";
    parseFEN(fen, board);
    TestMoveSink sink;

    generateLegalPawnMoves<TestMoveSink, Color::WHITE>(board, sink);

    const std::array<std::string, 3> expected_moves = {"d4d5", "d4c5", "d4e5"};
    for (const auto& move : expected_moves) {
        EXPECT_TRUE(sink.moves.contains(move));
    }
    EXPECT_EQ(sink.moves.size(), expected_moves.size());
}

// Test pawn promotion moves for a White pawn
TEST(generateLegalPawnMovesTest, pawnPromotionMoves) {
    BoardState board;
    // FEN: White pawn on d7 is ready to promote when moving to d8.
    std::string_view fen = "k7/3P4/8/8/8/8/8/K7 w - - 0 1";
    parseFEN(fen, board);
    TestMoveSink sink;

    generateLegalPawnMoves<TestMoveSink, Color::WHITE>(board, sink);

    const std::array<std::string, 4> expected_moves = {"d7d8q", "d7d8r", "d7d8b", "d7d8n"};
    for (const auto& move : expected_moves) {
        EXPECT_TRUE(sink.moves.contains(move));
    }
    EXPECT_EQ(sink.moves.size(), expected_moves.size());
}

TEST(generateLegalPawnMovesTest, blackPawnPromotionMoves) {
    BoardState board;
    // FEN: Black pawn on d2 is ready to promote.
    std::string_view fen = "7k/8/8/8/8/8/3p4/7K b - - 0 1";
    parseFEN(fen, board);
    TestMoveSink sink;

    generateLegalPawnMoves<TestMoveSink, Color::BLACK>(board, sink);

    // Expected promotion moves: using lowercase promotion suffixes.
    const std::array<std::string, 4> expected_moves = {"d2d1q", "d2d1r", "d2d1b", "d2d1n"};
    for (const auto& move : expected_moves) {
        EXPECT_TRUE(sink.moves.contains(move));
    }
    EXPECT_EQ(sink.moves.size(), expected_moves.size());
}

TEST(generateLegalPawnMovesTest, pawnEnPassantMoves) {
    BoardState board;
    // FEN: White pawn on g5 and enemy pawn on f5, with en passant square set to f6.
    std::string_view fen = "k7/8/8/5pP1/8/8/8/K7 w - f6 0 1";
    parseFEN(fen, board);
    TestMoveSink sink;

    generateLegalPawnMoves<TestMoveSink, Color::WHITE>(board, sink);

    const std::array<std::string, 2> expected_moves = {"g5g6", "g5f6"};
    for (const auto& move : expected_moves) {
        EXPECT_TRUE(sink.moves.contains(move));
    }
    EXPECT_EQ(sink.moves.size(), expected_moves.size());
}

TEST(generateLegalPawnMovesTest, pawnBlocked) {
    BoardState board;
    // FEN: Black pawn on a3 blocks the white pawn on a2.
    std::string_view fen = "7k/8/8/8/8/p7/P7/7K w - - 0 1";
    parseFEN(fen, board);
    TestMoveSink sink;

    generateLegalPawnMoves<TestMoveSink, Color::WHITE>(board, sink);

    EXPECT_EQ(sink.moves.size(), 0);
}

TEST(generateLegalPawnMovesTest, pawnPinnedVertically) {
    BoardState board;
    // FEN: White pawn on d2 pinned by black rook on d8.
    std::string_view fen = "3r3k/8/8/8/8/8/3P4/3K4 w - - 0 1";
    parseFEN(fen, board);
    TestMoveSink sink;

    generateLegalPawnMoves<TestMoveSink, Color::WHITE>(board, sink);

    const std::array<std::string, 2> expected_moves = {"d2d3", "d2d4"};
    for (const auto& move : expected_moves) {
        EXPECT_TRUE(sink.moves.contains(move));
    }
    EXPECT_EQ(sink.moves.size(), expected_moves.size());
}

TEST(generateLegalPawnMovesTest, pawnPinnedHorizontally) {
    BoardState board;
    // FEN: White pawn on d2 pinned by black rook on d8.
    std::string_view fen = "7k/8/8/8/8/8/K2P3r/8 w - - 0 1";
    parseFEN(fen, board);
    TestMoveSink sink;

    generateLegalPawnMoves<TestMoveSink, Color::WHITE>(board, sink);

    EXPECT_EQ(sink.moves.size(), 0);
}

TEST(generateLegalPawnMovesTest, pawnEnPassantExposingKingVertically) {
    BoardState board;
    // Fen: White pawn on e5 pinned by rook on e8 with en passant square on d6
    std::string_view fen = "4r2k/8/8/3pP3/8/8/8/4K3 w - d6 0 1";
    parseFEN(fen, board);
    TestMoveSink sink;

    generateLegalPawnMoves<TestMoveSink, Color::WHITE>(board, sink);

    EXPECT_TRUE(sink.moves.contains("e5e6")); // pawn can still move forward

    EXPECT_FALSE(sink.moves.contains("e5d6")); // no invalid en passant

    EXPECT_EQ(sink.moves.size(), 1);
}

TEST(generateLegalPawnMovesTest, pawnEnPassantExposingKingHorizontally) {
    BoardState board;
    // Fen: White pawn on f5 pinned by black rook on h5 because taking en passant would remove both
    // pawns from 5 rank exposing check.
    std::string_view fen = "7k/8/8/K3pP1r/8/8/8/8 w - - 0 1";
    parseFEN(fen, board);
    TestMoveSink sink;

    generateLegalPawnMoves<TestMoveSink, Color::WHITE>(board, sink);

    EXPECT_TRUE(sink.moves.contains("f5f6")); // pawn can still move forward

    EXPECT_FALSE(sink.moves.contains("f5e6")); // no invalid en passant

    EXPECT_EQ(sink.moves.size(), 1);
}