#!/bin/bash
# ========================================
# Simple CMake build script
# Usage:
#   ./build.sh [Debug|Release]
# ========================================

set -e  # Exit on error

BUILD_TYPE=${1:-Release}
BUILD_DIR=build

echo "=== Clean old build ==="
rm -rf "$BUILD_DIR"

echo "=== Create build directory ==="
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "=== Configure CMake (${BUILD_TYPE}) ==="
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo "=== Build project ==="
cmake --build .

echo "=== Build finished ==="

