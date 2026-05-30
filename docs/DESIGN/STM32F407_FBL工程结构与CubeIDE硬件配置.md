# STM32F407 AUTOSAR ECU - FBL工程结构与CubeIDE配置指南

## 第一部分：FBL 工程结构 (Bootloader设计)

### 1.1 FBL 是否需要单独工程？

**答案：不需要单独工程，但需要在同一工程中分离编译。**

#### 为什么这样做？

1. **代码共享**：FBL和APP共享MCAL驱动、类型定义、配置文件
2. **一致性**：确保两者使用相同的底层驱动实现
3. **版本管理**：同一个Git仓库，便于版本控制
4. **编译效率**：共享编译的目标文件，减少重复编译

#### FBL vs APP 的差异

```
共享部分 (src/config, src/mcal, inc/):
├── MCAL驱动层 (CAN/UART/Flash/GPIO等)
├── 公共头文件 (types.h, common.h, error_codes.h)
├── 配置文件 (pin_config.c, system_config.c)
└── 工具函数 (CRC32, 内存操作等)

仅FBL部分 (src/fbl/):
├── fbl_main.c              # FBL入口
├── fbl_flash_driver.c      # Flash擦写逻辑
├── fbl_dcm_service.c       # FBL诊断服务 (0x10/0x34/0x36/0x37)
├── fbl_security.c          # AES128加密
├── fbl_counter_manage.c    # 刷写计数器
└── fbl_app_validate.c      # APP有效性检查

仅APP部分 (src/app, src/asw, src/bsw):
├── app/                    # 主应用程序
├── asw/                    # Simulink应用层
└── bsw/com/dcm等           # 应用层诊断服务
```

---

### 1.2 FBL 工程文件放置方案

#### 方案A：同一工程 (推荐 ✓)

```
STM32F407_ECU_Project/
│
├── src/
│   ├── mcal/               # FBL和APP共享
│   ├── config/             # FBL和APP共享
│   │
│   ├── fbl/                # 仅FBL
│   │   ├── fbl_main.c
│   │   ├── fbl_flash_driver.c
│   │   ├── fbl_dcm_service.c
│   │   ├── fbl_security.c
│   │   ├── fbl_counter_manage.c
│   │   ├── fbl_app_validate.c
│   │   ├── fbl_app_jump.c
│   │   ├── fbl_crc_check.c
│   │   └── fbl_recovery.c
│   │
│   ├── bsw/                # APP特定模块 (FBL简化版)
│   ├── rte/                # APP特定
│   ├── asw/                # APP特定
│   └── app/                # APP特定
│
├── inc/
│   ├── mcal/               # FBL和APP共享
│   ├── config/             # FBL和APP共享
│   ├── fbl/                # FBL特定头文件
│   ├── bsw/                # APP特定头文件
│   └── ...
│
├── CMakeLists.txt          # 支持两个编译目标
│
└── build/
    ├── firmware_app.elf    # APP固件
    ├── firmware_app.hex
    ├── firmware_fbl.elf    # FBL固件
    └── firmware_fbl.hex
```

#### 优点：
✓ 代码复用，减少维护负担
✓ 共享MCAL驱动，保证一致性
✓ 一个Git仓库管理
✓ CMake支持多目标编译

#### 方案B：分离工程 (不推荐 ✗)

```
Project_Root/
├── STM32F407_APP_Project/     # APP工程
├── STM32F407_FBL_Project/     # FBL工程 (独立)
└── Shared_Libraries/          # 共享库 (复杂)
```

缺点：代码重复、同步困难、维护复杂

---

### 1.3 FBL 与 APP 编译关键配置

#### CMakeLists.txt 配置 (支持双编译目标)

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(STM32F407_ECU C ASM)

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

# ============= 公共源文件 (FBL和APP都需要) =============

set(COMMON_SOURCES
    src/mcal/can/can_driver.c
    src/mcal/can/can_interrupt.c
    src/mcal/adc/adc_driver.c
    src/mcal/timer/timer_driver.c
    src/mcal/timer/timer_systick.c
    src/mcal/timer/timer_iwdg.c
    src/mcal/gpio/gpio_driver.c
    src/mcal/uart/uart_driver.c
    src/mcal/spi/spi_driver.c
    src/mcal/spi/spi_flash.c
    src/mcal/pwm/pwm_driver.c
    src/mcal/nvm/eeprom_driver.c
    src/mcal/nvm/flash_driver.c
    src/mcal/clock/clock_driver.c
    src/mcal/power/power_manager.c
    src/config/pin_config.c
    src/config/system_config.c
    src/config/can_matrix.c
    src/common.c
)

