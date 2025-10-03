#include "bitboard_enums.hpp"
#include "legal_move_generators/bishop_legal_moves.hpp"
#include "move_generation_fixture.hpp"
#include "restriction_context.hpp"
#include <gtest/gtest.h>

using bitcrusher::BoardState;
using bitcrusher::Color;
using bitcrusher::generateLegalBishopMoves;
using bitcrusher::RestrictionContext;
using test_helpers::LegalMoveGenerationParametrizedTest;
using test_helpers::LegalMovesTestCase;
using test_helpers::TestMoveSink;

namespace {
void generateLegalBishopMovesWhite(const BoardState&         board,
                                   const RestrictionContext& restriction_context,
                                   TestMoveSink&             sink) {
    generateLegalBishopMoves<Color::WHITE>(board, restriction_context, sink);
}

void generateLegalBishopMovesBlack(const BoardState&         board,
                                   const RestrictionContext& restriction_context,
                                   TestMoveSink&             sink) {
    generateLegalBishopMoves<Color::BLACK>(board, restriction_context, sink);
}
} // namespace

const std::array<LegalMovesTestCase, 6> LEGAL_BISHOP_MOVES_TEST_CASES{{
    {.name           = "Bishops blocked in starting positions should not have legal moves.",
     .fen            = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
     .expected_moves = {}, // No legal moves as the bishop's diagonals are obstructed by own pieces.
     .move_generator = generateLegalBishopMovesWhite},

    {.name           = "Bishop in corner with open diagonal should have 7 legal moves.",
     .fen            = "k7/8/8/8/8/8/8/B3K3 w - - 0 1",
     .expected_moves = {"a1b2", "a1c3", "a1d4", "a1e5", "a1f6", "a1g7", "a1h8"},
     .move_generator = generateLegalBishopMovesWhite},

    {.name           = "Bishop should be able to capture enemy pieces along diagonal.",
     .fen            = "7k/8/8/8/5p2/8/3B4/K7 w - - 0 1",
     .expected_moves = {"d2c3", "d2b4", "d2a5", "d2e3", "d2f4", "d2c1", "d2e1"},
     .move_generator = generateLegalBishopMovesWhite},

    {.name = "Bishop in center with open diagonals should be able to move to every legal square.",
     .fen  = "8/k7/8/3B4/8/8/8/2K5 w - - 0 1",
     .expected_moves = {"d5c6", "d5b7", "d5a8", "d5e6", "d5f7", "d5g8", "d5c4", "d5b3", "d5a2",
                        "d5e4", "d5f3", "d5g2", "d5h1"},
     .move_generator = generateLegalBishopMovesWhite},

    {.name           = "Bishop pinned to king should not be able to move.",
     .fen            = "k7/8/8/8/8/8/8/KB5r w - - 0 1",
     .expected_moves = {}, // Moving the bishop would expose the king to check.
     .move_generator = generateLegalBishopMovesWhite},

    // Test bishop moves for Black in an open board scenario
    {.name = "Black bishop moves with open diagonals should be able to move to every legal square.",
     .fen  = "8/2k5/8/8/8/4b3/8/K7 b - - 0 1",
     .expected_moves = {"e3d4", "e3c5", "e3b6", "e3a7", "e3f4", "e3g5", "e3h6", "e3d2", "e3c1",
                        "e3f2", "e3g1"},
     .move_generator = generateLegalBishopMovesBlack},
}};

INSTANTIATE_TEST_SUITE_P(LegalBishopMovesGenerationTests,
                         LegalMoveGenerationParametrizedTest,
                         ::testing::ValuesIn(LEGAL_BISHOP_MOVES_TEST_CASES));
