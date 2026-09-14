#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
CONFIG_FILE="$ROOT_DIR/hawkeye_config.json"
TRACE_FILE="$ROOT_DIR/traces/456.hmmer-191B.champsimtrace.xz"
RESULTS_DIR="$ROOT_DIR/Q1"

# Add or remove sweep values here.
SETS=(8192 4096 2048)
WAYS=(4 8 16)
REPLACEMENTS=(lru)

if [[ ${#SETS[@]} -ne ${#WAYS[@]} ]]; then
    printf 'SETS and WAYS must contain the same number of entries\n' >&2
    exit 1
fi

cd "$ROOT_DIR"
mkdir -p "$RESULTS_DIR"

for replacement in "${REPLACEMENTS[@]}"; do
    for index in "${!SETS[@]}"; do
        sets=${SETS[$index]}
        ways=${WAYS[$index]}
        printf 'Running replacement=%s sets=%s ways=%s\n' "$replacement" "$sets" "$ways"

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
            "$TRACE_FILE" \
            > "$RESULTS_DIR/run_${sets}_${ways}_${replacement}" 2>&1
    done
done
