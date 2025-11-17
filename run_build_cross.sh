#!/bin/bash
# ========================================
# ARM64 交叉编译构建脚本
# 用法:
#   ./build_cross.sh [Debug|Release]
# ========================================

set -e  # 出现错误就停止执行

# -----------------------
# 构建类型（默认 Release）
# -----------------------
BUILD_TYPE=${1:-Release}

# 构建目录
BUILD_DIR=build_cross

# 工具链文件
TOOLCHAIN_FILE=toolchain.cmake

echo "=== Build type: ${BUILD_TYPE} ==="

# -----------------------
# 清理旧的构建目录
# -----------------------
if [ -d "$BUILD_DIR" ]; then
    echo "=== Cleaning old build directory ==="
    rm -rf "$BUILD_DIR"
fi

# -----------------------
# 创建新的构建目录
# -----------------------
echo "=== Creating build directory ==="
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# -----------------------
# 调用 CMake 配置项目
# 使用交叉编译工具链
# -----------------------
echo "=== Configuring with CMake ==="
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=../$TOOLCHAIN_FILE \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# -----------------------
# 编译项目
# -----------------------
echo "=== Building project ==="
cmake --build . -- -j"$(nproc)"

# -----------------------
# 输出编译结果路径
# -----------------------
OUTPUT_DIR=$(realpath ../build_output/bin)
echo "=== Build finished ==="
echo "Output directory: ${OUTPUT_DIR}"
