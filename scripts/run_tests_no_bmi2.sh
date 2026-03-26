#!/usr/bin/env bash
set -e
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"
[ -f "build/linux-tests-no-bmi2/CMakeCache.txt" ] || cmake --preset linux-tests-no-bmi2
cmake --build --preset linux-tests-no-bmi2
bin/Release/Tests --gtest_filter=-*slow
