#!/usr/bin/env bash
set -e
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"
[ -f "build/linux-benchmarks-release/CMakeCache.txt" ] || cmake --preset linux-benchmarks-release
cmake --build --preset linux-benchmarks-release
mkdir -p benchmarks/results
cd benchmarks
../bin/Release/BenchmarkRunner --benchmark_out_format=json --benchmark_out=results/result.json --benchmark_report_aggregates_only=true --benchmark_min_warmup_time=0.2 "$@"
