#!/bin/bash

#############################################################################
# Project Build Script
# Purpose: Build STM32F407 AUTOSAR ECU Project
# Usage: bash tools/build.sh [Debug|Release]
#############################################################################

BUILD_TYPE=${1:-Debug}
BUILD_DIR="build"
NUM_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

echo "=========================================="
echo "  Build Configuration: $BUILD_TYPE"
echo "  Parallel Jobs: $NUM_JOBS"
echo "=========================================="
echo ""

# Create build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure CMake
echo "Configuring CMake..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed"
    exit 1
fi

# Build
echo ""
echo "Building..."
make -j$NUM_JOBS

if [ $? -ne 0 ]; then
    echo "❌ Build failed"
    exit 1
fi

# Generate hex and bin files
echo ""
echo "Generating firmware files..."
arm-none-eabi-objcopy -O ihex firmware_app.elf firmware_app.hex
arm-none-eabi-objcopy -O binary firmware_app.elf firmware_app.bin

# Display firmware size
echo ""
echo "Firmware Size:"
arm-none-eabi-size firmware_app.elf

echo ""
echo "✓ Build completed"
echo "  Output files: $(pwd)/firmware_app.{elf,hex,bin}"