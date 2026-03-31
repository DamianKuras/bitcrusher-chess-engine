#include "benchmark/benchmark.h"
#include "benchmark_helper_functions.hpp"
#include "fen_formatter.hpp"

class FENFormatterBenchmarksFixture : public benchmark::Fixture {
public:
    std::string            initial_position;
    std::string            kiwipete_position;
    bitcrusher::BoardState board;

    FENFormatterBenchmarksFixture() {
        initial_position  = bench::utils::loadFENFromFile("../data/fens/initial_position.fen");
        kiwipete_position = bench::utils::loadFENFromFile("../data/fens/kiwipete.fen");
    }
};

BENCHMARK_DEFINE_F(FENFormatterBenchmarksFixture,
                   ParseFenInitialPosition)(benchmark::State& state) {
    for (auto _ : state) {
        bitcrusher::parseFEN(initial_position, board);
    }
}

BENCHMARK_DEFINE_F(FENFormatterBenchmarksFixture,
                   ParseFenKiwipetePosition)(benchmark::State& state) {
    for (auto _ : state) {
        bitcrusher::parseFEN(kiwipete_position, board);
    }
}

const int REPETITION_COUNT = 2;
BENCHMARK_REGISTER_F(FENFormatterBenchmarksFixture, ParseFenInitialPosition)
    ->Repetitions(REPETITION_COUNT);
BENCHMARK_REGISTER_F(FENFormatterBenchmarksFixture, ParseFenKiwipetePosition)
    ->Repetitions(REPETITION_COUNT);
