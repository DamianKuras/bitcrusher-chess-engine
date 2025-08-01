#include "legal_move_generators/knight_legal_moves.hpp"
#include "move_generation_fixture.hpp"
#include <gtest/gtest.h>

using bitcrusher::BoardState;
using bitcrusher::Color;
using bitcrusher::generateLegalKnightMoves;
using bitcrusher::RestrictionContext;
using test_helpers::LegalMoveGenerationParametrizedTest;
using test_helpers::LegalMovesTestCase;
using test_helpers::TestMoveSink;

namespace {
void generateLegalKnightMovesWhite(const BoardState&         board,
                                   const RestrictionContext& restriction_context,
                                   TestMoveSink&             sink) {
    generateLegalKnightMoves<TestMoveSink, Color::WHITE>(board, restriction_context, sink);
}

void generateLegalKnightMovesBlack(const BoardState&         board,
                                   const RestrictionContext& restriction_context,
                                   TestMoveSink&             sink) {
    generateLegalKnightMoves<TestMoveSink, Color::BLACK>(board, restriction_context, sink);
}
} // namespace

const std::array<LegalMovesTestCase, 8> LEGAL_KNIGHT_MOVES_TEST_CASES{{
    {.name           = "starting chess position white knight moves",
     .fen            = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
     .expected_moves = {"b1a3", "b1c3", "g1f3", "g1h3"},
     .move_generator = generateLegalKnightMovesWhite},

    {.name           = "starting chess position black knight moves",
     .fen            = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
     .expected_moves = {"b8a6", "b8c6", "g8f6", "g8h6"},
     .move_generator = generateLegalKnightMovesBlack},

    {.name           = " knight in center should have 8 legal moves",
     .fen            = "k7/8/8/8/3N4/8/8/K7 w - - 0 1",
     .expected_moves = {"d4b5", "d4b3", "d4c6", "d4c2", "d4e6", "d4e2", "d4f5", "d4f3"},
     .move_generator = generateLegalKnightMovesWhite},

    {.name           = " knight blocked by friendly pieces should not be able to jump on them",
     .fen            = "k7/8/2P1P3/8/3N4/8/8/K7 w - - 0 1",
     .expected_moves = {"d4b5", "d4b3", "d4c2", "d4e2", "d4f5", "d4f3"},
     .move_generator = generateLegalKnightMovesWhite},

    {.name           = "knight in corner should have 2 legal moves",
     .fen            = "7k/8/8/8/8/8/8/N6K w - - 0 1",
     .expected_moves = {"a1b3", "a1c2"},
     .move_generator = generateLegalKnightMovesWhite},

    {.name           = "knight should be able to capture enemy pieces",
     .fen            = "k7/8/8/5p2/3N4/1p6/8/K7 w - - 0 1",
     .expected_moves = {"d4b5", "d4b3", "d4c6", "d4c2", "d4e6", "d4e2", "d4f5", "d4f3"},
     .move_generator = generateLegalKnightMovesWhite},

    {.name           = "knight pinned vertically should not be able to move",
     .fen            = "3r3k/8/8/8/8/8/3N4/3K4 w - - 0 1",
     .expected_moves = {},
     .move_generator = generateLegalKnightMovesWhite},

    {.name           = "knight pinned diagonally should not be able to move",
     .fen            = "k7/8/7b/8/8/8/3N4/2K5 w - - 0 1",
     .expected_moves = {},
     .move_generator = generateLegalKnightMovesWhite},

}};

INSTANTIATE_TEST_SUITE_P(KnightMovesTests,
                         LegalMoveGenerationParametrizedTest,
                         ::testing::ValuesIn(LEGAL_KNIGHT_MOVES_TEST_CASES));