#!/usr/bin/env bash
# usage: scripts/lint_code.sh [src/engine/include/transposition_table.hpp]
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT" || exit 1

if [ ! -f "build/compile-commands/compile_commands.json" ]; then
    echo "Generating compile_commands.json..."
    ./scripts/create_compile_commands.sh
fi

TIDY_PROJECT="-p build/compile-commands"

if [ -n "$1" ]; then
    echo "Linting $1..."
    clang-tidy $TIDY_PROJECT "$1"
else
    where_rct=$(command -v run-clang-tidy 2>/dev/null)
    if [ -n "$where_rct" ]; then
        echo "Running run-clang-tidy on compilation database..."
        run-clang-tidy $TIDY_PROJECT -j "$(nproc)"
    else
        echo "Running clang-tidy on C++ source files..."
        find src tests benchmarks -type f \( -name "*.cpp" -o -name "*.hpp" \) \
            -exec clang-tidy $TIDY_PROJECT {} +
    fi
fi

echo "Linting complete."
