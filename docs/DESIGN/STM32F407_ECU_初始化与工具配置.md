# STM32F407 AUTOSAR ECU 项目初始化与工具配置

## 1. 项目初始化脚本

### 1.1 项目自动初始化脚本 (tools/init_project.sh)

```bash
#!/bin/bash

#############################################################################
# STM32F407 AUTOSAR ECU 项目初始化脚本
# 用途: 自动创建标准的项目目录结构
# 使用: bash tools/init_project.sh
#############################################################################

set -e  # 遇到错误立即退出

PROJECT_NAME="STM32F407_ECU"
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# 颜色定义 (用于终端输出)
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'  # No Color

echo -e "${YELLOW}========================================${NC}"
echo -e "${YELLOW}  STM32F407 AUTOSAR ECU 项目初始化${NC}"
echo -e "${YELLOW}========================================${NC}"
echo ""

# ============= 创建核心目录结构 =============

echo -e "${YELLOW}[1/4] 创建核心目录结构...${NC}"

# 源文件目录
mkdir -p src/{mcal,bsw,rte,asw,app,fbl,config}

# 头文件目录
mkdir -p inc/{mcal,bsw,rte,asw,app,fbl,config}

# MCAL细分目录
mkdir -p src/mcal/{can,adc,timer,gpio,uart,spi,pwm,nvm,clock,power,startup}
mkdir -p inc/mcal/{can,adc,timer,gpio,uart,spi,pwm,nvm,clock,power}

# BSW细分目录
mkdir -p src/bsw/{com,canif,pdur,dcm,dem,cannm,nvm,nm}
mkdir -p inc/bsw/{com,canif,pdur,dcm,dem,cannm,nvm,nm}

# RTE目录
mkdir -p src/rte inc/rte

# ASW和App目录
mkdir -p src/asw inc/asw
mkdir -p src/app inc/app

# FBL目录
mkdir -p src/fbl inc/fbl

# 工程文件
mkdir -p cubeide_project matlab_simulink

# 测试目录
mkdir -p test/{unit,integration,mock,fixtures}

# 工具目录
mkdir -p tools/{scripts,python,cmakelists}

# 文档目录
mkdir -p docs/{SRS,API,DESIGN,TESTING,TOOLS}

# 构建目录
mkdir -p build

echo -e "${GREEN}✓ 核心目录创建完成${NC}"
echo ""

# ============= 创建基础源文件模板 =============

echo -e "${YELLOW}[2/4] 创建源文件模板...${NC}"

# 创建类型定义文件
cat > inc/types.h << 'EOF'
/**
 * @file types.h
 * @brief 标准类型定义
 * @version 1.0.0
 * @date $(date +%Y-%m-%d)
 */

#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============= Basic Types ============= */
typedef uint8_t     uint8;
typedef int8_t      int8;
typedef uint16_t    uint16;
typedef int16_t     int16;
typedef uint32_t    uint32;
typedef int32_t     int32;
typedef float       float32;
typedef double      float64;

typedef uint8 boolean;
#define TRUE  1U
#define FALSE 0U

/* ============= Return Types ============= */
typedef uint8 Std_ReturnType;
#define STD_OK     0U
#define STD_NOT_OK 1U

#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */
EOF

echo -e "${GREEN}✓ types.h 创建完成${NC}"

# 创建common.h
cat > inc/common.h << 'EOF'
/**
 * @file common.h
 * @brief 通用宏定义和函数
 * @version 1.0.0
 * @date $(date +%Y-%m-%d)
 */

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* ============= Memory Operations ============= */
#define MEMSET(dst, val, len)    memset((dst), (val), (len))
#define MEMCPY(dst, src, len)    memcpy((dst), (src), (len))
#define MEMCMP(dst, src, len)    memcmp((dst), (src), (len))

/* ============= Bit Operations ============= */
#define SET_BIT(val, bit)        ((val) |= (1U << (bit)))
#define CLR_BIT(val, bit)        ((val) &= ~(1U << (bit)))
#define TST_BIT(val, bit)        (((val) >> (bit)) & 1U)
#define TGL_BIT(val, bit)        ((val) ^= (1U << (bit)))

/* ============= Byte Operations ============= */
#define GET_LOW_BYTE(word)       ((uint8)((word) & 0xFFU))
#define GET_HIGH_BYTE(word)      ((uint8)(((word) >> 8) & 0xFFU))
#define MAKE_WORD(h, l)          (((uint16)(h) << 8) | ((uint16)(l) & 0xFFU))

/* ============= Min/Max ============= */
#define MIN(a, b)                (((a) < (b)) ? (a) : (b))
#define MAX(a, b)                (((a) > (b)) ? (a) : (b))

/* ============= Array Size ============= */
#define ARRAY_SIZE(arr)          (sizeof(arr) / sizeof((arr)[0]))

/* ============= Assert ============= */
#ifdef DEBUG
#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("Assertion failed at %s:%d\n", __FILE__, __LINE__); \
        } \
    } while(0)
#else
#define ASSERT(condition)
#endif

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H */
EOF

echo -e "${GREEN}✓ common.h 创建完成${NC}"

# 创建main.c模板
cat > src/app/main.c << 'EOF'
/**
 * @file main.c
 * @brief 应用主程序入口点
 * @version 1.0.0
 * @date $(date +%Y-%m-%d)
 */

#include "types.h"
#include "app_init.h"
#include "app_task_scheduler.h"
#include "bsw_dcm.h"

/**
 * @brief 主程序入口
 */
int main(void)
{
    /* 初始化系统 */
    App_System_Init();
    
    /* 初始化调度器 */
    App_TaskScheduler_Init();
    
    /* 主循环 */
    while (1) {
        /* 执行周期任务 */
        App_Task_10ms();
        App_Task_100ms();
        App_Task_1000ms();
    }
    
    return 0;
}
EOF

echo -e "${GREEN}✓ main.c 创建完成${NC}"

echo ""

# ============= 创建配置文件 =============

echo -e "${YELLOW}[3/4] 创建配置文件...${NC}"

# 创建CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.10)
project(STM32F407_ECU C ASM)

# Toolchain Configuration
set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_ASM_COMPILER "arm-none-eabi-as")

set(MCU "STM32F407VE")
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS "${CPU_FLAGS} -Wall -Wextra -Wpedantic -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O2 -g0")

# Include paths
include_directories(
    inc
    inc/mcal
    inc/bsw
    inc/rte
    inc/asw
    inc/app
    inc/fbl
    inc/config
)

# Source files
file(GLOB_RECURSE MCAL_SOURCES "src/mcal/*.c")
file(GLOB_RECURSE BSW_SOURCES "src/bsw/*.c")
file(GLOB_RECURSE RTE_SOURCES "src/rte/*.c")
file(GLOB_RECURSE ASW_SOURCES "src/asw/*.c")
file(GLOB_RECURSE APP_SOURCES "src/app/*.c")
file(GLOB_RECURSE CONFIG_SOURCES "src/config/*.c")

# Build target
add_executable(firmware_app.elf
    ${MCAL_SOURCES}
    ${BSW_SOURCES}
    ${RTE_SOURCES}
    ${ASW_SOURCES}
    ${APP_SOURCES}
    ${CONFIG_SOURCES}
)

# Post-build
add_custom_command(TARGET firmware_app.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex firmware_app.elf firmware_app.hex
    COMMAND arm-none-eabi-objcopy -O binary firmware_app.elf firmware_app.bin
)
EOF

echo -e "${GREEN}✓ CMakeLists.txt 创建完成${NC}"

# 创建.gitignore
cat > .gitignore << 'EOF'
# Build
build/
dist/
*.o
*.a
*.elf
*.hex
*.bin
*.map

# IDE
.vscode/
.settings/
*.project
*.pydevproject
*.cproject

# Temp
*~
*.swp
*.tmp

# Generated code
src/asw/app_models/

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake

# OS
.DS_Store
Thumbs.db
EOF

echo -e "${GREEN}✓ .gitignore 创建完成${NC}"

# 创建README.md
cat > README.md << 'EOF'
# STM32F407 AUTOSAR ECU 项目

基于STM32F407VET6微控制器的AUTOSAR电子控制单元(ECU)原型项目。

## 项目特性

- **AUTOSAR 分层架构**
  - MCAL (微控制器抽象层)
  - BSW (基础软件层)
  - RTE (运行时环境)
  - ASW (应用软件层)

- **核心功能**
  - CAN网络通信 (500kbps)
  - UDS诊断服务 (ISO 14229-1)
  - 车身网络管理 (AUTOSAR CanNm)
  - 故障检测与管理 (Dem/Dtc)
  - Flash Bootloader (FBL)

- **开发环境**
  - STM32CubeIDE (硬件配置)
  - MATLAB/Simulink (应用模型)
  - VSCode (代码编辑)
  - CMake (构建系统)

## 快速开始

### 1. 项目初始化
\`\`\`bash
bash tools/init_project.sh
\`\`\`

### 2. 编译
\`\`\`bash
mkdir -p build
cd build
cmake ..
make -j4
\`\`\`

### 3. 运行单元测试
\`\`\`bash
bash tools/run_tests.sh
\`\`\`

## 目录结构

\`\`\`
src/
├── mcal/          # 微控制器抽象层
├── bsw/           # 基础软件层
├── rte/           # 运行时环境
├── asw/           # 应用软件层
├── app/           # 主应用程序
├── fbl/           # Bootloader
└── config/        # 配置文件

test/
├── unit/          # 单元测试
├── integration/   # 集成测试
├── mock/          # Mock对象
└── fixtures/      # 测试数据
\`\`\`

## 文件命名规范

- **源文件**: \`{layer}_{module}_{feature}.c\`
  - 例: \`mcal_can_driver.c\`, \`bsw_dcm_service_0x22.c\`

- **头文件**: \`{layer}_{module}_{feature}.h\`
  - 例: \`can_driver.h\`, \`dcm_service.h\`

- **函数**: \`{Layer}_{Module}_{Operation}\`
  - 例: \`Mcal_Can_Init()\`, \`Bsw_Dcm_MainFunction()\`

- **测试**: \`test_{module}_{feature}.c\`
  - 例: \`test_can_driver_send.c\`

## 关键文档

- [项目文件结构设计](docs/DESIGN/Architecture.md)
- [API文档](docs/API/)
- [测试计划](docs/TESTING/)

## 编码规范

- **编程语言**: C (MISRA C遵从)
- **缩进**: 4个空格
- **命名**: 驼峰命名法 (函数/变量), 大写+下划线 (宏定义)
- **注释**: Doxygen格式

## 构建和调试

### 调试器连接
- ST-LINK V2 (SWD接口)
- 调试UART: USART1 (PA9/PA10, 115200bps)

### 调试命令
\`\`\`bash
# 烧写固件
arm-none-eabi-gdb firmware_app.elf
(gdb) target remote localhost:4242
(gdb) load
(gdb) continue

# 查看固件大小
arm-none-eabi-size firmware_app.elf
\`\`\`

## 许可证

MIT License

## 联系方式

- 项目维护者: [Rpcket]
- 邮箱: [ys.niu@outlook.com]

---

**最后更新**: $(date +%Y-%m-%d)
EOF

echo -e "${GREEN}✓ README.md 创建完成${NC}"

echo ""

# ============= 创建Git仓库 =============

echo -e "${YELLOW}[4/4] 初始化Git仓库...${NC}"

if [ ! -d .git ]; then
    git init
    git add .
    git commit -m "Initial commit: Project structure"
    echo -e "${GREEN}✓ Git仓库初始化完成${NC}"
else
    echo -e "${YELLOW}⚠ Git仓库已存在，跳过初始化${NC}"
fi

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  ✓ 项目初始化完成!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "后续步骤:"
echo "1. 在 cubeide_project/ 中导入 STM32CubeIDE 工程"
echo "2. 在 matlab_simulink/ 中配置 Simulink 模型"
echo "3. 编辑配置文件在 src/config/ 目录"
echo "4. 在 VSCode 中打开项目进行开发"
echo ""
echo "构建命令:"
echo "  mkdir -p build && cd build"
echo "  cmake .. && make -j4"
echo ""
echo "文档: 参考 docs/ 目录"
echo ""
```

