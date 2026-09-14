#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
CONFIG_FILE="$ROOT_DIR/hawkeye_config.json"
TRACE_FILE1="$ROOT_DIR/traces/456.hmmer-191B.champsimtrace.xz"
TRACE_FILE2="$ROOT_DIR/traces/429.mcf-22B.champsimtrace.xz"
TRACE_FILE3="$ROOT_DIR/traces/473.astar-42B.champsimtrace.xz"
RESULTS_DIR="$ROOT_DIR/Q2"

# Add or remove sweep values here.
REPLACEMENTS=(hawkeye lru)

cd "$ROOT_DIR"
mkdir -p "$RESULTS_DIR"

for replacement in "${REPLACEMENTS[@]}"; do
    for trace in "$TRACE_FILE1" "$TRACE_FILE2" "$TRACE_FILE3"; do
        trace_name=$(basename -- "$trace" .champsimtrace.xz)
        sets=2048
        ways=16
        printf 'Running replacement=%s sets=%s ways=%s trace=%s\n' \
            "$replacement" "$sets" "$ways" "$trace_name"

        python3 - "$CONFIG_FILE" "$sets" "$ways" "$replacement" <<'PY'
import json
import sys

config_path, sets, ways, replacement = sys.argv[1:]
with open(config_path, encoding="utf-8") as config_file:
    config = json.load(config_file)

config["LLC"]["sets"] = int(sets)
config["LLC"]["ways"] = int(ways)
config["LLC"]["replacement"] = replacement

with open(config_path, "w", encoding="utf-8") as config_file:
    json.dump(config, config_file, indent=4)
    config_file.write("\n")
PY

        ./config.sh "$CONFIG_FILE"
        make
        ./bin/champsim \
            --warmup_instructions 20000000 \
            --simulation_instructions 50000000 \
            "$trace" \
            > "$RESULTS_DIR/run_${sets}_${ways}_${replacement}_${trace_name}" 2>&1
    done
done
