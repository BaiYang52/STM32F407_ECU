#!/bin/bash

#############################################################################
# 项目清理脚本
# 用途: 清理编译产物和临时文件
# 使用: bash tools/clean.sh
#############################################################################

echo "清理编译产物..."

# 删除build目录
rm -rf build/

# 删除编译输出
find . -name "*.o" -delete
find . -name "*.a" -delete
find . -name "*.elf" -delete
find . -name "*.hex" -delete
find . -name "*.bin" -delete
find . -name "*.map" -delete

# 删除临时文件
find . -name "*~" -delete
find . -name "*.swp" -delete
find . -name ".DS_Store" -delete

# 删除CMake缓存
rm -f CMakeCache.txt
rm -rf CMakeFiles/

echo "✓ 清理完成"