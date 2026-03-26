#!/usr/bin/env bash
set -e
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"
[ -f "build/linux-tests-release/CMakeCache.txt" ] || cmake --preset linux-tests-release
cmake --build --preset linux-tests-release
bin/Release/Tests