### 1.2 项目清理脚本 (tools/clean.sh)

```bash
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
```

### 1.3 项目构建脚本 (tools/build.sh)

```bash
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
```

### 1.4 测试运行脚本 (tools/run_tests.sh)

```bash
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
```

---

## 2. VSCode 配置

### 2.1 settings.json

```json
{
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/inc",
        "${workspaceFolder}/inc/mcal",
        "${workspaceFolder}/inc/bsw",
        "${workspaceFolder}/inc/rte",
        "${workspaceFolder}/inc/asw",
        "${workspaceFolder}/inc/app",
        "${workspaceFolder}/inc/fbl",
        "${workspaceFolder}/inc/config",
        "${workspaceFolder}/cubeide_project/Core/Inc",
        "${workspaceFolder}/cubeide_project/Drivers/STM32F4xx_HAL_Driver/Inc"
    ],
    "C_Cpp.default.defines": [
        "STM32F407xx",
        "USE_HAL_DRIVER",
        "ARM_MATH_CM4",
        "DEBUG"
    ],
    "C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json",
    "C_Cpp.intelliSenseEngine": "default",
    "[c]": {
        "editor.defaultFormatter": "ms-vscode.cpptools",
        "editor.formatOnSave": true,
        "editor.tabSize": 4,
        "editor.insertSpaces": true
    },
    "[cmake]": {
        "editor.defaultFormatter": "cheshirekow.cmake-format"
    },
    "editor.formatOnPaste": true,
    "editor.wordWrapColumn": 100,
    "editor.rulers": [80, 100],
    "files.exclude": {
        "build/": true,
        "**/*.o": true,
        "**/.DS_Store": true
    },
    "cppcheck.cppcheckPath": "/usr/bin/cppcheck",
    "cppcheck.enable": true,
    "cppcheck.standard": ["c11"],
    "cppcheck.unusedFunction": true
}
```

