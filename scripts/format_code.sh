#!/usr/bin/env bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT"

if ! command -v clang-format &>/dev/null; then
    echo "Error: 'clang-format' could not be found."
    echo "Please ensure it is installed and added to your system PATH."
    exit 1
fi

echo "Formatting C++ source files..."
find src tests benchmarks -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
    -exec clang-format -i {} +
echo "Formatting complete."
