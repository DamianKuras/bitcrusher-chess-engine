#include "bitboard_enums.hpp"
#include "legal_moves_generator.hpp"
#include "move_generation_fixture.hpp"
#include "test_helpers.hpp"

#include <gtest/gtest.h>

using bitcrusher::BoardState;
using bitcrusher::Color;
using test_helpers::LegalMoveGenerationParametrizedTest;
using test_helpers::LegalMovesTestCase;
using test_helpers::TestMoveSink;

namespace {
void generateLegalRookMovesWhite(const BoardState& board, TestMoveSink& sink) {
    generateLegalRookMoves<TestMoveSink, Color::WHITE>(board, sink);
}

void generateLegalRookMovesBlack(const BoardState& board, TestMoveSink& sink) {
    generateLegalRookMoves<TestMoveSink, Color::BLACK>(board, sink);
}
} // namespace

const std::array<LegalMovesTestCase, 7> LEGAL_ROOK_MOVES_TEST_CASES{{
    {.name           = "starting chess position white rook moves blocked by own piece",
     .fen            = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
     .expected_moves = {},
     .move_generator = generateLegalRookMovesWhite},

    {.name = "rook in corner with no blocking piece along file and rank should have 14 legal moves",
     .fen  = "7k/8/8/8/8/8/1K6/R7 w - - 0 1",
     .expected_moves = {"a1b1", "a1c1", "a1d1", "a1e1", "a1f1", "a1g1", "a1h1", "a1a2", "a1a3",
                        "a1a4", "a1a5", "a1a6", "a1a7", "a1a8"},
     .move_generator = generateLegalRookMovesWhite},

    {.name           = "rook should be able to capture enemy pieces",
     .fen            = "k7/8/3p4/8/3R1n2/8/8/K7 w - - 0 1",
     .expected_moves = {"d4d5", "d4d6", "d4d3", "d4d2", "d4d1", "d4c4", "d4b4", "d4a4", "d4e4",
                        "d4f4"},
     .move_generator = generateLegalRookMovesWhite},

    {.name           = "starting chess position black rook moves",
     .fen            = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
     .expected_moves = {},
     .move_generator = generateLegalRookMovesBlack},

    {.name           = "black rook capturing enemy pieces",
     .fen            = "k7/3P4/8/3r1N2/8/8/8/K7 b - - 0 1",
     .expected_moves = {"d5d6", "d5d7", "d5d4", "d5d3", "d5d2", "d5d1", "d5c5", "d5b5", "d5a5",
                        "d5e5", "d5f5"},
     .move_generator = generateLegalRookMovesBlack},

    {.name = "white rook pinned along file should be able along file and capture pinning piece",
     .fen  = "3r4/8/8/8/8/8/3R4/3K4 w - - 0 1",
     .expected_moves = {"d2d3", "d2d4", "d2d5", "d2d6", "d2d7", "d2d8"},
     .move_generator = generateLegalRookMovesWhite},

    {.name           = "black rook pinned along diagonal should not be able to move",
     .fen            = "6KB/8/8/8/8/8/1r6/k7 w - - 0 1",
     .expected_moves = {},
     .move_generator = generateLegalRookMovesWhite},
}};

INSTANTIATE_TEST_SUITE_P(LegalRookMovesGenerationTests,
                         LegalMoveGenerationParametrizedTest,
                         ::testing::ValuesIn(LEGAL_ROOK_MOVES_TEST_CASES));