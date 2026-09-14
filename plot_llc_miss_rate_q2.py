#!/usr/bin/env python3
"""Plot Hawkeye LLC miss-rate improvement over LRU by benchmark."""

import argparse
import re
from pathlib import Path

import matplotlib.pyplot as plt


LLC_TOTAL_PATTERN = re.compile(
    r"cpu0->LLC TOTAL\s+"
    r"ACCESS:\s*(?P<access>\d+)\s+"
    r"HIT:\s*(?P<hit>\d+)\s+"
    r"MISS:\s*(?P<miss>\d+)"
)
RESULT_PATTERN = re.compile(
    r"run_(?P<sets>\d+)_(?P<associativity>\d+)_(?P<policy>hawkeye|lru)_(?P<benchmark>.+)"
)


def read_llc_totals(path):
    """Return LLC access, hit, and miss counts from a simulation output file."""
    for line in path.read_text(encoding="utf-8").splitlines():
        match = LLC_TOTAL_PATTERN.search(line)
        if match:
            totals = {name: int(value) for name, value in match.groupdict().items()}
            if totals["access"] != totals["hit"] + totals["miss"]:
                raise ValueError(f"{path}: LLC TOTAL access does not equal hit + miss")
            return totals

    raise ValueError(f"{path}: could not find a cpu0->LLC TOTAL line")


def collect_results(results_dir):
    """Collect LRU and Hawkeye miss rates grouped by benchmark."""
    results = {}
    for path in results_dir.glob("run_*_*_*_*"):
        match = RESULT_PATTERN.fullmatch(path.name)
        if not match:
            continue

        benchmark = match.group("benchmark")
        policy = match.group("policy")
        totals = read_llc_totals(path)
        miss_rate = totals["miss"] / totals["access"]
        benchmark_results = results.setdefault(benchmark, {})
        if policy in benchmark_results:
            raise ValueError(f"Duplicate {policy} result for benchmark {benchmark}")
        benchmark_results[policy] = miss_rate

    if not results:
        raise ValueError(
            f"No matching run_<sets>_<associativity>_<policy>_<benchmark> files in {results_dir}"
        )

    missing = [benchmark for benchmark, policies in results.items() if set(policies) != {"hawkeye", "lru"}]
    if missing:
        raise ValueError(f"Missing LRU or Hawkeye result for benchmark(s): {missing}")

    improvements = {}
    for benchmark, rates in results.items():
        lru_rate = rates["lru"]
        if lru_rate == 0:
            raise ValueError(f"{benchmark}: LRU miss rate is zero")
        improvements[benchmark] = (lru_rate - rates["hawkeye"]) / lru_rate * 100

    return dict(sorted(improvements.items())), dict(sorted(results.items()))


def plot_results(improvements, miss_rates):
    """Create a bar chart of Hawkeye's miss-rate improvement over LRU."""
    figure, axis = plt.subplots(figsize=(8, 6))
    bars = axis.bar(list(improvements), list(improvements.values()), color="#2f6f9f")
    axis.bar_label(bars, fmt="%.2f%%", padding=3)
    axis.axhline(0, color="black", linewidth=0.8)
    axis.set_xlabel("Benchmark")
    axis.set_ylabel("Hawkeye improvement over LRU (%)")
    axis.set_title("LLC Miss-Rate Improvement of Hawkeye over LRU")
    axis.grid(axis="y", alpha=0.3)
    table_data = [
        [
            benchmark,
            f"{miss_rates[benchmark]['lru'] * 100:.2f}%",
            f"{miss_rates[benchmark]['hawkeye'] * 100:.2f}%",
        ]
        for benchmark in improvements
    ]
    table = axis.table(
        cellText=table_data,
        colLabels=["Benchmark", "LRU miss rate", "Hawkeye miss rate"],
        cellLoc="center",
        colWidths=[0.42, 0.29, 0.29],
        bbox=[0, -0.64, 1, 0.4],
    )
    for (row, column), cell in table.get_celld().items():
        cell.set_edgecolor("#d0d7de")
        cell.set_linewidth(0.7)
        cell.set_height(0.09)
        if row == 0:
            cell.set_facecolor("#2f6f9f")
            cell.get_text().set_color("white")
            cell.get_text().set_weight("bold")
        else:
            cell.set_facecolor("#f1f5f9" if row % 2 else "white")
    table.auto_set_font_size(False)
    table.set_fontsize(9)
    figure.subplots_adjust(bottom=0.44)
    return figure


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--results-dir",
        type=Path,
        default=Path(__file__).parent,
        help="Directory containing run_<sets>_<associativity>_<policy>_<benchmark> files",
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Save the plot to this path instead of displaying it",
    )
    args = parser.parse_args()

    improvements, miss_rates = collect_results(args.results_dir)
    print("Benchmark improvement (%):")
    for benchmark, improvement in improvements.items():
        print(f"{benchmark}: {improvement:.2f}%")

    figure = plot_results(improvements, miss_rates)
    if args.output:
        figure.savefig(args.output, dpi=200)
    else:
        plt.show()


if __name__ == "__main__":
    main()