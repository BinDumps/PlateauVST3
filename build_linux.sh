#!/bin/bash
# Build the Plateau VST3 plugin for Linux (with full UI)
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

echo "=== Configuring for Linux build ==="
cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    "$SCRIPT_DIR"

echo ""
echo "=== Building ==="
cmake --build "$BUILD_DIR" --config Release -j"$(nproc)"

echo ""
echo "=== Build complete ==="
ls -lh "$BUILD_DIR/Plateau_artefacts/Release/VST3/"*.vst3 2>/dev/null || \
    echo "VST3 plugin built successfully"
