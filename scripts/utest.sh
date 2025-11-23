#!/bin/bash

# Este script ejecuta los tests unitarios para las builds utcommon, utaos, y utsoa.

set -e

export LD_LIBRARY_PATH="/opt/gcc-14/lib64${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

BASE_PATH="out/build/clang-tidy"
BUILD_TYPE="Debug"
echo "--- Ejecutando tests para la build: ${BUILD_TYPE} ---"
echo

# Tests Common
echo "Ejecutando tests Common (utest-common)..."
./${BASE_PATH}/utcommon/${BUILD_TYPE}/utcommon
echo

# Tests AOS
echo "Ejecutando tests AOS (utest-aos)..."
./${BASE_PATH}/utaos/${BUILD_TYPE}/utest-aos
echo

# Tests SOA
echo "Ejecutando tests SOA (utest-soa)..."
./${BASE_PATH}/utsoa/${BUILD_TYPE}/utest-soa
echo
echo "Todos los tests se completaron con éxito"
