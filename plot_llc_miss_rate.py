#!/usr/bin/env python3
"""Plot LLC miss rate for LRU and Hawkeye simulation results."""

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
RESULT_PATTERN = re.compile(r"run_(?P<sets>\d+)_(?P<associativity>\d+)_(?P<policy>hawkeye|lru)$")


def read_llc_totals(path):
    """Return LLC access, hit, and miss counts from a simulation output file."""
    for line in path.read_text().splitlines():
        match = LLC_TOTAL_PATTERN.search(line)
        if match:
            totals = {name: int(value) for name, value in match.groupdict().items()}
            if totals["access"] != totals["hit"] + totals["miss"]:
                raise ValueError(
                    f"{path}: LLC TOTAL access does not equal hit + miss"
                )
            return totals

    raise ValueError(f"{path}: could not find a cpu0->LLC TOTAL line")


def collect_results(results_dir):
    """Collect matching result files grouped by cache configuration."""
    results = {}
    for path in results_dir.glob("run_*_*_*"):
        match = RESULT_PATTERN.fullmatch(path.name)
        if not match:
            continue

        configuration = (
            int(match.group("sets")),
            int(match.group("associativity")),
        )
        policy = match.group("policy")
        totals = read_llc_totals(path)
        results.setdefault(configuration, {})[policy] = totals["miss"] / totals["access"]

    if not results:
        raise ValueError(f"No matching run_<sets>_<associativity>_<policy> files in {results_dir}")

    missing = [configuration for configuration, policies in results.items() if set(policies) != {"hawkeye", "lru"}]
    if missing:
        raise ValueError(f"Missing LRU or Hawkeye result for configuration(s): {missing}")

    return dict(sorted(results.items()))


def plot_results(results):
    configurations = list(results)
    labels = [f"{sets}, {associativity}" for sets, associativity in configurations]
    positions = range(len(configurations))

    figure, axis = plt.subplots(figsize=(8, 5))
    axis.plot(
        positions,
        [results[configuration]["lru"] for configuration in configurations],
        marker="o",
        linewidth=2,
        label="LRU",
    )
    axis.plot(
        positions,
        [results[configuration]["hawkeye"] for configuration in configurations],
        marker="o",
        linewidth=2,
        label="Hawkeye",
    )
    axis.set_xticks(list(positions), labels)
    axis.set_xlabel("Sets, Associativity")
    axis.set_ylabel("LLC miss rate")
    axis.set_title("LLC Miss Rate vs. Cache Configuration")
    axis.set_ylim(bottom=0)
    axis.grid(axis="y", alpha=0.3)
    axis.legend()
    figure.tight_layout()
    return figure


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--results-dir",
        type=Path,
        default=Path(__file__).parent,
        help="Directory containing run_<sets>_<associativity>_<policy> files",
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Save the plot to this path instead of displaying it",
    )
    args = parser.parse_args()

    figure = plot_results(collect_results(args.results_dir))
    if args.output:
        figure.savefig(args.output, dpi=200)
    else:
        plt.show()


if __name__ == "__main__":
    main()