#!/bin/bash
# ftest.sh - Pruebas funcionales del renderizador
# Ejecuta casos de éxito (comparación con imágenes de referencia) 
# y casos de error (archivos inválidos, argumentos incorrectos)

set -u

export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# === CONFIGURACIÓN ===

# Directorios y binarios
TEST_DIR="tests"
OUTPUT_DIR="out/build/test_outputs"
BUILD_PRESET="clang-tidy"
AOS_BIN="out/build/$BUILD_PRESET/aos/Release/render-aos"
SOA_BIN="out/build/$BUILD_PRESET/soa/Release/render-soa"
PAR_BIN="out/build/$BUILD_PRESET/par/Release/render-par"
COMPARE_SCRIPT="$(pwd)/scripts/compare_ppm.py"

# Contadores de pruebas
tests_passed=0
tests_failed=0

# === VERIFICACIÓN INICIAL ===

echo "INFO: Verificando binarios..."
if [ ! -f "$AOS_BIN" ]; then
    echo "Error: Ejecutable AOS no encontrado: $AOS_BIN"
    exit 1
fi

if [ ! -f "$SOA_BIN" ]; then
    echo "Error: Ejecutable SOA no encontrado: $SOA_BIN"
    exit 1
fi

if [ ! -f "$PAR_BIN" ]; then
    echo "Error: Ejecutable PAR no encontrado: $PAR_BIN"
    exit 1
fi

if [ ! -f "$COMPARE_SCRIPT" ]; then
    echo "Error: Script de comparación no encontrado: $COMPARE_SCRIPT"
    exit 1
fi

echo "INFO: Creando directorio de salida: $OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

# === FUNCIONES AUXILIARES ===

# Prueba un caso de éxito: ejecuta el renderizador y compara con referencia
check_success_case() {
    local test_name="$1"
    local bin="$2"
    local config="$3"
    local scene="$4"
    local output="$5"
    local ref="$6"

    echo ""
    echo "--- TEST (Éxito): $test_name ---"

    # Verificar que existe la imagen de referencia
    if [ ! -f "$ref" ]; then
        echo "¡FALLÓ! (Referencia no encontrada: '$ref')"
        ((tests_failed++))
        echo "---------------------------------"
        return
    fi

    # Ejecutar el renderizador
    "$bin" "$config" "$scene" "$output" > /dev/null 2>&1
    local exit_code=$?

    if [ $exit_code -ne 0 ]; then
        echo "¡FALLÓ! (El programa devolvió error: $exit_code)"
        ((tests_failed++))
        echo "---------------------------------"
        return
    fi

    # Comparar imagen generada con referencia
    "$COMPARE_SCRIPT" "$output" "$ref"
    local diff_code=$?

    if [ $diff_code -eq 0 ]; then
        ((tests_passed++))
    else
        ((tests_failed++))
    fi

    echo "---------------------------------"
}

# Prueba un caso de error: verifica que el programa falla como se espera
check_error_case() {
    local test_name="$1"
    local bin="$2"
    shift 2

    echo -n "TEST (Error): $test_name ... "

    # Ejecutar el programa y capturar código de salida
    "$bin" "$@" > /dev/null 2>&1
    local exit_code=$?

    if [ $exit_code -eq 0 ]; then
        echo "¡FALLÓ! (El programa no detectó el error)"
        ((tests_failed++))
    else
        echo "PASÓ (Código de error: $exit_code)"
        ((tests_passed++))
    fi
}

# === CASOS DE ÉXITO ===

echo ""
echo "========================================="
echo " PRUEBAS FUNCIONALES - CASOS DE ÉXITO"
echo "========================================="

echo ""
echo "INFO: ===== Ejecutando Casos de Éxito (AOS) ====="
check_success_case "AOS (Caso 1)" "$AOS_BIN" \
    "$TEST_DIR/config1.txt" "$TEST_DIR/scene1.txt" \
    "$OUTPUT_DIR/aos_out1.ppm" "$TEST_DIR/s1.ppm"

check_success_case "AOS (Caso 2)" "$AOS_BIN" \
    "$TEST_DIR/config2.txt" "$TEST_DIR/scene2.txt" \
    "$OUTPUT_DIR/aos_out2.ppm" "$TEST_DIR/s2.ppm"

