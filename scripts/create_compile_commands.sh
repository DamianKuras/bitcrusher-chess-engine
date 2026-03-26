#!/usr/bin/env bash
set -e
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"
cmake -S . -B build/compile-commands -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBITCRUSHER_BUILD_UCI=ON \
    -DBITCRUSHER_BUILD_TESTS=ON \
    -DBITCRUSHER_BUILD_BENCHMARKS=ON \
    -DBITCRUSHER_WITH_BMI2=ON
cp build/compile-commands/compile_commands.json compile_commands.json
echo "compile_commands.json written to repo root."
