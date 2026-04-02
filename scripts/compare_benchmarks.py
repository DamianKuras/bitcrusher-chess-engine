"""
compare_benchmarks.py  base.json  dev.json  [base_label]  [dev_label]

Compares two Google Benchmark JSON output files and prints a table showing
the relative change for each benchmark, plus a geometric mean summary.

Only benchmarks present in both files are compared.
Aggregate entries other than 'mean' (e.g. stddev, median) are skipped.
"""

import json
import math
import sys


def load(path):
    with open(path, encoding="utf-8") as f:
        data = json.load(f)
    result = {}
    for bm in data.get("benchmarks", []):
        run_type = bm.get("run_type", "iteration")
        if run_type == "aggregate" and bm.get("aggregate_name") != "mean":
            continue
        result[bm["name"]] = bm["real_time"]
    return result


def fmt_time(ns):
    if ns >= 1e9:
        return f"{ns / 1e9:.3f} s "
    if ns >= 1e6:
        return f"{ns / 1e6:.3f} ms"
    if ns >= 1e3:
        return f"{ns / 1e3:.3f} us"
    return f"{ns:.3f} ns"


def verdict(pct):
    if pct < -5:
        return "FASTER"
    if pct < -1:
        return "faster"
    if pct > 5:
        return "SLOWER"
    if pct > 1:
        return "slower"
    return "~same"


def main():
    if len(sys.argv) < 3:
        print("Usage: compare_benchmarks.py base.json dev.json [base_label] [dev_label]")
        sys.exit(1)

    base_path, dev_path = sys.argv[1], sys.argv[2]
    base_label = sys.argv[3] if len(sys.argv) > 3 else "base"
    dev_label = sys.argv[4] if len(sys.argv) > 4 else "dev"

    base = load(base_path)
    dev = load(dev_path)

    common = sorted(set(base) & set(dev))
    only_base = sorted(set(base) - set(dev))
    only_dev = sorted(set(dev) - set(base))

    if not common:
        print("No matching benchmarks found in both files.")
        sys.exit(1)

    name_w = max(len(n) for n in common)
    name_w = max(name_w, 36)
    col_w = 13

    header = (
        f"{'Benchmark':<{name_w}}  "
        f"{base_label:>{col_w}}  "
        f"{dev_label:>{col_w}}  "
        f"{'Change':>9}  Verdict"
    )
    sep = "-" * len(header)

    print(f" Benchmark Comparison: {dev_label} (new) vs {base_label} (base)")
    print(sep)
    print(header)
    print(sep)

    ratios = []
    for name in common:
        b, d = base[name], dev[name]
        pct = (d - b) / b * 100
        ratios.append(d / b)
        print(
            f"{name:<{name_w}}  "
            f"{fmt_time(b):>{col_w}}  "
            f"{fmt_time(d):>{col_w}}  "
            f"{pct:+.1f}%  "
            f"{verdict(pct)}"
        )

    print(sep)
    geomean_ratio = math.exp(sum(math.log(r) for r in ratios) / len(ratios))
    geomean_pct = (geomean_ratio - 1) * 100
    print(
        f"{'Geometric mean':<{name_w}}  "
        f"{'':>{col_w}}  "
        f"{'':>{col_w}}  "
        f"{geomean_pct:+.1f}%"
    )

    if only_base:
        print()
        print(f"Benchmarks only in {base_label}: {', '.join(only_base)}")
    if only_dev:
        print(f"Benchmarks only in {dev_label}: {', '.join(only_dev)}")


if __name__ == "__main__":
    main()
