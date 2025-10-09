#include "perft_fixture.hpp"
#include "bitboard_enums.hpp"
#include "fen_formatter.hpp"
#include "move_processor.hpp"
#include "gtest/gtest.h"
#include <perft.hpp>

using bitcrusher::Color;
using bitcrusher::MoveProcessor;
using bitcrusher::RestrictionContext;
using test_helpers::TestPerftMoveSink;

namespace {


} // namespace

namespace test_helpers {

static std::ostream& operator<<(std::ostream& os, const PerftTestCase& test_case) {
    return os << test_case.name << "\nFEN: " << test_case.fen << "\n";
}

void PerftParametrizedTest::runTest() {
    const auto& test_case = GetParam();
    parseFEN(test_case.fen, board_);
    TestPerftMoveSink  local_sink{};
    MoveProcessor      move_processor{};
    RestrictionContext restriction_context{};
    uint64_t           leaf_node_count = 0;
    local_sink.setDepth(test_case.depth);
    if (board_.isWhiteMove()) {
        leaf_node_count = perft<Color::WHITE>(test_case.depth, board_, move_processor, local_sink,
                                              restriction_context);
    } else {
        leaf_node_count = perft<Color::BLACK>(test_case.depth, board_, move_processor, local_sink,
                                              restriction_context);
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