### 2.2 launch.json (调试配置)

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "STM32F407 GDB Debug",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/firmware_app.elf",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": true,
            "MIMode": "gdb",
            "miDebuggerPath": "arm-none-eabi-gdb",
            "setupCommands": [
                {
                    "description": "启用详细输出",
                    "text": "set print pretty on",
                    "ignoreFailures": true
                },
                {
                    "description": "连接到ST-LINK",
                    "text": "target remote localhost:4242",
                    "ignoreFailures": false
                },
                {
                    "description": "加载固件",
                    "text": "load",
                    "ignoreFailures": false
                },
                {
                    "description": "设置断点在main",
                    "text": "break main",
                    "ignoreFailures": false
                }
            ],
            "preLaunchTask": "Build Debug"
        },
        {
            "name": "Unit Test Debug",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/test/test_can_driver",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/build/test",
            "environment": [],
            "externalConsole": true,
            "MIMode": "gdb",
            "miDebuggerPath": "gdb"
        }
    ]
}
```

### 2.3 tasks.json (构建任务)

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build Debug",
            "type": "shell",
            "command": "bash",
            "args": ["./tools/build.sh", "Debug"],
            "problemMatcher": ["$gcc"],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
        {
            "label": "Build Release",
            "type": "shell",
            "command": "bash",
            "args": ["./tools/build.sh", "Release"]
        },
        {
            "label": "Clean",
            "type": "shell",
            "command": "bash",
            "args": ["./tools/clean.sh"]
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "bash",
            "args": ["./tools/run_tests.sh"],
            "group": {
                "kind": "test",
                "isDefault": true
            }
        },
        {
            "label": "Format Code",
            "type": "shell",
            "command": "find",
            "args": [
                "src",
                "inc",
                "-type",
                "f",
                "-name",
                "*.c",
                "-o",
                "-name",
                "*.h"
            ]
        }
    ]
}
```

