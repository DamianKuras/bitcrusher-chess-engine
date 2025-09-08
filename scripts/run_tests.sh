#!/usr/bin/env bash
# Navigate to repo root (one levels up from this script)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT" || exit 1

# Generate Makefiles
premake5 gmake --with-tests

# Build and run
cd build
make clean
make Tests config=debug_x64
../bin/Debug/Tests --gtest_filter=-*slow
