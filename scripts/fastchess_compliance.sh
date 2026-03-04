#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT" || exit 1

if ! command -v fastchess &> /dev/null; then
    echo "Error: 'fastchess' could not be found."
    echo "Please ensure it is installed and added to your system PATH."
    exit 1
fi

fastchess --compliance "$REPO_ROOT/bin/Release/Uci"
