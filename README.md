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