# ============= APP 特定源文件 =============

set(APP_SOURCES
    src/bsw/com/com_manager.c
    src/bsw/canif/canif_driver.c
    src/bsw/dcm/dcm_main.c
    src/bsw/dcm/dcm_service_0x22.c
    src/bsw/dcm/dcm_service_0x2E.c
    src/bsw/dcm/dcm_service_0x27.c
    src/bsw/dem/dem_main.c
    src/bsw/dem/dem_monitor.c
    src/bsw/cannm/cannm_main.c
    src/bsw/nvm/nvm_manager.c
    src/rte/rte_main.c
    src/rte/rte_interface.c
    src/asw/app_model_step_10ms.c
    src/asw/app_motor_control.c
    src/app/main.c
    src/app/app_init.c
    src/app/app_task_scheduler.c
    src/app/app_systick_handler.c
    src/app/app_lowpower.c
    src/app/app_watchdog.c
)

# ============= FBL 特定源文件 =============

set(FBL_SOURCES
    src/fbl/fbl_main.c
    src/fbl/fbl_init.c
    src/fbl/fbl_flash_driver.c
    src/fbl/fbl_download.c
    src/fbl/fbl_dcm_service.c
    src/fbl/fbl_security.c
    src/fbl/fbl_crc_check.c
    src/fbl/fbl_counter_manage.c
    src/fbl/fbl_app_validate.c
    src/fbl/fbl_app_jump.c
    src/fbl/fbl_recovery.c
    src/fbl/fbl_watchdog.c
)

# ============= 编译目标 1: APP固件 =============

add_executable(firmware_app.elf
    ${COMMON_SOURCES}
    ${APP_SOURCES}
)

target_include_directories(firmware_app.elf PRIVATE
    inc inc/mcal inc/bsw inc/rte inc/asw inc/app inc/config
)

target_compile_definitions(firmware_app.elf PRIVATE
    STM32F407xx
    USE_HAL_DRIVER
    ARM_MATH_CM4
    # APP 特定定义
    BUILD_TARGET_APP
)

# APP 链接脚本 (0x0801_0000 开始)
set_target_properties(firmware_app.elf PROPERTIES
    LINK_FLAGS "-Wl,--script=${CMAKE_SOURCE_DIR}/linker/STM32F407VE_APP.ld"
)

# ============= 编译目标 2: FBL固件 =============

add_executable(firmware_fbl.elf
    ${COMMON_SOURCES}
    ${FBL_SOURCES}
)

target_include_directories(firmware_fbl.elf PRIVATE
    inc inc/mcal inc/fbl inc/config
)

target_compile_definitions(firmware_fbl.elf PRIVATE
    STM32F407xx
    USE_HAL_DRIVER
    ARM_MATH_CM4
    # FBL 特定定义
    BUILD_TARGET_FBL
)

# FBL 链接脚本 (0x0800_0000 开始)
set_target_properties(firmware_fbl.elf PROPERTIES
    LINK_FLAGS "-Wl,--script=${CMAKE_SOURCE_DIR}/linker/STM32F407VE_FBL.ld"
)

# ============= Post-build Commands =============

add_custom_command(TARGET firmware_app.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex firmware_app.elf firmware_app.hex
    COMMAND arm-none-eabi-objcopy -O binary firmware_app.elf firmware_app.bin
    COMMAND arm-none-eabi-size firmware_app.elf
)

add_custom_command(TARGET firmware_fbl.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex firmware_fbl.elf firmware_fbl.hex
    COMMAND arm-none-eabi-objcopy -O binary firmware_fbl.elf firmware_fbl.bin
    COMMAND arm-none-eabi-size firmware_fbl.elf
)
```

#### 编译命令

```bash
# 编译APP固件
mkdir -p build && cd build
cmake ..
make firmware_app.elf -j4

# 编译FBL固件
make firmware_fbl.elf -j4

