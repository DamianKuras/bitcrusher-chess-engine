#ifndef BITCRUSHER_MOVE_GENERATION_FIXTURE_HPP
#define BITCRUSHER_MOVE_GENERATION_FIXTURE_HPP

#include "board_state.hpp"
#include "fen_formatter.hpp"
#include "test_helpers.hpp"
#include <gtest/gtest.h>
#include <string>

namespace test_helpers {
using bitcrusher::BoardState;
using ::test_helpers::TestMoveSink;

// Define a function type for move generation.
using MoveGeneratorFunc = void (*)(const BoardState&, TestMoveSink&);

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