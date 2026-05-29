#!/bin/bash

#############################################################################
# 项目构建脚本
# 用途: 编译STM32F407 AUTOSAR ECU项目
# 使用: bash tools/build.sh [Debug|Release]
#############################################################################

BUILD_TYPE=${1:-Debug}
BUILD_DIR="build"
NUM_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

echo "=========================================="
echo "  编译配置: $BUILD_TYPE"
echo "  并行任务数: $NUM_JOBS"
echo "=========================================="
echo ""

# 创建build目录
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# 配置CMake
echo "正在配置CMake..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

if [ $? -ne 0 ]; then
    echo "❌ CMake配置失败"
    exit 1
fi

# 编译
echo ""
echo "正在编译..."
make -j$NUM_JOBS

if [ $? -ne 0 ]; then
    echo "❌ 编译失败"
    exit 1
fi

# 生成hex和bin文件
echo ""
echo "正在生成固件文件..."
arm-none-eabi-objcopy -O ihex firmware_app.elf firmware_app.hex
arm-none-eabi-objcopy -O binary firmware_app.elf firmware_app.bin

# 显示固件大小
echo ""
echo "固件大小:"
arm-none-eabi-size firmware_app.elf

echo ""
echo "✓ 编译完成"
echo "  输出文件: $(pwd)/firmware_app.{elf,hex,bin}"