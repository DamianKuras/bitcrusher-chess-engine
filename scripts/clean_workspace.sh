#!/usr/bin/env bash
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"
echo "Cleaning workspace..."
rm -rf bin/ build/
echo "Workspace clean."