# 同时编译两个
make -j4
```

---

### 1.4 FBL 链接脚本 (linker/STM32F407VE_FBL.ld)

```ld
/* STM32F407VE_FBL.ld - FBL Bootloader 链接脚本 */

MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 64K   /* FBL空间 (64KB) */
  RAM (rwx)       : ORIGIN = 0x20000000, LENGTH = 192K  /* 全部RAM */
  CCRAM (rwx)     : ORIGIN = 0x10000000, LENGTH = 64K   /* 核心耦合RAM */
}

/* FBL段定义 */
SECTIONS
{
  /* 代码段 (FBL代码) */
  .text :
  {
    KEEP(*(.vectors))           /* 中断向量表 */
    *(.text)                    /* FBL代码 */
    *(.text.*)
    *(.rodata)                  /* 常数 */
    *(.rodata.*)
  } > FLASH

  /* 初始化数据段 */
  .data :
  {
    *(.data)
    *(.data.*)
  } > RAM AT > FLASH

  /* 未初始化数据段 */
  .bss :
  {
    *(.bss)
    *(.bss.*)
    *(COMMON)
  } > RAM

  /* FBL特定段：刷写次数计数器存储位置 */
  .fbl_config :
  {
    fbl_config_start = .;
    *(.fbl_config)
    fbl_config_end = .;
  } > RAM

  /* 堆和栈 */
  .heap :
  {
    heap_start = .;
    . += 4K;                    /* 堆大小 4KB */
    heap_end = .;
  } > RAM

  .stack :
  {
    . += 8K;                    /* 栈大小 8KB */
    stack_top = .;
  } > RAM
}

/* 符号定义 */
_etext = LOADADDR(.data);
_sdata = ADDR(.data);
_edata = _sdata + SIZEOF(.data);
_sbss = ADDR(.bss);
_ebss = _sbss + SIZEOF(.bss);
```

---

### 1.5 APP 链接脚本 (linker/STM32F407VE_APP.ld)

```ld
/* STM32F407VE_APP.ld - APP应用程序 链接脚本 */

MEMORY
{
  FLASH (rx)      : ORIGIN = 0x08010000, LENGTH = 448K  /* APP空间 (448KB) */
  RAM (rwx)       : ORIGIN = 0x20000000, LENGTH = 192K  /* 全部RAM */
  CCRAM (rwx)     : ORIGIN = 0x10000000, LENGTH = 64K   /* 核心耦合RAM */
}

