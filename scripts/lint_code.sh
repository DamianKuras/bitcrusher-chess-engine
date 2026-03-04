#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT" || exit 1

echo "Running clang-tidy on C++ source files..."
if [ ! -f "compile_commands.json" ]; then
    echo "Generating compile_commands.json..."
    ./scripts/create_compile_commands.sh
fi

find src tests benchmarks -type f \( -name "*.cpp" -o -name "*.h" \) -exec clang-tidy -p . {} +
echo "Linting complete."
