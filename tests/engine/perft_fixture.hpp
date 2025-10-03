#ifndef BITCRUSHER_PERFT_FIXTURE_HPP
#define BITCRUSHER_PERFT_FIXTURE_HPP

#include "board_state.hpp"
#include "concepts.hpp"
#include "move.hpp"
#include <gtest/gtest.h>
#include <string>

namespace test_helpers {

using bitcrusher::Color;
using bitcrusher::Move;
using bitcrusher::MoveSink;
using bitcrusher::MoveSinkBase;
using bitcrusher::MoveType;
using bitcrusher::PieceType;
using bitcrusher::Square;
using bitcrusher::toUci;

const int LEAF_DEPTH      = 1;
const int MAX_LEGAL_MOVES = 218;
const int MAX_PLY         = 90;

// Sink to collect and count moves during perft testing.
struct TestPerftMoveSink : MoveSinkBase<TestPerftMoveSink> {
    uint64_t      captures_count{0};
    uint64_t      en_passant_count{0};
    uint64_t      promotions_count{0};
    std::uint64_t castling_count{0};
    // Current depth. In leaf nodes this is used to count specific move types.
    uint64_t depth{0};

    std::array<std::array<Move, MAX_LEGAL_MOVES>, MAX_PLY> moves{};
    std::array<int, MAX_PLY>                               count{};
    int                                                    ply = 0;

    template <MoveType  MoveT,
              PieceType MovedOrPromotedToPiece,
              Color     SideToMove,
              PieceType CapturedPiece = PieceType::NONE>
    void emplace(Square from, Square to) noexcept {
        moves[ply][count[ply]].setMove<MoveT, MovedOrPromotedToPiece, SideToMove, CapturedPiece>(
            from, to);
        const Move move = moves[ply][count[ply]];
        count[ply]++;

        if (depth != LEAF_DEPTH) {
            return;
        }
        if (move.isEnPassant()) {
            en_passant_count++;
        }
        if (move.isCapture()) {
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
    BoardState board_;

protected:
    void runTest();
};

} // namespace test_helpers

#endif // BITCRUSHER_PERFT_FIXTURE_HPP