SECTIONS
{
  /* APP有效性标志位 (0x08010000处) */
  .app_valid :
  {
    app_valid_marker = .;
    LONG(0x55AA55AA)            /* APP有效标志 */
  } > FLASH

  /* 代码段 */
  .text :
  {
    KEEP(*(.vectors))           /* 中断向量表 */
    *(.text)
    *(.text.*)
    *(.rodata)
    *(.rodata.*)
  } > FLASH

  /* 初始化数据 */
  .data :
  {
    *(.data)
    *(.data.*)
  } > RAM AT > FLASH

  /* 未初始化数据 */
  .bss :
  {
    *(.bss)
    *(.bss.*)
    *(COMMON)
  } > RAM

  /* 堆和栈 */
  .heap :
  {
    heap_start = .;
    . += 8K;
    heap_end = .;
  } > RAM

  .stack :
  {
    . += 12K;
    stack_top = .;
  } > RAM
}
```

---

## 第二部分：STM32CubeIDE 硬件配置详解

### 2.1 STM32CubeIDE 创建工程

#### 步骤 1: 新建工程

1. **File → New → STM32 Project**
2. **选择芯片**：搜索 `STM32F407VE`
3. **选择MCU**: `STM32F407VET6`
4. **工程名称**: `STM32F407_ECU`
5. **工程模式**: C Project → Empty (不使用模板)
6. **Toolchain**: STM32CubeIDE

#### 步骤 2: 打开CubeMX配置

1. 右键工程 → **Properties**
2. 找到 `.ioc` 文件 (通常在 Core 目录下)
3. **双击** `.ioc` 文件打开CubeMX配置器

---

### 2.2 硬件引脚配置 (基于电路设计)

#### 配置清单及原因分析

| 模块 | 引脚 | 功能 | CubeMX设置 | 配置原因 |
|------|------|------|-----------|---------|
| **CAN1** | PB8 | CAN1_RX | 复用功能 AF9 | 通往CAN收发器RX引脚 |
| | PB9 | CAN1_TX | 复用功能 AF9 | 通往CAN收发器TX引脚 |
| | PA8 | GPIO Out | GPIO_Output (低电平=工作, 高=待机) | CAN收发器睡眠/唤醒控制 (SIT1042T) |
| **CAN2** | PB12 | CAN2_RX | 复用功能 AF9 | 第二路CAN接口 |
| | PB13 | CAN2_TX | 复用功能 AF9 | 第二路CAN接口 |
| **LIN1** | PA2 | USART2_TX | 复用功能 AF7 | LIN1主节点发送 |
| | PA3 | USART2_RX | 复用功能 AF7 | LIN1主节点接收 |
| **LIN2** | PB10 | USART3_TX | 复用功能 AF7 | LIN2备用收发 |
| | PB11 | USART3_RX | 复用功能 AF7 | LIN2备用收发 |
| **调试UART** | PA9 | USART1_TX | 复用功能 AF7 | 调试信息输出 |
| | PA10 | USART1_RX | 复用功能 AF7 | 接收调试命令 |
| **LED_PWM** | PD12 | TIM4_CH1 | PWM输出 | LED亮度调节 (0-80% PWM占空比) |
| **MOTOR_PWM** | PD13 | TIM4_CH2 | PWM输出 | 马达速度控制 |
| **MOTOR_DIR** | PD14 | GPIO Out | GPIO_Output | 马达正反转控制 |
| **温度传感器** | PC5 | ADC1_IN15 | ADC模拟输入 | ADC采样温度 (0-100℃对应0-4095) |
| **按键K0** | PE4 | GPIO In + EXTI | GPIO_Input + 中断下降沿 | 本地唤醒源 (低电平=按下) |
| **按键K1** | PE3 | GPIO In + EXTI | GPIO_Input + 中断下降沿 | 普通按键输入 |
| **W25Q16_CS** | PB0 | SPI1_NSS | 复用功能 AF5 | SPI Flash片选 (默认高电平 低电平选中) |
| **W25Q16_CLK** | PB3 | SPI1_SCK | 复用功能 AF5 | SPI时钟 |
| **W25Q16_MISO** | PB4 | SPI1_MISO | 复用功能 AF5 | SPI主入从出 (数据读) |
| **W25Q16_MOSI** | PB5 | SPI1_MOSI | 复用功能 AF5 | SPI主出从入 (数据写) |
| **调试器SWD** | PA13 | SWDIO | SWD | ST-LINK调试器数据线 |
| | PA14 | SWCLK | SWD | ST-LINK调试器时钟线 |

---

### 2.3 CubeMX 具体配置步骤

#### 2.3.1 CAN配置 (0x210, 0x415, 0x1A0等报文)

**为什么配置CAN？**
- ECU需要接收VCU控制命令 (Vehicle_Ctrl 0x210)
- ECU需要发送状态反馈 (ECU_Status 0x1A0)
- ECU需要网络管理报文 (ECU_NM_0x415)

**CubeIDE CAN1配置步骤：**

1. **Connectivity → CAN1**
   ```
   Mode: Normal
   Prescaler: 5           // 42MHz / 5 = 8.4MHz时钟, 可产生500kbps
   Time Segment 1: 3TQ    // 传播延迟段 + 相位缓冲1段
   Time Segment 2: 4TQ    // 相位缓冲2段
   SJW: 1TQ               // 同步跳转窗口
   ```

2. **Pin配置**
   - PB8 → CAN1_RX
   - PB9 → CAN1_TX
   - PA8 → GPIO_Output (CAN收发器控制)

3. **NVIC中断配置**
   ```
   CAN1 RX0 interrupt    → Enable
   CAN1 RX1 interrupt    → Enable
   CAN1 SCE interrupt    → Enable (Busoff)
   ```

**为什么选择PA8作为CAN收发器睡眠控制？**
- PA8不与其他关键功能冲突
- SIT1042T收发器: STB (待机) 引脚低电平=工作, 高电平=待机
- 低功耗模式时 PA8 → 高，禁用CAN收发器

---

#### 2.3.2 ADC配置 (温度采样)

**为什么需要ADC？**
- 采集温度传感器 (DS18B20) 电压 (0-3.3V)
- 触发DTC 0x030003 (温度过高)
- 映射到ECU_Temperature信号

**CubeIDE ADC1配置：**

1. **Analog → ADC1**
   ```
   ADC Resolution: 12-bit (精度 3.3V/4096=0.8mV/LSB)
   Sampling Time: 480 Cycles (转换时间约7.2μs)
   Regular Conversion Mode: Single
   ```

2. **选择通道**
   - Channel: `IN15` (PE0引脚对应)
   - Rank: 1
   - Sampling Time: 480 Cycles (确保稳定采样)

3. **NVIC中断**
   ```
   ADC1 global interrupt → Enable
   ```

**温度映射公式：**
```
物理温度(℃) = ADC_Value × 0.1 - 40
例: ADC_Value = 0x80 (128) → 温度 = 128×0.1 - 40 = -27.2℃
例: ADC_Value = 0xF0 (240) → 温度 = 240×0.1 - 40 = -16℃
```

---

#### 2.3.3 PWM配置 (LED和马达)

**为什么需要PWM？**
- LED亮度调节 (0-80% PWM占空比)
- 马达速度控制 (0-100% PWM)

**CubeIDE Timer4 (TIM4) 配置：**

1. **Timers → TIM4**
   ```
   Clock Source: Internal Clock
   Channel 1: PWM Generation (LED)
   Channel 2: PWM Generation (Motor)
   ```

2. **PWM参数**
   ```
   Prescaler: 84 - 1          // 168MHz / 84 = 2MHz时钟
   Counter Period (ARR): 2000 // 频率 = 2MHz / 2000 = 1kHz (1ms周期)
   Channel 1 Pulse: 0-2000    // 占空比 = Pulse/2000 × 100%
   Channel 2 Pulse: 0-2000
   ```

3. **PIN配置**
   - PD12 → TIM4_CH1 (LED_PWM)
   - PD13 → TIM4_CH2 (MOTOR_PWM)

4. **NVIC**
   ```
   TIM4 global interrupt → Enable
   ```

**LED亮度映射：**
```
LED_Brightness_Level (0-10) → PWM占空比 (0-80%)
占空比 = LED_Brightness × 8 (即 0, 8, 16, 24... 160%)
例: Level=5 → 占空比 = 40% → Pulse = 800
```

---

#### 2.3.4 SPI配置 (W25Q16 外部Flash)

**为什么需要外部Flash？**
- 存储DID参数 (VIN、温度阈值)
- 存储DTC故障码 (持久化)
- 存储刷写次数计数器 (F501)
- APP有效性标志位

**CubeIDE SPI1配置：**

1. **Connectivity → SPI1**
   ```
   Mode: Transmit and Receive
   Data Format: 8-bit (MSB First)
   Baudrate Prescaler: 32    // 168MHz / 32 = 5.25MHz (W25Q16支持最高104MHz)
   CPOL: Low
   CPHA: 1Edge
   ```

2. **Pin配置**
   - PB3 → SPI1_SCK (关闭JTAG, 仅保留SWD)
   - PB4 → SPI1_MISO
   - PB5 → SPI1_MOSI
   - PB0 → GPIO_Output (CS片选, 低电平选中)

3. **注意PB3冲突**
   ```
   PB3默认为JTDOTRACESWO (JTAG调试接口)
   需要在CubeMX中配置：
   Debug → 选择 "Serial Wire" (仅SWD)
   这样才能释放PB3用于SPI时钟
   ```

---

#### 2.3.5 UART配置 (调试串口)

**为什么需要调试UART？**
- 打印调试日志 (FBL启动、APP启动、HardFault等)
- 串口波特率：115200 bps

**CubeIDE USART1配置：**

1. **Connectivity → USART1**
   ```
   Mode: Asynchronous
   Baud Rate: 115200
   Word Length: 8 Bit
   Stop Bits: 1
   Parity: None
   ```

2. **Pin配置**
   - PA9 → USART1_TX
   - PA10 → USART1_RX

3. **NVIC中断**
   ```
   USART1 global interrupt → Enable
   ```

**示例：打印版本信息**
```c
void print_version(void) {
    printf("FBL Version: %s\n", FBL_VERSION);
    printf("Build Date: %s %s\n", __DATE__, __TIME__);
}
```

---

#### 2.3.6 GPIO配置 (按键和马达方向)

**为什么需要按键GPIO？**
- PE1 (K0): 本地唤醒源 (外部中断下降沿)
- PE2 (K1): 普通按键输入 (外部中断下降沿)

**为什么需要马达方向GPIO？**
- PD14: 马达正反转控制 (0=反转, 1=正转)

**CubeIDE GPIO配置：**

1. **GPIO→Port E**
   ```
   PE1: GPIO_Input + EXTI
        GPIO mode: Input with external interrupt
        GPIO Pull-up/Pull-down: No Pull
        EXTI Line: PE1, Trigger: Falling edge
   
   PE2: GPIO_Input + EXTI
        GPIO mode: Input with external interrupt
        GPIO Pull-up/Pull-down: No Pull
        EXTI Line: PE2, Trigger: Falling edge
   ```

2. **GPIO→Port D**
   ```
   PD14: GPIO_Output
         Output level: Low
         GPIO mode: Output Push-Pull
         GPIO Speed: Medium
   ```

3. **NVIC配置**
   ```
   EXTI line[1:0] interrupts → Enable
   EXTI line[9:5] interrupts → Enable  // PE1、PE2属于[9:5]范围
   ```

---

### 2.4 时钟配置 (Clock Tree)

#### 为什么要配置时钟树？

1. **系统工作频率**：168MHz (App Performance = High Speed)
2. **外设分频**：APB1/APB2时钟用于各外设
3. **外部晶振**：8MHz (CubeMX自动配置PLL)

**CubeIDE时钟配置步骤：**

1. **System Core → RCC**
   ```
   High Speed Clock (HSE): Crystal/Ceramic Resonator
   Low Speed Clock (LSE): Crystal (32.768kHz)
   ```

2. **System Core → SYS**
   ```
   Timebase Source: SysTick (1ms中断)
   ```

3. **Clock Configuration → Clock Tree**
   ```
   输入: 8MHz HSE
   ↓ (/2) → 4MHz
   ↓ (×168) → 672MHz
   ↓ (/4) → 168MHz (SYSCLK)
   
   HCLK: 168MHz (AHB)
   PCLK1: 42MHz (APB1) - CAN/UART/SPI/Timer等
   PCLK2: 84MHz (APB2) - ADC/GPIO等
   ```

**为什么要配置SysTick 1ms？**
- 任务调度器基准 (10ms、100ms、1s任务轮询)
- 看门狗喂狗计时
- 故障监控防抖时间计数

---

### 2.5 完整的配置流程检查清单

```
☐ 1. 创建STM32F407_ECU工程
     └─ STM32CubeIDE新建工程