check_success_case "AOS (Caso 3)" "$AOS_BIN" \
    "$TEST_DIR/config3.txt" "$TEST_DIR/scene3.txt" \
    "$OUTPUT_DIR/aos_out3.ppm" "$TEST_DIR/s3.ppm"

check_success_case "AOS (Caso 4)" "$AOS_BIN" \
    "$TEST_DIR/config4.txt" "$TEST_DIR/scene4.txt" \
    "$OUTPUT_DIR/aos_out4.ppm" "$TEST_DIR/s4.ppm"

echo ""
echo "INFO: ===== Ejecutando Casos de Éxito (SOA) ====="
check_success_case "SOA (Caso 1)" "$SOA_BIN" \
    "$TEST_DIR/config1.txt" "$TEST_DIR/scene1.txt" \
    "$OUTPUT_DIR/soa_out1.ppm" "$TEST_DIR/s1.ppm"

check_success_case "SOA (Caso 2)" "$SOA_BIN" \
    "$TEST_DIR/config2.txt" "$TEST_DIR/scene2.txt" \
    "$OUTPUT_DIR/soa_out2.ppm" "$TEST_DIR/s2.ppm"

check_success_case "SOA (Caso 3)" "$SOA_BIN" \
    "$TEST_DIR/config3.txt" "$TEST_DIR/scene3.txt" \
    "$OUTPUT_DIR/soa_out3.ppm" "$TEST_DIR/s3.ppm"

check_success_case "SOA (Caso 4)" "$SOA_BIN" \
    "$TEST_DIR/config4.txt" "$TEST_DIR/scene4.txt" \
    "$OUTPUT_DIR/soa_out4.ppm" "$TEST_DIR/s4.ppm"

echo ""
echo "INFO: ===== Ejecutando Casos de Éxito (PAR) ====="
check_success_case "PAR (Caso 1)" "$PAR_BIN" \
    "$TEST_DIR/config1.txt" "$TEST_DIR/scene1.txt" \
    "$OUTPUT_DIR/par_out1.ppm" "$TEST_DIR/s1.ppm"

check_success_case "PAR (Caso 2)" "$PAR_BIN" \
    "$TEST_DIR/config2.txt" "$TEST_DIR/scene2.txt" \
    "$OUTPUT_DIR/par_out2.ppm" "$TEST_DIR/s2.ppm"

check_success_case "PAR (Caso 3)" "$PAR_BIN" \
    "$TEST_DIR/config3.txt" "$TEST_DIR/scene3.txt" \
    "$OUTPUT_DIR/par_out3.ppm" "$TEST_DIR/s3.ppm"

check_success_case "PAR (Caso 4)" "$PAR_BIN" \
    "$TEST_DIR/config4.txt" "$TEST_DIR/scene4.txt" \
    "$OUTPUT_DIR/par_out4.ppm" "$TEST_DIR/s4.ppm"

# === CASOS DE ERROR ===

echo ""
echo "========================================="
echo " PRUEBAS FUNCIONALES - CASOS DE ERROR"
echo "========================================="

echo ""
echo "INFO: ===== Errores de Línea de Comandos (AOS) ====="
check_error_case "AOS (Argumentos insuficientes)" \
    "$AOS_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/scene1.txt"

check_error_case "AOS (Argumentos excesivos)" \
    "$AOS_BIN" "a" "b" "c" "d"

echo ""
echo "INFO: ===== Errores de Archivos No Encontrados (AOS) ====="
check_error_case "AOS (Config no existe)" \
    "$AOS_BIN" "$TEST_DIR/no_existe.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "AOS (Escena no existe)" \
    "$AOS_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/no_existe.txt" "$OUTPUT_DIR/error.ppm"

echo ""
echo "INFO: ===== Errores de Configuración Inválida (AOS) ====="
check_error_case "AOS (Gamma negativo)" \
    "$AOS_BIN" "$TEST_DIR/invalid_config_gamma.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "AOS (FOV >= 180)" \
    "$AOS_BIN" "$TEST_DIR/invalid_config_fov.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "AOS (Image width cero)" \
    "$AOS_BIN" "$TEST_DIR/invalid_config_width.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

