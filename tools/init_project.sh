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

- 项目维护者: [Rocket]
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