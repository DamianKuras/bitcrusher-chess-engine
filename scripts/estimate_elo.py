#!/usr/bin/env python3
"""
Universal Chess Engine Elo Estimator
======================================
Reference engine: Stockfish with UCI_LimitStrength.
Anchor:           CCRL 40/4 (official Stockfish calibration source).
Method:           Adaptive binary search  ->  multi-level gauntlet  ->  Bayesian MLE.
Statistics:       Fisher-information 95% confidence interval on the MLE estimate.

Usage:
  python elo_estimator.py                          # auto-detect engine
  python elo_estimator.py --games 200              # more games = tighter CI
  python elo_estimator.py --engine ./myengine
  python elo_estimator.py --sf /path/to/stockfish  # skip auto-download
  python elo_estimator.py --tc 60+0.6              # LTC for better accuracy
  python elo_estimator.py --openings openings.pgn
  python elo_estimator.py --probe-only             # bracket only, no gauntlet

Requirements: fastchess must be in PATH.
              Stockfish is auto-downloaded from GitHub if --sf is not given.

Time control note:
  Stockfish's UCI_LimitStrength is calibrated at LTC (120s+1s per side).
  The default TC here (10+0.1) is faster and gives a slightly different scale
  but is fine for development testing. Use --tc 60+0.6 for results closer to
  the official CCRL 40/4 scale.
"""

import os, sys, re, math, json, platform, shutil, argparse, subprocess
import zipfile, tarfile, urllib.request
from pathlib import Path


# ─────────────────────────────────────────────────────────────────────────────
# Constants
# ─────────────────────────────────────────────────────────────────────────────

# Stockfish UCI_Elo range (SF 16+).
# Officially calibrated to CCRL 40/4 using Ordo. (github.com/official-stockfish)
SF_ELO_MIN = 1320
SF_ELO_MAX = 3190

CACHE_DIR = Path(__file__).parent / "elo_estimator_cache"


# ─────────────────────────────────────────────────────────────────────────────
# CPU helpers
# ─────────────────────────────────────────────────────────────────────────────

