#!/usr/bin/env bash
# run_benchmark_comparison.sh [branch_dev] [bench_filter]
# Builds and runs Google Benchmarks for branch_dev and main, then compares.
# If branch_dev does not exist on the remote, the current local state is used.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(realpath "$SCRIPT_DIR/..")"
cd "$REPO_ROOT"

BRANCH_DEV="${1:-dev}"
BENCH_FILTER="${2:-.}"
BRANCH_BASE="main"

echo " Starting Benchmark Comparison"
echo " Comparing: $BRANCH_DEV (New) vs $BRANCH_BASE (Base)"
echo "================================================================"

TMP_DIR="$(mktemp -d /tmp/bitcrusher_bench_XXXXXX)"
BASE_DIR="$TMP_DIR/base"
DEV_DIR="$TMP_DIR/dev"

cleanup() {
    rm -rf "$TMP_DIR"
}
trap cleanup EXIT

REMOTE_URL="$(git config --get remote.origin.url)"
if [ -z "$REMOTE_URL" ]; then
    echo "Error: Could not determine remote origin URL."
    exit 1
fi
echo "Using remote: $REMOTE_URL"

echo "Cloning base branch '$BRANCH_BASE'..."
git clone -q --branch "$BRANCH_BASE" "$REMOTE_URL" "$BASE_DIR"

if git ls-remote --exit-code --heads "$REMOTE_URL" "$BRANCH_DEV" &>/dev/null; then
    echo "Cloning dev branch '$BRANCH_DEV' from remote..."
    git clone -q --branch "$BRANCH_DEV" "$REMOTE_URL" "$DEV_DIR"
else
    echo "Dev branch '$BRANCH_DEV' not found on remote, copying current local state..."
    rsync -a --exclude=bin/ --exclude=obj/ --exclude=build/ "$REPO_ROOT/" "$DEV_DIR/"
fi

echo "Compiling base benchmarks ($BRANCH_BASE)..."
BASE_EXE="$("$SCRIPT_DIR/build_engine_benchmarks.sh" "$BASE_DIR")"

echo "Compiling dev benchmarks ($BRANCH_DEV)..."
DEV_EXE="$("$SCRIPT_DIR/build_engine_benchmarks.sh" "$DEV_DIR")"

BASE_JSON="$TMP_DIR/base_bench.json"
DEV_JSON="$TMP_DIR/dev_bench.json"

echo "Running base benchmarks..."
(cd "$BASE_DIR/benchmarks" && "$BASE_EXE" \
    --benchmark_filter="$BENCH_FILTER" \
    --benchmark_out_format=json --benchmark_out="$BASE_JSON" \
    --benchmark_repetitions=20 \
    --benchmark_report_aggregates_only=true \
    --benchmark_min_warmup_time=0.5 2>/dev/null)

echo "Running dev benchmarks..."
(cd "$DEV_DIR/benchmarks" && "$DEV_EXE" \
    --benchmark_filter="$BENCH_FILTER" \
    --benchmark_out_format=json --benchmark_out="$DEV_JSON" \
    --benchmark_repetitions=20 \
    --benchmark_report_aggregates_only=true \
    --benchmark_min_warmup_time=0.5 2>/dev/null)

echo
python "$SCRIPT_DIR/compare_benchmarks.py" "$BASE_JSON" "$DEV_JSON" "$BRANCH_BASE" "$BRANCH_DEV"
echo
echo "Done."
