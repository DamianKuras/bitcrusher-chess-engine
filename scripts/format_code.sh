#!/usr/bin/env bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$REPO_ROOT"

# Prefer clang-format-20 to match CI. Fall back to VS (Windows) then system PATH.
find_clang_format() {
    if command -v clang-format-20 >/dev/null 2>&1; then
        echo "clang-format-20"; return
    fi
    if [ -x "/usr/lib/llvm20/bin/clang-format" ]; then
        echo "/usr/lib/llvm20/bin/clang-format"; return
    fi
    local vswhere="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
    if [ -f "$vswhere" ]; then
        local vs_path
        vs_path=$("$vswhere" -latest -property installationPath 2>/dev/null)
        local cf="${vs_path}/VC/Tools/Llvm/x64/bin/clang-format.exe"
        if [ -f "$cf" ]; then
            echo "$cf"; return
        fi
    fi
    if command -v clang-format >/dev/null 2>&1; then
        echo "clang-format"; return
    fi
    echo ""
}

CLANG_FORMAT=$(find_clang_format)

clang_format_install_hint() {
    if command -v pacman >/dev/null 2>&1; then
        echo "  paru -S clang20   # or: yay -S clang20"
    else
        echo "  sudo apt-get install clang-format-20"
    fi
}

if [ -z "$CLANG_FORMAT" ]; then
    echo "Error: clang-format not found. Install clang-format-20 to match CI:"
    clang_format_install_hint
    exit 1
fi

CF_VERSION=$("$CLANG_FORMAT" --version 2>/dev/null | grep -o '[0-9]*\.[0-9]*' | head -1)
CF_MAJOR=${CF_VERSION%%.*}
if [ -n "$CF_MAJOR" ] && [ "$CF_MAJOR" != "20" ]; then
    echo "Warning: clang-format version $CF_VERSION found; CI uses 20.x. Results may differ."
    echo "  Install clang-format-20:"
    clang_format_install_hint
fi

echo "Formatting C++ source files with $("$CLANG_FORMAT" --version | head -1)..."
find src tests benchmarks -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
    -exec "$CLANG_FORMAT" -i {} +
echo "Formatting complete."
