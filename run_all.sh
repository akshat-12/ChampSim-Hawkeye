#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

cd "$ROOT_DIR"

./q1.sh
python3 plot_llc_miss_rate.py --results-dir Q1 --output Q1/llc_miss_rate.png

./q2.sh
python3 plot_llc_miss_rate_q2.py --results-dir Q2 --output Q2/llc_miss_rate.png