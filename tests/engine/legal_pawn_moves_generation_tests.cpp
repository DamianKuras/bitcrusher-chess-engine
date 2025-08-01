#include "bitboard_enums.hpp"
#include "legal_move_generators/pawn_legal_moves.hpp"
#include "move_generation_fixture.hpp"
#include <gtest/gtest.h>

using bitcrusher::BoardState;
using bitcrusher::Color;
using bitcrusher::generateLegalPawnMoves;
using bitcrusher::RestrictionContext;
using test_helpers::LegalMoveGenerationParametrizedTest;
using test_helpers::LegalMovesTestCase;
using test_helpers::TestMoveSink;

namespace {
void generateLegalPawnMovesWhite(const BoardState&         board,
                                 const RestrictionContext& restriction_context,
                                 TestMoveSink&             sink) {
    generateLegalPawnMoves<TestMoveSink, Color::WHITE>(board, restriction_context, sink);
}

void generateLegalPawnMovesBlack(const BoardState&         board,
                                 const RestrictionContext& restriction_context,
                                 TestMoveSink&             sink) {
    generateLegalPawnMoves<TestMoveSink, Color::BLACK>(board, restriction_context, sink);
}
} // namespace

const std::array<LegalMovesTestCase, 12> LEGAL_PAWN_MOVES_TEST_CASES{{
    {.name           = "starting chess position white pawn moves",
     .fen            = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
     .expected_moves = {"a2a3", "a2a4", "b2b3", "b2b4", "c2c3", "c2c4", "d2d3", "d2d4", "e2e3",
                        "e2e4", "f2f3", "f2f4", "g2g3", "g2g4", "h2h3", "h2h4"},
     .move_generator = generateLegalPawnMovesWhite},

    {.name           = "starting chess position black pawn moves",
     .fen            = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
     .expected_moves = {"a7a6", "a7a5", "b7b6", "b7b5", "c7c6", "c7c5", "d7d6", "d7d5", "e7e6",
                        "e7e5", "f7f6", "f7f5", "g7g6", "g7g5", "h7h6", "h7h5"},
     .move_generator = generateLegalPawnMovesBlack},

    {.name           = "pawns should be able to capture enemy pieces",
     .fen            = "k7/8/8/2p1p3/3P4/8/8/K7 w - - 0 1",
     .expected_moves = {"d4d5", "d4c5", "d4e5"},
     .move_generator = generateLegalPawnMovesWhite},

    {.name           = "pawns should be able to promote to knight or bishop or rook or queen.",
     .fen            = "k7/3P4/8/8/8/8/8/K7 w - - 0 1",
     .expected_moves = {"d7d8q", "d7d8r", "d7d8b", "d7d8n"},
     .move_generator = generateLegalPawnMovesWhite},

    {.name = "black pawns should be able to promote to knight or bishop or rook or queen.",
     .fen  = "7k/8/8/8/8/8/3p4/7K b - - 0 1",
     .expected_moves = {"d2d1q", "d2d1r", "d2d1b", "d2d1n"},
     .move_generator = generateLegalPawnMovesBlack},

    {.name           = "pawns should be able to take en-passant.",
     .fen            = "k7/8/8/5pP1/8/8/8/K7 w - f6 0 1",
     .expected_moves = {"g5g6", "g5f6"},
     .move_generator = generateLegalPawnMovesWhite},

    //
    {.name           = "pawns should not be able to move over enemy piece.",
     .fen            = "7k/8/8/8/8/p7/P7/7K w - - 0 1",
     .expected_moves = {}, //  No legal Pawn moves.
     .move_generator = generateLegalPawnMovesWhite},

    {.name = "pawns pinned only vertically should be able to move vertically if squares are empty.",
     .fen  = "3r3k/8/8/8/8/8/3P4/3K4 w - - 0 1",
     .expected_moves = {"d2d3", "d2d4"},
     .move_generator = generateLegalPawnMovesWhite},

    {.name           = "pawns pinned horizontally should not be able to move",
     .fen            = "7k/8/8/8/8/8/K2P3r/8 w - - 0 1",
     .expected_moves = {},
     .move_generator = generateLegalPawnMovesWhite},

    {.name = "pawns should not be able to take en passant exposing king to check vertically.",
     .fen  = "4r2k/8/8/3pP3/8/8/8/4K3 w - d6 0 1",
     .expected_moves = {"e5e6"},
     .move_generator = generateLegalPawnMovesWhite},

    {.name = "pawns should not be able to take en passant exposing king to check horizontally.",
     .fen  = "7k/8/8/K3pP1r/8/8/8/8 w - e6 0 1",
     .expected_moves = {"f5f6"},
     .move_generator = generateLegalPawnMovesWhite},

    {.name = "pawns should not be able to take en passant exposing king to check diagonally.",
     .fen  = "7k/6b1/8/3pP3/8/2K5/8/8 w - d6 0 1",
     .expected_moves = {},
     .move_generator = generateLegalPawnMovesWhite},

}};

INSTANTIATE_TEST_SUITE_P(LegalPawnMovesGenerationTests,
                         LegalMoveGenerationParametrizedTest,
                         ::testing::ValuesIn(LEGAL_PAWN_MOVES_TEST_CASES));