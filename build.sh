#!/bin/bash
# Build script for 3D Printer Locker with memory injection
# This script runs the double compilation process with memory info injection

echo "=== 3D Printer Locker Build Script ==="
echo "Step 1: Initial build..."
platformio run
if [ $? -ne 0 ]; then
    echo "❌ Initial build failed!"
    exit 1
fi

echo "✅ Initial build successful!"
echo ""
echo "Step 2: Injecting memory info..."
python3 scripts/inject_memory_info.py
if [ $? -ne 0 ]; then
    echo "❌ Memory injection failed!"
    exit 1
fi

echo "✅ Memory injection successful!"
echo ""
echo "Step 3: Rebuilding with injected memory values..."
platformio run
if [ $? -ne 0 ]; then
    echo "❌ Rebuild failed!"
    exit 1
fi

echo "✅ Rebuild successful!"
echo ""
echo "🎯 Build complete with accurate memory info!"
echo "Flash usage: $(grep 'BUILD_FLASH_USED' src/BuildMemoryInfo.h | cut -d' ' -f3) bytes"
echo "Flash free: $(grep 'BUILD_FLASH_FREE' src/BuildMemoryInfo.h | cut -d' ' -f3) bytes"