#!/usr/bin/env bash
# Cross-platform build and test script for Unix (Linux and macOS)
# Author: Prathik Nambiar (PES2UG24CS371)
set -euo pipefail

echo "=============================================="
echo " Hospital Management System - Unix Build Script"
echo " Target: Linux / macOS (GCC or Clang)"
echo "=============================================="

BUILD_DIR="build"

echo "==> Configuring CMake..."
cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release

echo "==> Compiling source files..."
cmake --build "${BUILD_DIR}" --config Release -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)"

echo "==> Running automated test suite..."
ctest --test-dir "${BUILD_DIR}" -C Release --output-on-failure

echo "=============================================="
echo " Build and tests completed successfully!"
echo " Run executable: ./${BUILD_DIR}/hms"
echo "=============================================="