☐ 2. 打开CubeMX配置器
     └─ 双击 .ioc 文件

☐ 3. 基础设置
     └─ MCU: STM32F407VET6
     └─ 工具链: STM32CubeIDE

☐ 4. 时钟配置
     ├─ RCC: HSE = 8MHz, LSE = 32.768kHz
     ├─ SYS: SYSCLK = 168MHz
     └─ SysTick: Timebase = 1ms

☐ 5. CAN配置
     ├─ CAN1: 500kbps
     ├─ CAN1_RX = PB8, CAN1_TX = PB9
     ├─ PA8 = GPIO输出 (CAN收发器STB)
     ├─ CAN1 RX0/RX1中断: Enable
     └─ CAN1 SCE中断: Enable (Busoff)

☐ 6. ADC配置
     ├─ ADC1_IN15 = PE0 (温度传感器)
     ├─ 12-bit分辨率
     ├─ 采样时间: 480周期
     └─ ADC1 全局中断: Enable

☐ 7. PWM配置 (TIM4)
     ├─ TIM4_CH1 = PD12 (LED_PWM)
     ├─ TIM4_CH2 = PD13 (MOTOR_PWM)
     ├─ 频率: 1kHz (ARR=2000, Prescaler=83)
     └─ TIM4 全局中断: Enable

