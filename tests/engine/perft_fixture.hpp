#ifndef BITCRUSHER_PERFT_FIXTURE_HPP
#define BITCRUSHER_PERFT_FIXTURE_HPP

#include "bitboard_concepts.hpp"
#include "board_state.hpp"
#include "move.hpp"
#include "test_helpers.hpp"
#include <gtest/gtest.h>
#include <string>

namespace test_helpers {

using bitcrusher::Move;
using bitcrusher::MoveSink;

// Sink to collect and count moves during perft testing.
struct TestPerftMoveSink {
    uint64_t      captures_count{0};
    uint64_t      en_passant_count{0};
    uint64_t      promotions_count{0};
    std::uint64_t castling_count{0};
    // Current depth. In leaf nodes this is used to count specific move types.
    uint64_t depth{0};

    void clearMoves() noexcept { moves.clear(); }

    std::vector<Move> moves;

    void operator()(const bitcrusher::Move& move) noexcept {
        moves.push_back(move);
        const int leaf_depth = 1;
        if (depth != leaf_depth) {
            return;
        }
        if (move.isEnPassant()) {
            en_passant_count++;
        }
        if (move.isCapture() || move.isPromotionCapture() || move.isEnPassant()) {
            captures_count++;
        }
        if (move.isPromotion()) {
            promotions_count++;
        }
        if (move.isKingsideCastle() || move.isQueensideCastle()) {
            castling_count++;
        }
    }
};

static_assert(MoveSink<TestPerftMoveSink>);

using bitcrusher::BoardState;
using ::test_helpers::TestPerftMoveSink;

// Structure to hold test parameters.
struct PerftTestCase {
    std::string   name;
    std::string   fen;
    int           depth{1};
    std::uint64_t leaf_node_count{0};
    std::uint64_t captures_count{0};
    std::uint64_t en_passant_count{0};
    std::uint64_t promotions_count{0};
    std::uint64_t castling_count{0};
};

class PerftParametrizedTest : public ::testing::TestWithParam<PerftTestCase> {
private:
    BoardState        board_;
    TestPerftMoveSink sink_;

protected:
    void runTest();
};

} // namespace test_helpers

#endif // BITCRUSHER_PERFT_FIXTURE_HPP