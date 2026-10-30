#!/usr/bin/env bash
# Interactive Demonstration Runner for Hospital Management System
# Author: Pratham P Shetty (PES2UG24CS370)
set -euo pipefail

echo "=========================================================="
echo " Hospital Management System - Interactive Demo Runner"
echo " Prepared by: Team 1 (PES University SE Mini Project)"
echo "=========================================================="

BUILD_BIN="build/hms"

if [ ! -f "${BUILD_BIN}" ]; then
    echo "==> Binary not found. Building project first..."
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
fi

export HMS_ADMIN_PASSWORD="${HMS_ADMIN_PASSWORD:-AdminStrong123!}"

echo "==> Launching Hospital Management System CLI..."
echo "==> Default credentials for demonstration:"
echo "    Username: admin"
echo "    Password: ${HMS_ADMIN_PASSWORD}"
echo "=========================================================="

exec "${BUILD_BIN}"