☐ 8. SPI配置 (W25Q16)
     ├─ SPI1: 5.25MHz波特率
     ├─ PB3 = SPI1_SCK (需要关闭JTAG)
     ├─ PB4 = SPI1_MISO
     ├─ PB5 = SPI1_MOSI
     └─ PB0 = GPIO输出 (CS片选)

☐ 9. UART配置 (调试)
     ├─ USART1: 115200 bps
     ├─ PA9 = USART1_TX
     ├─ PA10 = USART1_RX
     └─ USART1 全局中断: Enable

☐ 10. GPIO配置
      ├─ PE1 = GPIO_Input + EXTI (K0唤醒)
      ├─ PE2 = GPIO_Input + EXTI (K1)
      ├─ PD14 = GPIO输出 (马达方向)
      ├─ EXTI[1:0]中断: Enable
      └─ EXTI[9:5]中断: Enable

☐ 11. 调试器配置
      ├─ Debug: Serial Wire (SWD)
      └─ SWD引脚: PA13(SWDIO), PA14(SWCLK)

☐ 12. 代码生成
      └─ Project → Generate Code
```

---

### 2.6 CubeMX生成代码后的处理

**生成的文件结构：**

```
Core/
├── Inc/
│   ├── main.h
│   ├── stm32f4xx_it.h           ← 中断处理声明
│   └── stm32f4xx_hal_conf.h     ← HAL库配置
└── Src/
    ├── main.c                    ← main() 函数
    ├── stm32f4xx_it.c            ← 中断服务例程
    └── stm32f4xx_startup_*.c     ← 启动代码

