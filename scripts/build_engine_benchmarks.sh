#!/usr/bin/env bash
# build_engine_benchmarks.sh [WORK_DIR]
# Configures (first time only) and builds BenchmarkRunner.
# Prints the absolute path to the binary on stdout; all errors go to stderr.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [ -n "$1" ]; then
    WORK_DIR="$(realpath "$1")"
else
    WORK_DIR="$(realpath "$SCRIPT_DIR/..")"
fi

if [ -z "$VCPKG_ROOT" ]; then
    echo "Error: VCPKG_ROOT is not set." >&2
    exit 1
fi

BUILD_DIR="$WORK_DIR/build/linux-benchmarks"
EXE="$WORK_DIR/bin/Release/BenchmarkRunner"

if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
    cmake -S "$WORK_DIR" -B "$BUILD_DIR" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
        -DBITCRUSHER_BUILD_BENCHMARKS=ON \
        -DBITCRUSHER_WITH_BMI2=ON >/dev/null 2>&1
fi

cmake --build "$BUILD_DIR" --target BenchmarkRunner >/dev/null 2>&1

if [ ! -f "$EXE" ]; then
    echo "Error: Binary not found after build." >&2
    exit 1
fi
echo "$EXE"
