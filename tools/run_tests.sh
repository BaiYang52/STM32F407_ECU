#!/bin/bash

#############################################################################
# 单元测试运行脚本
# 用途: 编译并运行单元测试
# 使用: bash tools/run_tests.sh
#############################################################################

BUILD_DIR="build"
TEST_BUILD_DIR="${BUILD_DIR}/test"

echo "=========================================="
echo "  运行单元测试"
echo "=========================================="
echo ""

# 创建test build目录
mkdir -p $TEST_BUILD_DIR
cd $TEST_BUILD_DIR

# 配置CMake (启用测试)
echo "正在配置测试编译环境..."
cmake -DENABLE_TESTING=ON ../..

if [ $? -ne 0 ]; then
    echo "❌ CMake配置失败"
    exit 1
fi

# 编译
echo "正在编译测试..."
make -j$(nproc || echo 1)

if [ $? -ne 0 ]; then
    echo "❌ 编译测试失败"
    exit 1
fi

# 运行测试
echo ""
echo "正在运行测试..."
ctest --output-on-failure

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ 所有测试通过"
else
    echo ""
    echo "❌ 某些测试失败"
    exit 1
fi