def _physical_cores():
    """
    Best-effort physical core count. Falls back to logical count, then 2.
    We deliberately avoid psutil to keep zero dependencies.
    """
    try:
        # Works on Linux
        out = subprocess.run(
            ["lscpu"], capture_output=True, text=True
        ).stdout
        m = re.search(r"Core\(s\) per socket:\s*(\d+)", out)
        sockets = re.search(r"Socket\(s\):\s*(\d+)", out)
        if m and sockets:
            return int(m.group(1)) * int(sockets.group(1))
    except Exception:
        pass
    try:
        # Works on macOS
        out = subprocess.run(
            ["sysctl", "-n", "hw.physicalcpu"], capture_output=True, text=True
        ).stdout.strip()
        if out.isdigit():
            return int(out)
    except Exception:
        pass
    try:
        # Windows: WMIC
        out = subprocess.run(
            ["wmic", "cpu", "get", "NumberOfCores", "/value"],
            capture_output=True, text=True
        ).stdout
        m = re.search(r"NumberOfCores=(\d+)", out)
        if m:
            return int(m.group(1))
    except Exception:
        pass
    # Fallback to logical cores / 2 (accounts for hyperthreading)
    logical = os.cpu_count() or 4
    return max(1, logical // 2)


def _safe_concurrency():
    """
    Number of concurrent game pairs to run.
    Each pair uses 2 engine processes, so we cap at physical_cores // 2.
    This keeps time controls accurate and avoids CPU starvation.
    """
    cores = _physical_cores()
    return max(1, cores // 2)


# ─────────────────────────────────────────────────────────────────────────────
# Elo mathematics
# ─────────────────────────────────────────────────────────────────────────────

def elo_from_score(score):
    """Logistic Elo difference from a win percentage in [0, 1]."""
    score = max(1e-6, min(1.0 - 1e-6, score))
    return -400.0 * math.log10(1.0 / score - 1.0)


def score_from_elo(diff):
    """Expected score given Elo difference."""
    return 1.0 / (1.0 + 10.0 ** (-diff / 400.0))


def compute_stats(wins, losses, draws):
    """
    Returns (elo_diff, lo95, hi95) using:
      - Laplace-smoothed point estimate (handles 0% / 100% cleanly)
      - Wilson score interval -> converted to Elo for the 95% CI
    """
    n = wins + losses + draws
    if n == 0:
        return 0.0, -800.0, 800.0

    raw = (wins + draws * 0.5) / n

    # Point estimate: +1 synthetic win and +1 loss avoids ±inf at 0%/100%
    smoothed = (wins + draws * 0.5 + 1.0) / (n + 2.0)
    diff = elo_from_score(smoothed)

    # Wilson score CI (z = 1.96 for 95%)
    z = 1.96
    denom = 1.0 + z * z / n
    centre = (raw + z * z / (2 * n)) / denom
    spread = z * math.sqrt(max(0.0, raw * (1 - raw) / n + z * z / (4 * n * n))) / denom

    lo = elo_from_score(max(1e-6, centre - spread))
    hi = elo_from_score(min(1.0 - 1e-6, centre + spread))
    return diff, lo, hi


def bayesian_mle(results):
    """
    Maximum-likelihood Elo estimate from a gauntlet.
    results: list of (opponent_elo, wins, losses, draws)

    Uses Newton-Raphson on the log-likelihood of the logistic model.
    Converges in ~5-10 iterations rather than 20000 gradient steps.

    Returns (estimated_elo, sigma_95):
      sigma_95 = half-width of the 95% CI from Fisher information.
    """
    if not results:
        return None, None

    total_games = sum(w + l + d for _, w, l, d in results)
    if total_games == 0:
        return None, None

    k = math.log(10.0) / 400.0   # logistic slope constant

    # Initialise at the games-weighted mean opponent Elo
    engine_elo = sum(elo * (w + l + d) for elo, w, l, d in results) / total_games

    # Newton-Raphson: theta_new = theta - f'(theta) / f''(theta)
    # f'  = gradient of log-likelihood = sum_i n_i * (s_i - W_i(theta))
    # f'' = -Fisher information        = -sum_i n_i * W_i * (1-W_i) * k^2
    for iteration in range(50):
        grad = 0.0
        fisher = 0.0
        for opp_elo, wins, losses, draws in results:
            n = wins + losses + draws
            if n == 0:
                continue
            # Laplace-smoothed observed score (avoids log(0) at 0%/100%)
            score = (wins + draws * 0.5 + 0.5) / (n + 1.0)
            we = score_from_elo(engine_elo - opp_elo)
            grad   += n * (score - we)
            fisher += n * we * (1.0 - we) * k * k

        if fisher < 1e-12:
            break   # degenerate case

        step = grad / (fisher / (k * k)) * k   # Newton step in Elo units
        # Clamp step to avoid overshooting in early iterations
        step = max(-200.0, min(200.0, step))
        engine_elo += step

        if abs(step) < 0.001:   # converged
            break

    # Final Fisher information at the MLE for the CI
    fisher_final = 0.0
    for opp_elo, wins, losses, draws in results:
        n = wins + losses + draws
        if n == 0:
            continue
        we = score_from_elo(engine_elo - opp_elo)
        fisher_final += n * we * (1.0 - we) * k * k

    if fisher_final > 0:
        se = 1.0 / math.sqrt(fisher_final)   # standard error in Elo
        sigma_95 = 1.96 * se
    else:
        sigma_95 = 9999.0

    return engine_elo, sigma_95


# ─────────────────────────────────────────────────────────────────────────────
# Stockfish downloader
# ─────────────────────────────────────────────────────────────────────────────

def _best_sf_asset(assets):
    """Choose the best Stockfish asset for the current platform."""
    system = platform.system()
    machine = platform.machine().lower()

    if system == "Windows":
        prefs = ["windows-x86-64-avx2", "windows-x86-64-bmi2",
                 "windows-x86-64-modern", "windows-x86-64"]
        ext = ".zip"
    elif system == "Darwin":
        prefs = (["macos-apple-silicon", "macos-arm64"]
                 if ("arm" in machine or "m1" in machine or "m2" in machine)
                 else ["macos-x86-64-avx2", "macos-x86-64-modern", "macos-x86-64"])
        ext = ".tar"
    else:
        prefs = ["ubuntu-x86-64-avx2", "ubuntu-x86-64-bmi2",
                 "ubuntu-x86-64-modern", "ubuntu-x86-64-sse4-2",
                 "ubuntu-x86-64", "linux-x86-64"]
        ext = ".tar"

    for pref in prefs:
        for a in assets:
            name = a["name"].lower()
            if pref in name and name.endswith(ext):
                return a["browser_download_url"], a["name"]

    # Fallback: first archive that looks 64-bit
    for a in assets:
        name = a["name"].lower()
        if name.endswith(ext) and ("x86-64" in name or "arm64" in name):
            return a["browser_download_url"], a["name"]

    return None, None


def _extract_sf_binary(archive_path, dest_path, is_zip):
    """Extract the Stockfish binary from a zip or tar archive."""
    if is_zip:
        with zipfile.ZipFile(archive_path) as zf:
            candidates = [m for m in zf.namelist()
                          if Path(m).name.lower().startswith("stockfish")
                          and m.endswith(".exe")]
            if not candidates:
                return False
            candidates.sort(key=lambda x: x.count("/"))
            # Read bytes directly — avoids recreating subfolder structure
            dest_path.write_bytes(zf.read(candidates[0]))
    else:
        with tarfile.open(archive_path) as tf:
            candidates = [m for m in tf.getmembers()
                          if m.isfile()
                          and Path(m.name).name.lower().startswith("stockfish")]
            if not candidates:
                return False
            candidates.sort(key=lambda x: x.name.count("/"))
            f = tf.extractfile(candidates[0])
            if f is None:
                return False
            dest_path.write_bytes(f.read())
    return True


def download_stockfish(cache_dir):
    """Download the latest Stockfish release from GitHub. Returns exe path or None."""
    cache_dir.mkdir(parents=True, exist_ok=True)
    exe_name = "stockfish.exe" if platform.system() == "Windows" else "stockfish"
    cached = cache_dir / exe_name

    # is_file() rejects empty folders left by a failed prior extraction
    if cached.is_file() and cached.stat().st_size > 100_000:
        print(f"  Using cached Stockfish: {cached}")
        return str(cached)
    elif cached.exists():
        print(f"  Removing invalid cached entry: {cached}")
        import shutil as _shutil
        if cached.is_dir():
            _shutil.rmtree(cached)
        else:
            cached.unlink()

    print("  Fetching latest Stockfish release from GitHub...")
    api = "https://api.github.com/repos/official-stockfish/Stockfish/releases/latest"
    try:
        req = urllib.request.Request(api, headers={"User-Agent": "elo-estimator/1.0"})
        with urllib.request.urlopen(req, timeout=15) as r:
            release = json.loads(r.read().decode())
    except Exception as e:
        print(f"  GitHub API error: {e}")
        return None

    url, filename = _best_sf_asset(release.get("assets", []))
    if not url:
        print("  No suitable Stockfish asset found for this platform.")
        return None

    archive = cache_dir / filename
    print(f"  Downloading {filename}...")
    try:
        urllib.request.urlretrieve(url, archive)
    except Exception as e:
        print(f"  Download failed: {e}")
        return None

    print("  Extracting...")
    ok = _extract_sf_binary(archive, cached, filename.endswith(".zip"))
    archive.unlink(missing_ok=True)

    if not ok or not cached.exists():
        print("  Could not find Stockfish binary inside archive.")
        return None

    if platform.system() != "Windows":
        cached.chmod(0o755)

    print(f"  Stockfish ready: {cached}")
    return str(cached)


# ─────────────────────────────────────────────────────────────────────────────
# Match runner
# ─────────────────────────────────────────────────────────────────────────────

def run_match(engine_exe, sf_exe, sf_elo, games, tc, pgn_path, concurrency):
    """
    Play `games` games between engine_exe and Stockfish at sf_elo.
    Returns (wins, losses, draws) from the engine's perspective, or None.
    """
    rounds = max(1, games // 2)

    # Wrap paths containing spaces for fastchess cmd= / file= values
    def fc_path(p):
        return f'"{p}"' if " " in str(p) else str(p)

    engine_args = [
        "-engine", f"cmd={fc_path(engine_exe)}", "name=Engine",
        "option.Threads=1",
    ]
    sf_args = [
        "-engine", f"cmd={fc_path(sf_exe)}", f"name=SF_{sf_elo}",
        "option.UCI_LimitStrength=true",
        f"option.UCI_Elo={sf_elo}",
        "option.Threads=1",
    ]
    match_args = [
        "-each", f"tc={tc}",
        "-rounds", str(rounds), "-games", "2", "-repeat",
        "-concurrency", str(concurrency),
    ]
    adjudication_args = [
        # Saves significant time: skip dead draws and hopeless positions
        "-draw",   "movenumber=40", "movecount=8",  "score=10",
        "-resign", "movecount=6",   "score=700",
    ]
    opening_args = (
        ["-openings", f"file={fc_path(pgn_path)}", "format=pgn", "order=random"]
        if pgn_path and os.path.isfile(pgn_path)
        else []
    )

    cmd = (["fastchess"]
           + engine_args + sf_args
           + match_args + adjudication_args + opening_args)

    # check=False: fastchess exits non-zero on interrupted tournaments but
    # may still have valid partial results we can parse.
    proc = subprocess.run(cmd, capture_output=True, text=True)
    output = proc.stdout + proc.stderr

    if not output.strip():
        print(f"  fastchess produced no output (exit {proc.returncode}).")
        return None

    if "Cannot execute command" in output:
        bad = re.search(r"Cannot execute command:\s*(.+)", output)
        bad_path = bad.group(1).strip() if bad else "unknown"
        print(f"  ERROR: fastchess cannot launch: {bad_path}")
        print(f"  Possible fixes:")
        print(f"    1. Unblock in PowerShell: Unblock-File \"{bad_path}\"")
        print(f"    2. Install VC++ runtime: https://aka.ms/vs/17/release/vc_redist.x64.exe")
        print(f"    3. Use --sf to point to a local Stockfish binary.")
        return None

    # Grab the LAST W/L/D line (most complete result block after all games finish)
    wld_matches = list(re.finditer(
        r"Wins:\s*(\d+),\s*Losses:\s*(\d+),\s*Draws:\s*(\d+)", output
    ))
    if not wld_matches:
        print(f"  Could not parse W/L/D for SF_{sf_elo}. Output tail:")
        print(output[-600:])
        return None

    m = wld_matches[-1]
    w, l, d = int(m.group(1)), int(m.group(2)), int(m.group(3))
    n = w + l + d
    pct = (w + d * 0.5) / n * 100 if n else 0.0
    diff, lo, hi = compute_stats(w, l, d)
    print(
        f"    W/L/D {w}/{l}/{d}  "
        f"score {pct:4.0f}%  "
        f"diff {diff:+5.0f} Elo  "
        f"95% CI [{lo:+.0f}, {hi:+.0f}]"
    )
    return w, l, d


# ─────────────────────────────────────────────────────────────────────────────
# Phase 1 — Adaptive binary search
# ─────────────────────────────────────────────────────────────────────────────

def bracket_search(engine_exe, sf_exe, tc, pgn_path, concurrency, probe_games=20):
    """
    Fast coarse bracket using short probe matches.
    Returns the Stockfish Elo level closest to the engine's strength.

    probe_games=20 is the minimum for noise-tolerant probes.
    With 10 games the Wilson CI is ~±250 Elo, making binary search unreliable.
    With 20 games it narrows to ~±180 Elo, enough for reliable direction.
    """
    print(f"\n{'─'*60}")
    print(f"  Phase 1 — Bracket search ({probe_games} games per probe)")
    print(f"{'─'*60}")

    lo, hi = SF_ELO_MIN, SF_ELO_MAX

    while hi - lo > 150:
        mid = round(((lo + hi) / 2) / 50) * 50
        mid = max(SF_ELO_MIN, min(SF_ELO_MAX, mid))
        print(f"\n  Probe at SF_{mid} (bracket [{lo}, {hi}]):")
        wld = run_match(engine_exe, sf_exe, mid, probe_games, tc, pgn_path, concurrency)
        if wld is None:
            return (lo + hi) // 2  # fallback if match fails
        diff, _, _ = compute_stats(*wld)
        if diff > 100:
            lo = mid
        elif diff < -100:
            hi = mid
        else:
            break

    result = round(((lo + hi) / 2) / 50) * 50
    result = max(SF_ELO_MIN, min(SF_ELO_MAX, result))
    print(f"\n  Bracket result: engine is roughly SF_{result} Elo")
    return result


# ─────────────────────────────────────────────────────────────────────────────
# Phase 2 — Gauntlet + Bayesian MLE
# ─────────────────────────────────────────────────────────────────────────────

def gauntlet(engine_exe, sf_exe, centre_elo, games_per_opp, tc, pgn_path, concurrency):
    """
    Play against 5 Stockfish Elo levels centred on centre_elo, spaced 200 apart.

    Spacing of 200 ensures each opponent gives independent information:
    two opponents 150 apart have heavily correlated expected scores (~80%
    overlap on the logistic curve), while 200 apart gives more separation
    and a better-conditioned MLE.

    Returns (bayesian_elo, sigma_95, raw_results).
    """
    spacing = 200
    points = sorted(set(
        max(SF_ELO_MIN, min(SF_ELO_MAX, round((centre_elo + (i - 2) * spacing) / 50) * 50))
        for i in range(5)
    ))

    total = len(points) * games_per_opp
    print(f"\n{'─'*60}")
    print(f"  Phase 2 — Gauntlet: {len(points)} opponents x {games_per_opp} games = {total} total")
    print(f"  Opponents: {['SF_'+str(p) for p in points]}")
    print(f"  Concurrency: {concurrency} parallel game pairs")
    print(f"{'─'*60}")

    raw_results = []
    for sf_elo in points:
        print(f"\n  vs SF_{sf_elo} (CCRL 40/4 ~ {sf_elo}):")
        wld = run_match(engine_exe, sf_exe, sf_elo, games_per_opp, tc, pgn_path, concurrency)
        if wld is not None:
            raw_results.append((sf_elo, *wld))

    if not raw_results:
        return None, None, []

    est, sigma = bayesian_mle(raw_results)
    return est, sigma, raw_results


# ─────────────────────────────────────────────────────────────────────────────
# Output
# ─────────────────────────────────────────────────────────────────────────────

def print_results(raw_results, engine_elo, sigma_95, tc):
    total_w = sum(w for _, w, l, d in raw_results)
    total_l = sum(l for _, w, l, d in raw_results)
    total_d = sum(d for _, w, l, d in raw_results)
    total_n = total_w + total_l + total_d

    print(f"\n{'='*64}")
    print(f"  FINAL RESULTS")
    print(f"{'='*64}")
    print(f"  {'Opponent':>12}  {'W':>5}  {'L':>5}  {'D':>5}  {'Score%':>7}  {'diff±CI':>12}")
    print(f"  {'─'*58}")
    for (opp_elo, w, l, d) in raw_results:
        n = w + l + d
        pct = (w + d * 0.5) / n * 100 if n else 0.0
        diff, lo, hi = compute_stats(w, l, d)
        half = (hi - lo) / 2
        print(f"  {'SF_'+str(opp_elo):>12}  {w:>5}  {l:>5}  {d:>5}  "
              f"{pct:>6.1f}%  {diff:>+6.0f}+/-{half:<.0f}")

    overall_pct = (total_w + total_d * 0.5) / total_n * 100 if total_n else 0.0
    print(f"  {'─'*58}")
    print(f"  {'TOTAL':>12}  {total_w:>5}  {total_l:>5}  {total_d:>5}  {overall_pct:>6.1f}%")

    lo95 = engine_elo - sigma_95
    hi95 = engine_elo + sigma_95

    print(f"\n  Estimated Elo:  {engine_elo:.0f}")
    print(f"  95% CI:         [{lo95:.0f}, {hi95:.0f}]  (+/-{sigma_95:.0f})")
    print(f"  Total games:    {total_n}")
    print(f"\n  Scale: CCRL 40/4-equivalent")
    print(f"         (Stockfish UCI_LimitStrength, officially anchored to CCRL 40/4)")
    print(f"         Time control used: {tc}")

    # Correct games-needed formula:
    # CI half-width scales as 1/sqrt(N), so to reach target H from current sigma_95:
    #   needed = total_n * (sigma_95 / H_target)^2
    TARGET_CI = 30
    if sigma_95 > TARGET_CI:
        needed = int(math.ceil(total_n * (sigma_95 / TARGET_CI) ** 2))
        # Round up to nearest multiple of 5 opponents * 2 (even games)
        print(f"\n  Tip: to reach +/-{TARGET_CI} Elo accuracy, run with "
              f"--games {max(20, (needed // len(raw_results) // 2 + 1) * 2)}")

    print(f"{'='*64}\n")


# ─────────────────────────────────────────────────────────────────────────────
# Entry point
# ─────────────────────────────────────────────────────────────────────────────

def main():
    ap = argparse.ArgumentParser(
        description="Universal Chess Engine Elo Estimator (CCRL 40/4 scale)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    exe_ext = ".exe" if platform.system() == "Windows" else ""
    script_dir = Path(__file__).parent
    default_engine = script_dir.parent / "bin" / "Release" / f"Uci{exe_ext}"

    ap.add_argument("--engine", default=str(default_engine),
                    help=f"Path to your UCI engine (default: {default_engine})")
    ap.add_argument("--games",  type=int, default=100,
                    help="Games per opponent in the gauntlet (default 100, min 20). "
                         "100 gives ~+/-35 Elo, 300 gives ~+/-20 Elo.")
    ap.add_argument("--tc",     default="10+0.1",
                    help="Time control e.g. '10+0.1' or '60+0.6' (default 10+0.1). "
                         "60+0.6 is closer to Stockfish's official calibration TC.")
    ap.add_argument("--sf",     default=None,
                    help="Path to Stockfish (auto-downloaded if omitted)")
    ap.add_argument("--openings", default=None,
                    help="Openings PGN file (8move_v3.pgn recommended)")
    ap.add_argument("--probe-only", action="store_true",
                    help="Only run bracket search, skip full gauntlet")
    ap.add_argument("--concurrency", type=int, default=None,
                    help="Parallel game pairs (default: physical_cores // 2). "
                         "Higher values speed up testing but hurt time control accuracy.")
    args = ap.parse_args()

    # ── Sanity checks ──────────────────────────────────────────────────────────

    if not shutil.which("fastchess"):
        sys.exit("ERROR: fastchess not found in PATH.\n"
                 "       Download from github.com/Disservin/fastchess and add to PATH.")

    # Auto-build when using the default engine path.
    using_default_engine = args.engine == str(default_engine)
    if using_default_engine:
        ext = ".bat" if platform.system() == "Windows" else ".sh"
        build_script = script_dir / f"build_engine_tournament{ext}"
        if build_script.is_file():
            print("Building tournament engine...")
            result = subprocess.run(
                [str(build_script)], capture_output=True, text=True
            )
            if result.returncode != 0:
                sys.exit(f"ERROR: Build failed.\n{result.stderr.strip()}")
            built_exe = result.stdout.strip()
            if built_exe and os.path.isfile(built_exe):
                args.engine = built_exe
                print(f"  Built: {built_exe}")

    engine_exe = os.path.abspath(args.engine)
    if not os.path.isfile(engine_exe):
        sys.exit(f"ERROR: Engine not found: {engine_exe}\n"
                 f"       Compile it first (Release mode), or pass --engine /path/to/exe")
    if platform.system() != "Windows" and not os.access(engine_exe, os.X_OK):
        os.chmod(engine_exe, 0o755)

    # ── Concurrency ────────────────────────────────────────────────────────────

    if args.concurrency:
        concurrency = max(1, args.concurrency)
    else:
        concurrency = _safe_concurrency()
        print(f"Auto-detected concurrency: {concurrency} "
              f"(physical cores: {_physical_cores()})")

    # ── Stockfish ──────────────────────────────────────────────────────────────

    if args.sf:
        sf_exe = os.path.abspath(args.sf)
        if not os.path.isfile(sf_exe):
            sys.exit(f"ERROR: Stockfish not found: {sf_exe}")
    else:
        print("Setting up Stockfish reference engine...")
        sf_exe = download_stockfish(CACHE_DIR)
        if not sf_exe:
            sys.exit("ERROR: Could not obtain Stockfish. Use --sf /path/to/stockfish")

    # ── Verify Stockfish runs ──────────────────────────────────────────────────

    print("  Verifying Stockfish executable...")
    try:
        uci_input = "uci\nquit\n"
        test = subprocess.run(
            [sf_exe], input=uci_input, capture_output=True, text=True, timeout=10
        )
        if "uciok" not in test.stdout:
            raise RuntimeError("no 'uciok' in response")
        print("  Stockfish OK.")
    except Exception as e:
        print(f"\n  ERROR: Stockfish failed to run: {e}")
        print(f"  Fixes:")
        print(f"    1. Unblock in PowerShell: Unblock-File \"{sf_exe}\"")
        print(f"    2. Install VC++ runtime: https://aka.ms/vs/17/release/vc_redist.x64.exe")
        print(f"    3. Provide a local build: --sf C:/path/to/stockfish.exe")
        sys.exit(1)

    # ── Openings ───────────────────────────────────────────────────────────────

    pgn_path = args.openings
    if not pgn_path:
        for candidate in [
            script_dir / "8move_v3.pgn",
            script_dir / "openings.pgn",
            script_dir.parent / "data" / "pgn" / "8move_v3.pgn",
        ]:
            if candidate.is_file():
                pgn_path = str(candidate)
                print(f"Using openings: {pgn_path}")
                break
        if not pgn_path:
            print("No openings PGN found — playing from start position only.")
            print("For better accuracy, provide --openings 8move_v3.pgn")

    games = max(20, (args.games // 2) * 2)

    # ── Header ─────────────────────────────────────────────────────────────────

    print(f"\n{'='*64}")
    print(f"  Engine:      {engine_exe}")
    print(f"  Reference:   Stockfish UCI_LimitStrength [CCRL 40/4 scale]")
    print(f"  TC:          {args.tc}   Games/opponent: {games}")
    print(f"  Concurrency: {concurrency} parallel game pairs")
    if args.tc == "10+0.1":
        print(f"  Note: use --tc 60+0.6 for results closer to official CCRL 40/4 scale")
    print(f"{'='*64}")

    # ── Phase 1 — Bracket ──────────────────────────────────────────────────────

    centre = bracket_search(engine_exe, sf_exe, args.tc, pgn_path,
                            concurrency, probe_games=20)
    print(f"\n  -> Coarse estimate: ~{centre} Elo (CCRL 40/4)")

    if args.probe_only:
        print(f"\n  [probe-only] Done. Estimated Elo: ~{centre}")
        return

    # ── Phase 2 — Gauntlet ─────────────────────────────────────────────────────

    est, sigma, raw = gauntlet(engine_exe, sf_exe, centre, games,
                               args.tc, pgn_path, concurrency)
    if est is None:
        sys.exit("ERROR: Gauntlet produced no results.")

    print_results(raw, est, sigma, args.tc)


if __name__ == "__main__":
    main()