---

## 3. 代码格式化配置

### 3.1 .clang-format

```yaml
---
Language:        Cpp
BasedOnStyle:    LLVM

# 缩进
IndentWidth:     4
UseTab:          Never
TabWidth:        4
ColumnLimit:     100

# 括号和括号对齐
AlignAfterOpenBracket: Align
AlignConsecutiveAssignments: false
AlignConsecutiveDeclarations: false
AlignTrailingComments: true

# 断行
AllowShortBlocksOnASingleLine: false
AllowShortCaseLabelsOnASingleLine: false
AllowShortFunctionsOnASingleLine: None
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false

# 指针和引用对齐
PointerAlignment: Right
ReferenceAlignment: Pointer

# 函数定义格式
BinPackArguments: false
BinPackParameters: false

# 命名空间
NamespaceIndentation: None

# 注释
ReflowComments: true
MaxEmptyLinesToKeep: 2

# 其他
AccessModifierOffset: -4
BreakBeforeBraces: Allman
SortIncludes: true
SortUsingDeclarations: true
```

### 3.2 .editorconfig

```ini
# EditorConfig helps maintain consistent coding styles

root = true

# C files
[*.{c,h}]
charset = utf-8
end_of_line = lf
indent_style = space
indent_size = 4
insert_final_newline = true
trim_trailing_whitespace = true
max_line_length = 100

# CMake
[CMakeLists.txt]
indent_size = 2

# Shell scripts
[*.{sh,bash}]
indent_size = 2

# Markdown
[*.md]
trim_trailing_whitespace = false
```

