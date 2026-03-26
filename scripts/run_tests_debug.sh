#!/usr/bin/env bash
set -e
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"
[ -f "build/linux-tests-debug/CMakeCache.txt" ] || cmake --preset linux-tests-debug
cmake --build --preset linux-tests-debug
bin/Debug/Tests --gtest_filter=-*slow
