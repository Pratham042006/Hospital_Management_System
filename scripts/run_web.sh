#!/usr/bin/env bash
set -e

# Change directory to repo root
cd "$(dirname "$0")/.."

echo "=========================================================="
echo "      Hospital Management System - Web Dashboard v2.0     "
echo "=========================================================="

# Build if needed
if [ ! -f "build/hms" ]; then
    echo "[HMS] Building binaries..."
    cmake -S . -B build
    cmake --build build --config Release
fi

echo "[HMS] Starting Embedded C++ Web Server on http://localhost:8080..."
echo "[HMS] Opening browser at http://localhost:8080"
echo "=========================================================="

if command -v open >/dev/null 2>&1; then
    (sleep 1 && open "http://localhost:8080") &
elif command -v xdg-open >/dev/null 2>&1; then
    (sleep 1 && xdg-open "http://localhost:8080") &
fi

./build/hms --server 8080
