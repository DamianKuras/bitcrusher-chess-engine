#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT" || exit 1

if ! command -v cutechess-cli &> /dev/null; then
    echo "Error: 'cutechess-cli' could not be found."
    echo "Please ensure it is installed and added to your system PATH."
    exit 1
fi

if ! command -v stockfish &> /dev/null; then
    echo "Error: 'stockfish' could not be found."
    echo "Please ensure it is installed and added to your system PATH."
    exit 1
fi

cutechess-cli -engine cmd=Uci -engine cmd=stockfish -each proto=uci tc=40/30 -rounds 1
