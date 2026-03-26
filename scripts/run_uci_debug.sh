#!/usr/bin/env bash
set -e
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"
[ -f "build/linux-uci-debug/CMakeCache.txt" ] || cmake --preset linux-uci-debug
cmake --build --preset linux-uci-debug
bin/Debug/Uci
