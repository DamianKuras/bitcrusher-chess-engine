#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT" || exit 1

# Generate Makefiles
premake5 gmake --with-tests --with-bmi2

# Build and run
cd build &&
make clean &&
make Tests config=release_x64 && 
../bin/Release/Tests
