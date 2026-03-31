#include "benchmark/benchmark.h"
#include "benchmark_helper_functions.hpp"
#include "move_sink.hpp"
#include "search_manager.hpp"
#include <chrono>
#include <cstdint>
#include <thread>

namespace {

constexpr std::string_view INITIAL_POSITION_PATH = "../data/fens/initial_position.fen";
constexpr std::string_view KIWIPETE_PATH         = "../data/fens/kiwipete.fen";

// Search time per timed run. Longer = more stable NPS; shorter = faster benchmark.
constexpr int SEARCH_TIME_MS = 1000;

// Cap the maximum thread count tested even on high-core-count machines.
constexpr int MAX_BENCHMARK_THREADS = 8;

} // namespace

using bench::utils::loadFENFromFile;

static void registerThreadArgs(benchmark::internal::Benchmark* b) {
    const int hw    = static_cast<int>(std::thread::hardware_concurrency());
    const int max_t = std::min(hw > 0 ? hw : 4, MAX_BENCHMARK_THREADS);
    for (int t = 1; t <= max_t; t *= 2) {
        b->Arg(t);
    }
    if ((max_t & (max_t - 1)) != 0) {
        b->Arg(max_t);
    }
}

class SearchThreadingFixture : public benchmark::Fixture {
public:
    std::string initial_position_fen;
    std::string kiwipete_fen;

    SearchThreadingFixture() {
        initial_position_fen = loadFENFromFile(INITIAL_POSITION_PATH);
        kiwipete_fen         = loadFENFromFile(KIWIPETE_PATH);
    }
};

// Runs a timed search on the given FEN with num_threads and returns nodes/second.
static double timedSearch(const std::string& fen, int num_threads) {
    bitcrusher::SearchManager manager;
    manager.setMaxCores(num_threads);
    manager.setPos(fen);
    bitcrusher::SearchParameters params;
    params.move_time_ms = SEARCH_TIME_MS;

    auto t0 = std::chrono::steady_clock::now();
    manager.startSearch<bitcrusher::FastMoveSink>(params);
    manager.waitUntilSearchFinished();
    const double elapsed_s =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

    return static_cast<double>(manager.getNodeCount()) / elapsed_s;
}

// For each thread count N, runs a 1-thread baseline and an N-thread search back-to-back.
// Reports:
//   NPS_1T       — baseline nodes/second (1 thread)
//   NPS_NT       — nodes/second with N threads
//   Speedup      — NPS_NT / NPS_1T  (e.g. 1.84 means 84% faster)
//   Efficiency   — Speedup / N * 100%  (e.g. 92% means each extra thread is 92% productive)
//
// All timing is done with std::chrono; the benchmark loop runs entirely under PauseTiming.
// state.range(0) = N (number of threads for the N-thread run).
BENCHMARK_DEFINE_F(SearchThreadingFixture, Threading_InitialPosition)(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));

    double sum_nps_1t = 0.0;
    double sum_nps_nt = 0.0;
    int    count      = 0;

    for (auto _ : state) {
        const double nps_1t = timedSearch(initial_position_fen, 1);
        // For n=1, reuse the baseline to avoid a redundant search and a nonsensical
        // self-comparison.
        const double nps_nt = (n == 1) ? nps_1t : timedSearch(initial_position_fen, n);
        sum_nps_1t += nps_1t;
        sum_nps_nt += nps_nt;
        ++count;
        benchmark::DoNotOptimize(nps_1t);
        benchmark::DoNotOptimize(nps_nt);
    }

    const double avg_1t     = sum_nps_1t / count;
    const double avg_nt     = sum_nps_nt / count;
    const double speedup    = avg_nt / avg_1t;
    const double efficiency = speedup / n * 100.0;

    state.counters["NPS_1T"]       = benchmark::Counter(avg_1t, benchmark::Counter::kDefaults,
                                                        benchmark::Counter::OneK::kIs1000);
    state.counters["NPS_NT"]       = benchmark::Counter(avg_nt, benchmark::Counter::kDefaults,
                                                        benchmark::Counter::OneK::kIs1000);
    state.counters["Speedup"]      = speedup;
    state.counters["Efficiency_%"] = efficiency;
}

// Same as Threading_InitialPosition but for kiwipete (complex middlegame, high branching factor).
BENCHMARK_DEFINE_F(SearchThreadingFixture, Threading_Kiwipete)(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));

    double sum_nps_1t = 0.0;
    double sum_nps_nt = 0.0;
    int    count      = 0;

    for (auto _ : state) {
        const double nps_1t = timedSearch(kiwipete_fen, 1);
        const double nps_nt = (n == 1) ? nps_1t : timedSearch(kiwipete_fen, n);
        sum_nps_1t += nps_1t;
        sum_nps_nt += nps_nt;
        ++count;
        benchmark::DoNotOptimize(nps_1t);
        benchmark::DoNotOptimize(nps_nt);
    }

    const double avg_1t     = sum_nps_1t / count;
    const double avg_nt     = sum_nps_nt / count;
    const double speedup    = avg_nt / avg_1t;
    const double efficiency = speedup / n * 100.0;

    state.counters["NPS_1T"]       = benchmark::Counter(avg_1t, benchmark::Counter::kDefaults,
                                                        benchmark::Counter::OneK::kIs1000);
    state.counters["NPS_NT"]       = benchmark::Counter(avg_nt, benchmark::Counter::kDefaults,
                                                        benchmark::Counter::OneK::kIs1000);
    state.counters["Speedup"]      = speedup;
    state.counters["Efficiency_%"] = efficiency;
}

BENCHMARK_REGISTER_F(SearchThreadingFixture, Threading_InitialPosition)
    ->Apply(registerThreadArgs)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_REGISTER_F(SearchThreadingFixture, Threading_Kiwipete)
    ->Apply(registerThreadArgs)
    ->Unit(benchmark::kMillisecond);
