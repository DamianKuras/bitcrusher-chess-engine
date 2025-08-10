#include "legal_move_generators/king_legal_moves.hpp"
#include "move_generation_fixture.hpp"
#include "restriction_context.hpp"
#include <gtest/gtest.h>

using bitcrusher::BoardState;
using bitcrusher::Color;
using bitcrusher::RestrictionContext;
using test_helpers::LegalMoveGenerationParametrizedTest;
using test_helpers::LegalMovesTestCase;
using test_helpers::TestMoveSink;

namespace {
void generateLegalKingMovesWhite(const BoardState&         board,
                                 const RestrictionContext& restriction_context,
                                 TestMoveSink&             sink) {
    bitcrusher::generateLegalKingMoves<TestMoveSink, Color::WHITE>(board, restriction_context,
                                                                   sink);
}

void generateLegalKingMovesBlack(const BoardState&         board,
                                 const RestrictionContext& restriction_context,
                                 TestMoveSink&             sink) {
    bitcrusher::generateLegalKingMoves<TestMoveSink, Color::BLACK>(board, restriction_context,
                                                                   sink);
}
} // namespace

const std::array<LegalMovesTestCase, 9> LEGAL_KING_MOVES_TEST_CASES{{
    {.name = "king in the center of the board unblocked and not attacked should have 8 legal moves",
     .fen  = "k7/8/8/3K4/8/8/8/8 w - - 0 1",
     .expected_moves = {"d5c4", "d5d4", "d5e4", "d5c5", "d5e5", "d5c6", "d5d6", "d5e6"},
     .move_generator = generateLegalKingMovesWhite},

    {.name           = "black king should be able to move and castle",
     .fen            = "rnbqk2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
     .expected_moves = {"e8f8", "e8g8"},
     .move_generator = generateLegalKingMovesBlack},

    {.name           = "king should be able to capture enemy piece",
     .fen            = "k7/8/8/2pKp3/8/8/8/8 w - - 0 1",
     .expected_moves = {"d5c4", "d5e4", "d5c5", "d5e5", "d5c6", "d5d6", "d5e6"},
     .move_generator = generateLegalKingMovesWhite},

    {.name           = "king should not be able to move to square occupied by friendly piece",
     .fen            = "k7/8/2PPP3/2PKP3/2PPP3/8/8/8 w - - 0 1",
     .expected_moves = {},
     .move_generator = generateLegalKingMovesWhite},

    {.name = "king in position where kingside castle is allowed should be able to castle kingside",
     .fen  = "k7/8/8/8/8/8/8/4K2R w K - 0 1",
     .expected_moves = {"e1d1", "e1d2", "e1e2", "e1f2", "e1f1", /*white kingside castle*/ "e1g1"},
     .move_generator = generateLegalKingMovesWhite},

    {.name =
         "king in position where queenside castle is allowed should be able to castle queenside",
     .fen            = "7k/8/8/8/8/8/8/R3K3 w Q - 0 1",
     .expected_moves = {"e1d1", "e1d2", "e1e2", "e1f2", "e1f1", /* white queenside castle*/ "e1c1"},
     .move_generator = generateLegalKingMovesWhite},

    {.name           = "king should not be able to castle under check",
     .fen            = "k3r3/8/8/8/8/8/8/4K2R w K - 0 1",
     .expected_moves = {"e1d1", "e1d2", "e1f1", "e1f2"}, // only evasions moves are legal
     .move_generator = generateLegalKingMovesWhite},

    {.name = "king should not be able to castle when square between king and rook is occupied",
     .fen  = "k7/8/8/8/8/8/8/4KN1R w K - 0 1",
     .expected_moves = {"e1d1", "e1d2", "e1e2", "e1f2"},
     .move_generator = generateLegalKingMovesWhite},

    {.name           = "king should be able to capture checking piece",
     .fen            = "k7/8/8/8/8/8/4r3/4K3 w a - 0 1",
     .expected_moves = {"e1d1", "e1f1", /* capturing rook that is giving a check */ "e1e2"},
     .move_generator = generateLegalKingMovesWhite},

}};

INSTANTIATE_TEST_SUITE_P(KnightMovesTests,
                         LegalMoveGenerationParametrizedTest,
                         ::testing::ValuesIn(LEGAL_KING_MOVES_TEST_CASES));