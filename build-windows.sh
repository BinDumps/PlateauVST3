#!/bin/bash
# Build the Plateau VST3 plugin for Windows via cross-compilation
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-win"
TOOLCHAIN="${SCRIPT_DIR}/cmake/x86_64-w64-mingw32-cross.cmake"

# Clean old build cache to avoid stale CMakeCache issues
rm -rf "$BUILD_DIR"

echo "=== Configuring for Windows cross-compilation ==="
cmake -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DCMAKE_BUILD_TYPE=Release \
    "$SCRIPT_DIR"

echo ""
echo "=== Building ==="
cmake --build "$BUILD_DIR" --config Release -j"$(nproc)"

echo ""
echo "=== Generating VST3 moduleinfo.json ==="
VST3_DIR="$BUILD_DIR/Plateau_artefacts/Release/VST3/Plateau.vst3"
mkdir -p "$VST3_DIR/Contents/Resources"
cat > "$VST3_DIR/Contents/Resources/moduleinfo.json" << 'JSONEOF'
{
    "Version": "1.0.0",
    "VST3ID": "com.valley.plateau",
    "VST3Categories": "Fx"
}
JSONEOF

echo ""
echo "=== Build complete ==="
echo "VST3 plugin: $VST3_DIR"
file "$VST3_DIR/Contents/x86_64-win/Plateau.vst3"
