# =====================================================
# Cross-compilation toolchain file for ARM64 (aarch64)
# =====================================================

# 目标系统是 Linux
set(CMAKE_SYSTEM_NAME Linux)

# 目标架构为 aarch64（ARM 64位）
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# -----------------------------------------------------
# 指定交叉编译器路径 (需更改为实际路径)
# -----------------------------------------------------
# 注意：这里是 host 上的交叉编译器，生成的二进制是运行在 target 上的
set(CMAKE_C_COMPILER   /opt/linux/x86-arm/aarch64-v01c01-linux-gnu-gcc/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /opt/linux/x86-arm/aarch64-v01c01-linux-gnu-gcc/bin/aarch64-linux-gnu-g++)

# -----------------------------------------------------
# 目标环境根路径
# 用于查找 target 上的库和头文件
# -----------------------------------------------------
set(CMAKE_FIND_ROOT_PATH /opt/linux/x86-arm/aarch64-v01c01-linux-gnu-gcc)

# -----------------------------------------------------
# 查找模式
# -----------------------------------------------------
# 程序：始终使用 host 上的程序（如 cmake 或 cmake 内部调用的工具）
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# 库文件：只在 target 根路径下查找
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

# 头文件：只在 target 根路径下查找
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 包（Package）：只在 target 根路径下查找
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)