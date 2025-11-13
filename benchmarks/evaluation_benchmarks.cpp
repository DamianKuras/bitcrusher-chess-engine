#include "benchmark/benchmark.h"
#include "benchmark_helper_functions.hpp"
#include "bitboard_enums.hpp"
#include "board_state.hpp"
#include "evaluation.hpp"
#include <fen_formatter.hpp>
#include <string>

using bench::utils::Epd;
using bench::utils::loadEPDsFromFile;
using bench::utils::loadFENFromFile;

class EvaluationBenchmarksFixture : public benchmark::Fixture {
public:
    std::string initial_position_fen =
        bench::utils::loadFENFromFile("../data/fens/initial_position.fen");
    bitcrusher::BoardState initial_position_board{};

    std::string kiwipete_position_fen = bench::utils::loadFENFromFile("../data/fens/kiwipete.fen");
    bitcrusher::BoardState         kiwipete_position_board{};
    std::vector<bench::utils::Epd> bratko_kopec_epds =
        loadEPDsFromFile("../data/epd/Bratko_Kopec.epd");
    std::vector<bitcrusher::BoardState> bratko_kopec_board_states{};

    EvaluationBenchmarksFixture() {
        bitcrusher::parseFEN(initial_position_fen, initial_position_board);
        bitcrusher::parseFEN(kiwipete_position_fen, kiwipete_position_board);

        // Initialize Bratko-Kopec test suite.
        int bratko_kopec_size = bratko_kopec_epds.size();
        bratko_kopec_board_states.resize(bratko_kopec_size);

        for (int i = 0; i < bratko_kopec_epds.size(); i++) {
            bitcrusher::parseFEN(bratko_kopec_epds[i].fen, bratko_kopec_board_states[i]);
        }
    }
};

BENCHMARK_DEFINE_F(EvaluationBenchmarksFixture, EvalInitialPosition)(benchmark::State& state) {
    for (auto _ : state) {
        int eval1 = bitcrusher::basicEval(initial_position_board, bitcrusher::Color::WHITE);
        int eval2 = bitcrusher::basicEval(initial_position_board, bitcrusher::Color::BLACK);
        benchmark::DoNotOptimize(eval1);
        benchmark::DoNotOptimize(eval2);
    }
}

BENCHMARK_DEFINE_F(EvaluationBenchmarksFixture, EvalKiwipetePosition)(benchmark::State& state) {
    for (auto _ : state) {
        int eval1 = bitcrusher::basicEval(kiwipete_position_board, bitcrusher::Color::WHITE);
        int eval2 = bitcrusher::basicEval(kiwipete_position_board, bitcrusher::Color::BLACK);
        benchmark::DoNotOptimize(eval1);
        benchmark::DoNotOptimize(eval2);
    }
}

BENCHMARK_DEFINE_F(EvaluationBenchmarksFixture, EvalBratkoKopecPositions)(benchmark::State& state) {
    for (auto _ : state) {
        for (auto board_state : bratko_kopec_board_states) {
            int eval1 = bitcrusher::basicEval(board_state, bitcrusher::Color::WHITE);
            int eval2 = bitcrusher::basicEval(board_state, bitcrusher::Color::BLACK);
            benchmark::DoNotOptimize(eval1);
            benchmark::DoNotOptimize(eval2);
        }
    }
}

const int REPETITION_COUNT = 2;
BENCHMARK_REGISTER_F(EvaluationBenchmarksFixture, EvalInitialPosition)
    ->Repetitions(REPETITION_COUNT);
BENCHMARK_REGISTER_F(EvaluationBenchmarksFixture, EvalKiwipetePosition)
    ->Repetitions(REPETITION_COUNT);
BENCHMARK_REGISTER_F(EvaluationBenchmarksFixture, EvalBratkoKopecPositions)
    ->Repetitions(REPETITION_COUNT);
