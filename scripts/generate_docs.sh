#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT" || exit 1

echo "Generating documentation..."
rm -rf html/ latex/
doxygen Doxyfile
echo "Documentation generated in html/ and latex/"
