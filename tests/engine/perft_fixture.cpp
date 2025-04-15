#include "perft_fixture.hpp"
#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "fen_formatter.hpp"
#include "legal_move_generators/legal_moves_generator.hpp"
#include "move.hpp"
#include "move_maker.hpp"
#include "gtest/gtest.h"

using bitcrusher::BoardState;
using bitcrusher::Color;
using bitcrusher::generateLegalMoves;
using bitcrusher::makeMove;
using bitcrusher::Move;
using bitcrusher::unmakeMove;
using test_helpers::TestPerftMoveSink;

namespace {

template <Color SideToMove>
[[nodiscard]] uint64_t perft(int depth, BoardState& board, TestPerftMoveSink& sink) {
    uint64_t leave_node_count = 0;
    board.recalculateOccupancy();
    updateRestrictionContext<SideToMove>(board, board.restriction_context);
    sink.depth = depth;
    generateLegalMoves<TestPerftMoveSink, SideToMove>(board, sink);

    std::vector<Move> moves = std::move(sink.moves);
    sink.clearMoves();

    if (depth == test_helpers::LEAF_DEPTH) {
        return static_cast<uint64_t>(moves.size());
    }

    for (const auto& move : moves) {
        board.recalculateOccupancy();
        updateRestrictionContext<SideToMove>(board, board.restriction_context);
        makeMove(board, move);
        leave_node_count += perft<! SideToMove>(depth - 1, board, sink);
        unmakeMove(board, move);
    }

    return leave_node_count;
}

} // namespace

namespace test_helpers {

static std::ostream& operator<<(std::ostream& os, const PerftTestCase& test_case) {
    return os << test_case.name << "\nFEN: " << test_case.fen << "\n";
}

void PerftParametrizedTest::runTest() {
    const auto& test_case = GetParam();
    parseFEN(test_case.fen, board_);
    TestPerftMoveSink local_sink;
    uint64_t          leaf_node_count = 0;
    if (board_.isWhiteMove()) {
        leaf_node_count = perft<Color::WHITE>(test_case.depth, board_, local_sink);
    } else {
        leaf_node_count = perft<Color::BLACK>(test_case.depth, board_, local_sink);
    }

    EXPECT_EQ(leaf_node_count, test_case.leaf_node_count)
        << test_case << "Leaf node count mismatch expected: " << test_case.leaf_node_count
        << ", actual: " << leaf_node_count;

    EXPECT_EQ(local_sink.captures_count, test_case.captures_count)
        << test_case << "Capture count mismatch expected: " << test_case.captures_count
        << ", actual: " << local_sink.captures_count;

    EXPECT_EQ(local_sink.en_passant_count, test_case.en_passant_count)
        << test_case << "En passant count mismatch expected: " << test_case.en_passant_count
        << " actual: " << local_sink.en_passant_count;

    EXPECT_EQ(local_sink.promotions_count, test_case.promotions_count)
        << test_case << "Promotions count mismatch expected: " << test_case.promotions_count
        << ", actual: " << local_sink.promotions_count;
}

TEST_P(PerftParametrizedTest, VerifyPerftResults) {
    runTest();
}

} // namespace test_helpers
