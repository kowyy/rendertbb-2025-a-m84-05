#!/bin/bash
# rendimiento_aos.sh - Mide rendimiento de AOS
# Ejecuta las 4 imágenes de prueba con perf stat

set -Eeuo pipefail

# Configurar GCC-14
export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

BINARY="out/build/default/aos/Release/render-aos"
CONFIG_DIR="$(pwd)/tests"
OUTPUT_DIR="$(pwd)/out/rendimiento"

mkdir -p "$OUTPUT_DIR"

echo "========================================="
echo " EVALUACIÓN DE RENDIMIENTO AOS"
echo "========================================="
echo ""

# === CASO 1 ===
echo "Caso 1: config1.txt + scene1.txt (800x450)"
echo "Repeticiones: 15"
echo ""

perf stat -r 15 -a -e cycles,instructions,power/energy-pkg/,power/energy-ram/ \
    "$BINARY" "$CONFIG_DIR/config1.txt" "$CONFIG_DIR/scene1.txt" "$OUTPUT_DIR/aos_1.ppm" \
    2>&1 | tee "$OUTPUT_DIR/perf_aos_caso1.log"

echo ""
echo "---"
echo ""

# === CASO 2 ===
echo "Caso 2: config2.txt + scene2.txt (800x450)"
echo "Repeticiones: 10"
echo ""

perf stat -r 10 -a -e cycles,instructions,power/energy-pkg/,power/energy-ram/ \
    "$BINARY" "$CONFIG_DIR/config2.txt" "$CONFIG_DIR/scene2.txt" "$OUTPUT_DIR/aos_2.ppm" \
    2>&1 | tee "$OUTPUT_DIR/perf_aos_caso2.log"

echo ""
echo "---"
echo ""

# === CASO 3 ===
echo "Caso 3: config3.txt + scene3.txt (1280x720)"
echo "Repeticiones: 7"
echo ""

perf stat -r 7 -a -e cycles,instructions,power/energy-pkg/,power/energy-ram/ \
    "$BINARY" "$CONFIG_DIR/config3.txt" "$CONFIG_DIR/scene3.txt" "$OUTPUT_DIR/aos_3.ppm" \
    2>&1 | tee "$OUTPUT_DIR/perf_aos_caso3.log"

echo ""
echo "---"
echo ""

echo ""
echo "========================================="
echo " Evaluación completada"
echo " Resultados guardados en: $OUTPUT_DIR"
echo "========================================="
