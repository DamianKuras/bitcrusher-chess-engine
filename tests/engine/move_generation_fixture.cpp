#include "move_generation_fixture.hpp"
#include "bitboard_enums.hpp"
#include "fen_formatter.hpp"
#include <algorithm>
#include <string>

using bitcrusher::Color;

namespace test_helpers {

static std::ostream& operator<<(std::ostream& os, const LegalMovesTestCase& test_case) {
    return os << test_case.name << "\nFEN: " << test_case.fen << "\n";
}

void LegalMoveGenerationParametrizedTest::runTest() {
    const auto& test_case = GetParam();
    parseFEN(test_case.fen, board_);
    RestrictionContext restriction_context;
    if (board_.isWhiteMove()) {
        updateRestrictionContext<Color::WHITE>(board_, restriction_context);
    } else {
        updateRestrictionContext<Color::BLACK>(board_, restriction_context);
    }
    sink_.clear(); // Reset sink

    test_case.move_generator(board_, restriction_context, sink_);

    for (const std::string& expected_move : test_case.expected_moves) {
        EXPECT_TRUE(sink_.moves.contains(expected_move))
            << test_case << "Expected move: " << expected_move << " not generated";
    }

    // Prepare a string of extra moves if any.
    std::string extra_moves;
    for (const std::string& move : sink_.moves) {
        if (std::ranges::find(test_case.expected_moves, move) == test_case.expected_moves.end()) {
            extra_moves += move + " ";
        }
    }

    EXPECT_EQ(sink_.moves.size(), test_case.expected_moves.size())
        << test_case << "Generated moves count mismatch. Extra moves: " << extra_moves;
}

TEST_P(LegalMoveGenerationParametrizedTest, VerifyMoves) {
    runTest();
}

} // namespace test_helpers