---

## 4. 持续集成配置

### 4.1 GitHub Actions (.github/workflows/build.yml)

```yaml
name: Build and Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install ARM Toolchain
      run: |
        sudo apt-get update
        sudo apt-get install -y gcc-arm-none-eabi
    
    - name: Install CMake
      run: |
        sudo apt-get install -y cmake
    
    - name: Build Debug
      run: |
        bash tools/build.sh Debug
    
    - name: Build Release
      run: |
        bash tools/build.sh Release
    
    - name: Check Code Style
      run: |
        find src inc -name "*.c" -o -name "*.h" | \
        xargs clang-format --dry-run --Werror

  test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Install Dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y libcunit1 libcunit1-dev
    
    - name: Run Tests
      run: |
        bash tools/run_tests.sh
```

---

## 5. 项目使用指南

### 5.1 快速开始

```bash
# 1. 克隆项目
git clone <repository-url>
cd STM32F407_ECU_Project

# 2. 初始化项目
bash tools/init_project.sh

# 3. 编译Debug版本
bash tools/build.sh Debug

# 4. 运行测试
bash tools/run_tests.sh

# 5. 在VSCode中打开
code .
```

### 5.2 日常开发流程

```bash
# 修改代码后
bash tools/build.sh Debug

# 本地测试
bash tools/run_tests.sh

# 代码格式化
clang-format -i src/**/*.c inc/**/*.h

# 提交到git
git add .
git commit -m "功能说明"
git push
```

### 5.3 故障排查

**编译错误: undefined reference**
```bash
# 检查CMakeLists.txt中的源文件列表
# 确保所有.c文件都被包含
cmake --build build --target clean
rm -rf build
bash tools/build.sh Debug
```

**测试失败**
```bash
# 运行单个测试进行调试
cd build/test
ctest --verbose -R test_name

# 使用gdb调试
gdb ./test_can_driver
(gdb) run
```

---

## 6. 文档生成

### 6.1 Doxygen配置 (Doxyfile)

```
PROJECT_NAME           = "STM32F407 AUTOSAR ECU"
PROJECT_VERSION        = "1.0.0"
OUTPUT_DIRECTORY       = docs/doxygen
INPUT                  = src inc
RECURSIVE              = YES
FILE_PATTERNS          = *.c *.h
EXTRACT_ALL            = YES
GENERATE_HTML          = YES
GENERATE_LATEX         = YES
HTML_OUTPUT            = html
HAVE_DOT               = YES
DOT_PATH               = /usr/bin/dot
```

**生成文档**:
```bash
doxygen Doxyfile
# 输出在 docs/doxygen/html/index.html
```

---

## 7. 协作开发规范

### 7.1 Git分支策略

```
main          # 发布分支 (稳定版本)
├── release/* # 发布准备分支
└── develop   # 开发主分支
    ├── feature/xxx   # 功能分支
    ├── bugfix/xxx    # 修复分支
    └── refactor/xxx  # 重构分支
```

### 7.2 Commit消息规范

```
<type>(<scope>): <subject>
<blank line>
<body>
<blank line>
<footer>

type: feat(功能), fix(修复), docs(文档), style(格式), refactor(重构), test(测试)
scope: mcal, bsw, rte, asw, app, fbl
subject: 简述改动 (50字以内)
body: 详细说明 (可选)
footer: 关闭的issue编号 (可选)

示例:
feat(dcm): 添加0x22读DID服务支持

- 实现ReadDataByIdentifier服务
- 支持F190 VIN和F200温度阈值
- 集成权限检查

Closes #123
```

---

**项目初始化和工具配置完成！**
