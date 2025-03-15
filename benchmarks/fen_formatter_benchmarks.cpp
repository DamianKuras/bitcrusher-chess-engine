#include "benchmark/benchmark.h"
#include "benchmark_helper_functions.hpp"
#include "fen_formatter.hpp"

class FenFormatterBenchmarksFixture : public benchmark::Fixture {
public:
    std::string            initial_position;
    std::string            kiwipete_position;
    bitcrusher::BoardState board;

    FenFormatterBenchmarksFixture() {
        initial_position  = bench::utils::loadFenFromFile("../data/fens/initial_position.fen");
        kiwipete_position = bench::utils::loadFenFromFile("../data/fens/kiwipete.fen");
    }
};

BENCHMARK_DEFINE_F(FenFormatterBenchmarksFixture,
                   ParseFenInitialPosition)(benchmark::State& state) {
    for (auto _ : state) {
        bitcrusher::parseFEN(initial_position, board);
    }
}

BENCHMARK_DEFINE_F(FenFormatterBenchmarksFixture,
                   ParseFenKiwipetePosition)(benchmark::State& state) {
    for (auto _ : state) {
        bitcrusher::parseFEN(kiwipete_position, board);
    }
}

BENCHMARK_REGISTER_F(FenFormatterBenchmarksFixture, ParseFenInitialPosition)->Repetitions(10);
BENCHMARK_REGISTER_F(FenFormatterBenchmarksFixture, ParseFenKiwipetePosition)->Repetitions(10);
