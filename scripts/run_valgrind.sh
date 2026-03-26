#!/usr/bin/env bash
set -e
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"
[ -f "build/linux-uci-release/CMakeCache.txt" ] || cmake --preset linux-uci-release
cmake --build --preset linux-uci-release
valgrind --tool=cachegrind --cache-sim=yes bin/Release/Uci
