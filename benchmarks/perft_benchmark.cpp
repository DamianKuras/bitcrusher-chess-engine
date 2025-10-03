#include "benchmark/benchmark.h"
#include "benchmark_helper_functions.hpp"
#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "concepts.hpp"
#include "fen_formatter.hpp"
#include "legal_move_generators/legal_moves_generator.hpp"
#include "move_processor.hpp"
#include "move_sink.hpp"
#include "restriction_context.hpp"

namespace {

const int     PERFT_LIMIT           = 5;
constexpr int BENCHMARK_REPETITIONS = 3;

template <bitcrusher::Color SideToMove>
[[nodiscard]] uint64_t perft(int                       depth,
                             bitcrusher::BoardState&   board,
                             bitcrusher::MoveProcessor move_processor,
                             bitcrusher::FastMoveSink& sink,
                             int                       ply = 0) {
    uint64_t                       leaf_node_count = 0;
    bitcrusher::RestrictionContext restriction_context;
    updateRestrictionContext<SideToMove>(board, restriction_context);
    sink.count[ply] = 0;
    sink.ply        = ply;
    generateLegalMoves<SideToMove>(board, restriction_context, sink);

    if (depth == 1) {
        return static_cast<uint64_t>(sink.count[ply]);
    }

    for (std::size_t i = 0; i < sink.count[ply]; ++i) {
        updateRestrictionContext<SideToMove>(board, restriction_context);
        move_processor.applyMove(board, sink.moves[ply][i]);
        leaf_node_count += perft<! SideToMove>(depth - 1, board, move_processor, sink, ply + 1);
        move_processor.undoMove(board, sink.moves[ply][i]);
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
        initial_position  = bench::utils::loadFENFromFile("../data/fens/initial_position.fen");
        kiwipete_position = bench::utils::loadFENFromFile("../data/fens/kiwipete.fen");
        bitcrusher::parseFEN(initial_position, initial_position_board);
        bitcrusher::parseFEN(kiwipete_position, kiwipete_position_board);
    }
};

BENCHMARK_DEFINE_F(PerftBenchmarksFixture, PerftInitial)(benchmark::State& state) {
    const int depth       = static_cast<int>(state.range(0)); // Read the depth parameter
    uint64_t  total_nodes = 0;
    const bitcrusher::MoveProcessor move_processor{};
    bitcrusher::FastMoveSink        sink;
    for (auto _ : state) {
        total_nodes +=
            perft<bitcrusher::Color::WHITE>(depth, initial_position_board, move_processor, sink);
        benchmark::DoNotOptimize(total_nodes);
    }

    state.counters["leaf_nodes_per_second"] =
        benchmark::Counter(static_cast<double>(total_nodes), benchmark::Counter::kIsRate);
}

BENCHMARK_DEFINE_F(PerftBenchmarksFixture, PerftKiwipete)(benchmark::State& state) {
    const int depth = static_cast<int>(state.range(0)); // Read the depth parameter
    const bitcrusher::MoveProcessor move_processor{};
    uint64_t                        total_nodes = 0;
    bitcrusher::FastMoveSink        sink;
    for (auto _ : state) {
        // Run perft at the given depth
        total_nodes += perft<bitcrusher::Color::WHITE>(depth, kiwipete_position_board,
                                                       move_processor, sink); //
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