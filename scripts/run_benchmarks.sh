#!/usr/bin/env bash
# Navigate to repo root (one levels up from this script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT" || exit 1

# Generate Makefiles
premake5 gmake --with-benchmarks --with-bmi2

# Build and run
cd build
make clean
make BenchmarkRunner config=release_x64 && ../bin/Release/BenchmarkRunner --benchmark_out_format=json --benchmark_out=../benchmarks/results/result.json --benchmark_report_aggregates_only=true --benchmark_min_warmup_time=0.2
