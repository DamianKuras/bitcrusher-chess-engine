#include "benchmark/benchmark.h"
#include "benchmark_helper_functions.hpp"
#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "fen_formatter.hpp"
#include "legal_move_generators/legal_moves_generator.hpp"
#include "move_processor.hpp"
#include "move_sink.hpp"
#include "restriction_context.hpp"

namespace {

const int     PERFT_LIMIT           = 5;
constexpr int BENCHMARK_REPETITIONS = 3;

template <bitcrusher::Color SideToMove>
[[nodiscard]] uint64_t
perft(int depth, bitcrusher::BoardState& board, bitcrusher::MoveProcessor move_processor) {
    uint64_t                       leaf_node_count = 0;
    bitcrusher::RestrictionContext restriction_context;
    updateRestrictionContext<SideToMove>(board, restriction_context);
    bitcrusher::FastMoveSink sink;
    generateLegalMoves<bitcrusher::FastMoveSink, SideToMove>(board, restriction_context, sink);

    const std::size_t n = sink.count;
    if (depth == 1) {
        return static_cast<uint64_t>(n);
    }

    for (std::size_t i = 0; i < n; ++i) {
        const bitcrusher::Move& move = sink.moves[i];
        updateRestrictionContext<SideToMove>(board, restriction_context);
        move_processor.applyMove(board, move);
        leaf_node_count += perft<! SideToMove>(depth - 1, board, move_processor);
        move_processor.undoMove(board, move);
    }

    return leaf_node_count;
}

} // namespace

class PerftBenchmarksFixture : public benchmark::Fixture {
public:
    std::string initial_position;
    std::string kiwipete_position;

    bitcrusher::BoardState initial_position_board;
    bitcrusher::BoardState kiwipete_position_board;

    PerftBenchmarksFixture() {
        // Loading fen from file to prevent precomputed result.
        initial_position  = bench::utils::loadFenFromFile("../data/fens/initial_position.fen");
        kiwipete_position = bench::utils::loadFenFromFile("../data/fens/kiwipete.fen");
        bitcrusher::parseFEN(initial_position, initial_position_board);
        bitcrusher::parseFEN(kiwipete_position, kiwipete_position_board);
    }
};

BENCHMARK_DEFINE_F(PerftBenchmarksFixture, PerftInitial)(benchmark::State& state) {
    const int depth       = static_cast<int>(state.range(0)); // Read the depth parameter
    uint64_t  total_nodes = 0;
    const bitcrusher::MoveProcessor move_processor{};

    for (auto _ : state) {
        total_nodes +=
            perft<bitcrusher::Color::WHITE>(depth, initial_position_board, move_processor); //
        benchmark::DoNotOptimize(total_nodes);
    }

    state.counters["leaf_nodes_per_second"] =
        benchmark::Counter(static_cast<double>(total_nodes), benchmark::Counter::kIsRate);
}

BENCHMARK_DEFINE_F(PerftBenchmarksFixture, PerftKiwipete)(benchmark::State& state) {
    const int depth = static_cast<int>(state.range(0)); // Read the depth parameter
    const bitcrusher::MoveProcessor move_processor{};
    uint64_t                        total_nodes = 0;
    for (auto _ : state) {
        // Run perft at the given depth
        total_nodes +=
            perft<bitcrusher::Color::WHITE>(depth, kiwipete_position_board, move_processor); //
        benchmark::DoNotOptimize(total_nodes);
    }
    state.counters["leaf_nodes_per_second"] =
        benchmark::Counter(static_cast<double>(total_nodes), benchmark::Counter::kIsRate);
}

BENCHMARK_REGISTER_F(PerftBenchmarksFixture, PerftInitial)
    ->DenseRange(1, PERFT_LIMIT)
    ->Repetitions(BENCHMARK_REPETITIONS);

BENCHMARK_REGISTER_F(PerftBenchmarksFixture, PerftKiwipete)
    ->DenseRange(1, PERFT_LIMIT)
    ->Repetitions(BENCHMARK_REPETITIONS);