echo ""
echo "INFO: ===== Errores de Escena Inválida (AOS) ====="
check_error_case "AOS (Material no existe)" \
    "$AOS_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/invalid_scene_material.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "AOS (Sintaxis incorrecta)" \
    "$AOS_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/invalid_scene_syntax.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "AOS (Radio negativo)" \
    "$AOS_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/invalid_scene_radius.txt" "$OUTPUT_DIR/error.ppm"

echo ""
echo "INFO: ===== Errores de Línea de Comandos (SOA) ====="
check_error_case "SOA (Argumentos insuficientes)" \
    "$SOA_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/scene1.txt"

check_error_case "SOA (Argumentos excesivos)" \
    "$SOA_BIN" "a" "b" "c" "d"

echo ""
echo "INFO: ===== Errores de Archivos No Encontrados (SOA) ====="
check_error_case "SOA (Config no existe)" \
    "$SOA_BIN" "$TEST_DIR/no_existe.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "SOA (Escena no existe)" \
    "$SOA_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/no_existe.txt" "$OUTPUT_DIR/error.ppm"

echo ""
echo "INFO: ===== Errores de Configuración Inválida (SOA) ====="
check_error_case "SOA (Gamma negativo)" \
    "$SOA_BIN" "$TEST_DIR/invalid_config_gamma.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "SOA (FOV >= 180)" \
    "$SOA_BIN" "$TEST_DIR/invalid_config_fov.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

echo ""
echo "INFO: ===== Errores de Escena Inválida (SOA) ====="
check_error_case "SOA (Material no existe)" \
    "$SOA_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/invalid_scene_material.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "SOA (Radio negativo)" \
    "$SOA_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/invalid_scene_radius.txt" "$OUTPUT_DIR/error.ppm"

echo ""
echo "INFO: ===== Errores de Línea de Comandos (PAR) ====="
check_error_case "PAR (Argumentos insuficientes)" \
    "$PAR_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/scene1.txt"

check_error_case "PAR (Argumentos excesivos)" \
    "$PAR_BIN" "a" "b" "c" "d"

echo ""
echo "INFO: ===== Errores de Archivos No Encontrados (PAR) ====="
check_error_case "PAR (Config no existe)" \
    "$PAR_BIN" "$TEST_DIR/no_existe.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "PAR (Escena no existe)" \
    "$PAR_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/no_existe.txt" "$OUTPUT_DIR/error.ppm"

echo ""
echo "INFO: ===== Errores de Configuración Inválida (PAR) ====="
check_error_case "PAR (Gamma negativo)" \
    "$PAR_BIN" "$TEST_DIR/invalid_config_gamma.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "PAR (FOV >= 180)" \
    "$PAR_BIN" "$TEST_DIR/invalid_config_fov.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "PAR (Image width cero)" \
    "$PAR_BIN" "$TEST_DIR/invalid_config_width.txt" "$TEST_DIR/scene1.txt" "$OUTPUT_DIR/error.ppm"

echo ""
echo "INFO: ===== Errores de Escena Inválida (PAR) ====="
check_error_case "PAR (Material no existe)" \
    "$PAR_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/invalid_scene_material.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "PAR (Sintaxis incorrecta)" \
    "$PAR_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/invalid_scene_syntax.txt" "$OUTPUT_DIR/error.ppm"

check_error_case "PAR (Radio negativo)" \
    "$PAR_BIN" "$TEST_DIR/config1.txt" "$TEST_DIR/invalid_scene_radius.txt" "$OUTPUT_DIR/error.ppm"

# === RESUMEN FINAL ===

echo ""
echo "========================================="
echo " RESUMEN DE PRUEBAS FUNCIONALES"
echo "========================================="
echo " Archivos de salida en: $OUTPUT_DIR"
echo ""
echo " Pruebas Pasadas: $tests_passed"
echo " Pruebas Fallidas: $tests_failed"
echo "========================================="

if [ $tests_failed -gt 0 ]; then
    exit 1
else
    exit 0
fi
