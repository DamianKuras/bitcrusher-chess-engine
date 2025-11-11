#ifndef BITCRUSHER_MOVE_GENERATION_FIXTURE_HPP
#define BITCRUSHER_MOVE_GENERATION_FIXTURE_HPP

#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "concepts.hpp"
#include "legal_move_generators/pext_bitboards.hpp"
#include "move.hpp"
#include "restriction_context.hpp"
#include <gtest/gtest.h>
#include <string>
#include <unordered_set>

namespace test_helpers {

using bitcrusher::Color;
using bitcrusher::Move;
using bitcrusher::MoveSink;
using bitcrusher::MoveSinkBase;
using bitcrusher::MoveType;
using bitcrusher::PieceType;
using bitcrusher::Square;
using bitcrusher::toUci;

struct TestMoveSink : MoveSinkBase<TestMoveSink> {
    std::unordered_set<std::string> moves;

    void clear() { moves.clear(); }

    template <MoveType  MoveT,
              PieceType MovedOrPromotedToPiece,
              Color     SideToMove,
              PieceType CapturedPiece = PieceType::NONE>
    void emplace(Square from, Square to) noexcept {
        Move m;
        m.setMove<MoveT, MovedOrPromotedToPiece, SideToMove, CapturedPiece>(from, to);
        moves.insert(toUci(m));
    }
};

static_assert(MoveSink<TestMoveSink>);

using bitcrusher::BoardState;
using bitcrusher::RestrictionContext;
using ::test_helpers::TestMoveSink;

// Define a function type for move generation.
using MoveGeneratorFunc = void (*)(const BoardState&, const RestrictionContext&, TestMoveSink&);

// Structure to hold test parameters.
struct LegalMovesTestCase {
    std::string              name;
    std::string              fen;
    std::vector<std::string> expected_moves;
    // The move generator function; this could wrap any move generation function.
    MoveGeneratorFunc move_generator;
};

class LegalMoveGenerationParametrizedTest : public ::testing::TestWithParam<LegalMovesTestCase> {

private:
    BoardState   board_;
    TestMoveSink sink_;

protected:
    void runTest();
};

} // namespace test_helpers

#endif // BITCRUSHER_MOVE_GENERATION_FIXTURE_HPP