Drivers/
├── STM32F4xx_HAL_Driver/        ← HAL库驱动
└── CMSIS/                       ← ARM CMSIS标准库
```

**重点文件修改：**

1. **Core/Src/main.c** (保留CubeMX生成的初始化)
   ```c
   int main(void) {
       HAL_Init();                 // ← 保留
       SystemClock_Config();       // ← 保留
       MX_GPIO_Init();             // ← 保留
       MX_CAN1_Init();             // ← 保留
       // ... 其他外设初始化
       
       // 自己的应用代码
       App_System_Init();          // ← 添加
       
       while (1) {
           App_Task_10ms();        // ← 添加
           App_Task_100ms();
           App_Task_1000ms();
       }
   }
   ```

2. **Core/Src/stm32f4xx_it.c** (关键中断处理)
   ```c
   // SysTick 1ms 中断 (由CubeMX生成框架)
   void SysTick_Handler(void) {
       HAL_IncTick();              // ← 保留
       App_SysTick_ISR();          // ← 添加自己的处理
       App_Watchdog_Feed();        // ← 喂狗
   }
   
   // CAN RX0 中断
   void CAN1_RX0_IRQHandler(void) {
       Mcal_Can_RxISR();           // ← 调用MCAL驱动
   }
   
   // 外部中断 (按键K0/K1)
   void EXTI1_IRQHandler(void) {   // PE1
       __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
       App_Button_K0_Handler();    // ← 唤醒处理
   }
   ```

---

## 总结对比表

| 方面 | 配置原因 |
|------|---------|
| **CAN 500kbps** | 符合车载CAN标准, 满足实时通信需求 |
| **ADC 12-bit** | 精度0.8mV/LSB, 足以分辨温度变化 |
| **PWM 1kHz** | LED闪烁不可见, 马达转速平滑 |
| **SPI 5.25MHz** | W25Q16支持最高104MHz, 5.25MHz安全稳定 |
| **UART 115200** | 标准调试波特率, 足够快 |
| **SysTick 1ms** | 基准中断, 支持10ms/100ms/1s任务 |
| **EXTI下降沿** | 按键低电平有效, 下降沿触发中断 |
| **PA8控制CAN休眠** | SIT1042T睡眠模式需要STB脚 |
| **关闭JTAG仅保留SWD** | 释放PB3用于SPI_SCK |
| **GPIO_Output推挽** | 足以驱动LED和马达方向控制 |

---

## 下一步操作指南

1. **在STM32CubeIDE中完成上述配置** (参考清单)
2. **生成代码** (Project → Generate Code)
3. **验证编译** (无错误)
4. **修改main.c和中断处理** (集成你的应用层代码)
5. **烧写验证** (通过ST-LINK)

所有硬件配置都基于需求规格，确保了ECU的正确功能实现。
