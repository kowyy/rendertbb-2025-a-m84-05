#!/bin/bash

set -Eeuo pipefail

export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

BINARY="$(pwd)/out/build/default/aos/Release/render-aos"
CONFIG_DIR="$(pwd)/tests"
OUTPUT_DIR="$(pwd)/out/rendimiento"

mkdir -p "$OUTPUT_DIR"

# === CASO 4 ===
echo "=== Caso 4 AOS (1800x1012) - 5 repeticiones ==="
perf stat -r 5 -a -e cycles,instructions,power/energy-pkg/,power/energy-ram/ \
    "$BINARY" "$CONFIG_DIR/config4.txt" "$CONFIG_DIR/scene4.txt" "$OUTPUT_DIR/aos_4.ppm"

echo ""
echo " Caso 4 completado"
