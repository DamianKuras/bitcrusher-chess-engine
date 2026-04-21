#!/usr/bin/env bash
# run_elo_test.sh [fstc|stc|ltc|2phase] [branch_dev] [concurrency]
# Runs a fastchess SPRT Elo test comparing branch_dev against main.
# If branch_dev does not exist on the remote, the current local state is used.
#
# fstc    - Fast filter: 3+0.03, SPRT elo1=10. Quickly rejects bad patches.
# stc     - Standard:    10+0.1, SPRT elo1=5.  Default confirmation test.
# ltc     - Long:        60+0.6, SPRT elo1=5.
# 2phase  - Chains fstc (max 200 rounds) into stc. Only promotes if H1 accepted.
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(realpath "$SCRIPT_DIR/..")"
cd "$REPO_ROOT"

if ! command -v fastchess &>/dev/null; then
    echo "Error: 'fastchess' could not be found."
    echo "Please ensure it is installed and added to your system PATH."
    exit 1
fi

TC_TYPE="${1:-2phase}"
BRANCH_DEV="${2:-dev}"
CONCURRENCY="${3:-2}"
BRANCH_BASE="main"

case "$TC_TYPE" in
    fstc)   TC="3+0.03";  SPRT_ELO1=10 ;;
    stc)    TC="10+0.1";  SPRT_ELO1=5  ;;
    ltc)    TC="60+0.6";  SPRT_ELO1=5  ;;
    2phase) ;;
    *) echo "Error: Unknown TC type '$TC_TYPE'. Use fstc|stc|ltc|2phase."; exit 1 ;;
esac

echo "Starting Automated $TC_TYPE SPRT Elo Testing"
echo "Comparing: $BRANCH_DEV (New) vs $BRANCH_BASE (Base)"
if [ "$TC_TYPE" = "2phase" ]; then
    echo "Phase 1: 3+0.03, elo1=10, max 200 rounds  ->  Phase 2: 10+0.1, elo1=5"
else
    echo "Time Control: $TC | SPRT elo0=0, elo1=$SPRT_ELO1"
fi
echo "Concurrency : $CONCURRENCY threads"
echo "================================================================"

TMP_DIR="$(mktemp -d /tmp/bitcrusher_sprt_XXXXXX)"
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

echo "Compiling Base Engine ($BRANCH_BASE)..."
BASE_EXE="$("$SCRIPT_DIR/build_engine_tournament.sh" "$BASE_DIR")"

echo "Compiling Dev Engine ($BRANCH_DEV)..."
DEV_EXE="$("$SCRIPT_DIR/build_engine_tournament.sh" "$DEV_DIR")"

echo "Compilations successful."

run_fastchess() {
    local tc="$1" elo1="$2" rounds="$3" log_file="${4:-}"
    local args=(
        -engine cmd="$DEV_EXE"  name="Bitcrusher_${BRANCH_DEV}_new"
        -engine cmd="$BASE_EXE" name="Bitcrusher_${BRANCH_BASE}_base"
        -each tc="$tc"
        -rounds "$rounds" -games 2 -repeat
        -concurrency "$CONCURRENCY"
        -openings file="$REPO_ROOT/data/pgn/8move_v3.pgn" format=pgn order=random
        -sprt elo0=0.0 elo1="$elo1" alpha=0.05 beta=0.05
        -recover
    )
    if [ -n "$log_file" ]; then
        fastchess "${args[@]}" 2>&1 | tee "$log_file"
    else
        fastchess "${args[@]}"
    fi
}

if [ "$TC_TYPE" = "2phase" ]; then
    echo
    echo "=== Phase 1: Fast Filter (3+0.03, elo1=10, max 200 rounds) ==="
    phase1_log="$TMP_DIR/phase1.log"
    run_fastchess "3+0.03" 10 200 "$phase1_log"
    echo
    if grep -qi "h1.*accept\|accept.*h1" "$phase1_log"; then
        echo "Phase 1: H1 accepted. Proceeding to Phase 2."
        echo
        echo "=== Phase 2: Confirmation (10+0.1, elo1=5, max 500 rounds) ==="
        run_fastchess "10+0.1" 5 500
    elif grep -qi "h0.*accept\|accept.*h0" "$phase1_log"; then
        echo "Phase 1: H0 accepted. Patch rejected."
    else
        echo "Phase 1: Inconclusive (round cap reached)."
        echo "The improvement is likely small (0-10 Elo). Run 'stc' directly for fine-grained testing."
    fi
else
    echo "Starting $TC_TYPE fastchess match..."
    run_fastchess "$TC" "$SPRT_ELO1" 500
fi

echo
echo "$TC_TYPE match completed."
