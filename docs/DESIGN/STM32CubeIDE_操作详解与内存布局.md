# STM32CubeIDE 操作详解与内存布局

## 第一部分：STM32CubeIDE 创建工程详细步骤

### 1.1 创建新工程

#### 第1步：启动CubeIDE，创建新项目

```
菜单: File → New → STM32 Project
```

#### 第2步：Board Selector 界面

**搜索芯片步骤：**

```
┌─────────────────────────────────────────┐
│  Board Selector / MCU Selector           │
├─────────────────────────────────────────┤
│  搜索框: [STM32F407VE      ]             │
│                                          │
│  搜索结果:                               │
│  ✓ STM32F407VET6  (您的芯片)            │
│  ✓ STM32F407VEH6                        │
│  ✓ STM32F407VGT6  (高端版, 1MB Flash) │
│                                          │
│  [选择STM32F407VET6] → [Next]           │
└─────────────────────────────────────────┘
```

**关键参数确认：**
```
MCU Information:
├─ Core: Cortex-M4
├─ Frequency: 168 MHz
├─ Flash: 512 KB  ← 关键
├─ RAM: 192 KB    ← 关键
└─ Packages: LQFP100
```

#### 第3步：工程配置

```
┌─────────────────────────────────────────┐
│  Project Setup                           │
├─────────────────────────────────────────┤
│ Project Name: STM32F407_ECU             │
│                                          │
│ Project Location: 选择你的工作目录      │
│ [/path/to/STM32F407_ECU_Project]        │
│                                          │
│ Project Type:                            │
│   ○ C Project  ✓ (选中)                 │
│   ○ C++ Project                         │
│                                          │
│ Target language: C                       │
│                                          │
│ Toolchain/IDE: STM32CubeIDE             │
│                                          │
│ Targeted Binary: STM32CubeMX (即将生成) │
└─────────────────────────────────────────┘
```

#### 第4步：确认创建

```
[Finish] 
└─ 等待IDE初始化... (约20秒)
```

**创建完成后看到的结构：**

```
STM32F407_ECU
│
├── .cproject               ← IDE项目配置
├── .project                ← IDE项目文件
├── STM32F407VETx_FLASH.ld  ← 链接脚本 (自动生成)
├── Makefile                ← 编译脚本
│
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   └── stm32f4xx_hal_conf.h
│   │
│   └── Src/
│       ├── main.c
│       └── stm32f4xx_it.c
│
└── Drivers/
    └── STM32F4xx_HAL_Driver/
```

---

### 1.2 打开CubeMX配置器

#### 方法1：双击 .ioc 文件（推荐）

```
项目浏览窗口:
STM32F407_ECU/
│
└── STM32F407_ECU.ioc  ← 双击此文件
```

**CubeMX配置器打开后：**

```
┌──────────────────────────────────────────────────┐
│  STM32CubeMX - STM32F407VET6 Configuration      │
├──────────────────────────────────────────────────┤
│                                                   │
│  左侧菜单树:                                     │
│  ├─ Pinout & Configuration  ← 从这里开始       │
│  │  ├─ RCC (时钟配置)                           │
│  │  ├─ SYS (系统)                               │
│  │  ├─ GPIO (引脚配置)                          │
│  │  ├─ Connectivity (CAN/UART/SPI等)            │
│  │  ├─ Analog (ADC)                             │
│  │  ├─ Timer (定时器PWM)                        │
│  │  └─ NVIC (中断配置)                          │
│  │                                               │
│  └─ Clock Configuration  ← 时钟树配置          │
│                                                   │
│  中央区域: 芯片引脚图 (100个引脚视图)            │
│  右侧: 配置参数面板                              │
│                                                   │
└──────────────────────────────────────────────────┘
```

---

## 第二部分：详细硬件配置操作

### 2.1 第一步：时钟配置 (最重要)

#### 2.1.1 RCC (外部晶振配置)

```
菜单树: Pinout & Configuration → System Core → RCC
```

**配置内容：**

```
┌─────────────────────────────────────────────────────┐
│ RCC Configuration                                   │
├─────────────────────────────────────────────────────┤
│                                                     │
│ High Speed Clock (HSE):                             │
│   ○ Disabled                                        │
│   ○ Crystal/Ceramic Resonator  ✓ (选中)           │
│   └─ Frequency: 8 MHz           ← 板上晶振        │
│                                                     │
│ Low Speed Clock (LSE):                              │
│   ○ Disabled                                        │
│   ○ Crystal/Ceramic Resonator  ✓ (选中)           │
│   └─ Frequency: 32.768 kHz     ← RTC晶振          │
│                                                     │
│ 说明: 这两个晶振CubeMX会自动配置为时钟源          │
│                                                     │
└─────────────────────────────────────────────────────┘
```

#### 2.1.2 Clock Configuration (时钟树)

```
菜单树: Pinout & Configuration → Clock Configuration
```

**关键参数设置：**

```
输入侧:
┌─────────────────────┐
│  HSE: 8 MHz         │
│  (外部晶振)         │
└─────────────────────┘
         │
         ↓ (/M=2)
    ┌─────────────┐
    │ PLLM = 2    │
    │ 4 MHz       │
    └─────────────┘
         │
         ↓ (×N)
    ┌──────────────┐
    │ PLLN = 168   │
    │ 672 MHz      │
    └──────────────┘
         │
         ├─ ↓ (/P=4)  ──→ SYSCLK: 168 MHz ✓
         ├─ ↓ (/Q=7)  ──→ USB: 48 MHz (不用)
         └─ ↓ (/R)    ──→ (不用)
         
系统工作频率: SYSCLK = 168 MHz

```

**关键值参考表：**

```
┌──────┬──────┬────────┬──────┐
│  M   │  N   │  P     │ 结果 │
├──────┼──────┼────────┼──────┤
│  2   │ 168  │ 4      │ 168MHz (推荐)  │
│  2   │ 126  │ 4      │ 126MHz          │
│  2   │ 84   │ 4      │ 84MHz           │
└──────┴──────┴────────┴──────┘
```

**复制下面的数值到CubeMX:**

```
在 "Clock Configuration" 页面:

输入框 [PLLM]:  2
输入框 [PLLN]:  168
输入框 [PLLP]:  4
输入框 [PLLQ]:  7 (USB不用可任意)
```

**然后点击 "Resolve Clock Tree" 验证:**

```
应该看到:
✓ SYSCLK = 168 MHz
✓ HCLK = 168 MHz (AHB总线)
✓ PCLK1 = 42 MHz (APB1: CAN/UART/Timer)
✓ PCLK2 = 84 MHz (APB2: ADC/GPIO)
```

---

### 2.2 第二步：SYS 配置 (系统基础)

```
菜单树: Pinout & Configuration → System Core → SYS
```

**配置参数：**

```
┌──────────────────────────────────────────────────┐
│ System Configuration                             │
├──────────────────────────────────────────────────┤
│                                                  │
│ Debug:                                           │
│   ○ Disabled                                     │
│   ○ Serial Wire  ✓ (选中)                      │
│   └─ 说明: 仅使用PA13/PA14作为SWD调试器        │
│                                                  │
│ Timebase Source:                                 │
│   ○ SysTick (系统时钟)  ✓ (选中)                │
│   ○ TIM1                                         │
│   └─ 间隔: 1ms  ← 重要！任务调度基准           │
│                                                  │
│ MPU:                                             │
│   ○ Disabled  ✓ (不需要)                        │
│                                                  │
│ Use float with printf from newlib:              │
│   ☐ 不选 (减少代码量)                           │
│                                                  │
└──────────────────────────────────────────────────┘
```

**为什么选择SysTick 1ms？**
```
理由:
1. SysTick是内核定时器，精度高，专用
2. 1ms是理想的基准中断周期
3. 10ms、100ms、1s任务都可以轻松实现
4. 看门狗喂狗、故障防抖都用这个时基

计算:
系统频率 = 168 MHz
要产生 1ms 中断:
中断周期 = 1 / 1000 = 0.001 秒 = 1000 微秒
SysTick计数值 = 168 MHz × 0.001 = 168,000
```

---

### 2.3 第三步：CAN 配置 (最复杂)

#### 2.3.1 CAN1 基本配置

```
菜单树: Pinout & Configuration → Connectivity → CAN1
```

**参数设置：**

```
┌──────────────────────────────────────────────────┐
│ CAN1 Configuration                               │
├──────────────────────────────────────────────────┤
│                                                  │
│ Mode:                                            │
│   ○ Disabled                                     │
│   ○ Normal  ✓ (选中)                            │
│                                                  │
│ Hardware Settings:                               │
│   Prescaler: 5          ← 关键参数              │
│   Time Quantum 1: 3TQ   ← 传播延迟             │
│   Time Quantum 2: 4TQ   ← 相位缓冲             │
│   SJW: 1TQ              ← 同步跳跃窗口          │
│                                                  │
│ 说明:                                            │
│ APB1时钟 = 42 MHz                               │
│ Prescaler = 5                                    │
│ 时钟频率 = 42 MHz / 5 = 8.4 MHz                │
│ 时间量子 = 1 / 8.4 MHz = 0.119 μs              │
│ CAN波特率 = 8.4M / (1+3+4) = 500 kbps         │
│                                                  │
└──────────────────────────────────────────────────┘
```

**标准波特率参数表：**

```
┌───────────┬───────────┬──────┬──────┬──────┬─────────┐
│ 波特率    │ APB1时钟  │ 分频 │ Seg1 │ Seg2 │ Prescale│
├───────────┼───────────┼──────┼──────┼──────┼─────────┤
│ 1000 kbps │ 42 MHz    │ 3    │ 3TQ  │ 4TQ  │ 1       │
│ 500 kbps  │ 42 MHz    │ 5    │ 3TQ  │ 4TQ  │ 1   ✓   │
│ 250 kbps  │ 42 MHz    │ 10   │ 3TQ  │ 4TQ  │ 1       │
│ 125 kbps  │ 42 MHz    │ 21   │ 3TQ  │ 4TQ  │ 1       │
└───────────┴───────────┴──────┴──────┴──────┴─────────┘
```

#### 2.3.2 CAN1 引脚配置

**切换到 "Pinout" 标签页:**

```
芯片引脚视图 (100引脚LQFP):

找到 PB8 引脚:
├─ 左键单击 PB8
├─ 弹出菜单: CAN1_RX  ← 选中
├─ PB8 变为绿色，标注为 "CAN1_RX"
└─ ✓ 配置完成

找到 PB9 引脚:
├─ 左键单击 PB9
├─ 弹出菜单: CAN1_TX  ← 选中
├─ PB9 变为绿色，标注为 "CAN1_TX"
└─ ✓ 配置完成
```

**添加GPIO输出 (CAN收发器控制):**

```
找到 PA8 引脚:
├─ 左键单击 PA8
├─ 弹出菜单: GPIO_Output  ← 选中
├─ PA8 变为黄色，标注为 "GPIO_Output"
└─ ✓ 配置完成

配置PA8属性:
├─ GPIO mode: Output Push-Pull
├─ GPIO level: High (初始态，禁用CAN)
├─ GPIO speed: Medium
└─ GPIO Pull-up/Pull-down: No Pull
```

#### 2.3.3 CAN1 中断配置

```
菜单树: Pinout & Configuration → System Core → NVIC
```

**找到CAN相关中断并Enable：**

```
NVIC 中断列表:
│
├─ CAN1 RX0 interrupt (EXTI line[0])
│  └─ ☑ Enable (打勾)
│
├─ CAN1 RX1 interrupt
│  └─ ☑ Enable
│
├─ CAN1 SCE interrupt (Busoff)
│  └─ ☑ Enable
│
└─ CAN1 TX interrupt
   └─ ☑ Enable (可选)
```

---

### 2.4 第四步：ADC 配置 (温度采样)

```
菜单树: Pinout & Configuration → Analog → ADC1
```

**参数配置：**

```
┌──────────────────────────────────────────────────┐
│ ADC1 Configuration                               │
├──────────────────────────────────────────────────┤
│                                                  │
│ ADC Settings:                                    │
│   Resolution: 12-bit  ✓ (最精细: 3.3V/4096)    │
│   Data Alignment: Right                          │
│   Scan Conversion Mode: Disabled                 │
│   Continuous Conversion Mode: Disabled           │
│   DMA: Disabled                                  │
│                                                  │
│ Regular Channel Configuration:                   │
│   Rank: 1                                        │
│   Channel: IN15 (PE0)  ← 温度传感器           │
│   Sampling Time: 480 Cycles ← 防抖采样          │
│                                                  │
│ 说明:                                            │
│ 采样时间长→数据稳定，推荐480周期               │
│ ADC时钟 = 84 MHz / 2 = 42 MHz                   │
│ 转换时间 ≈ (480 + 12) / 42MHz ≈ 12 μs          │
│                                                  │
└──────────────────────────────────────────────────┘
```

**PE0 引脚配置：**

```
在 Pinout 标签页:

找到 PE0 引脚:
├─ 左键单击 PE0
├─ 弹出菜单: ADC1_IN15  ← 选中
├─ PE0 变为蓝色，标注为 "ADC1_IN15"
└─ ✓ 配置完成
```

**中断配置：**

```
菜单树: System Core → NVIC

找到:
├─ ADC1 global interrupt
│  └─ ☑ Enable
```

---

### 2.5 第五步：PWM 配置 (LED和马达)

```
菜单树: Pinout & Configuration → Timer → TIM4
```

**TIM4 基本配置：**

```
┌──────────────────────────────────────────────────┐
│ TIM4 Configuration                               │
├──────────────────────────────────────────────────┤
│                                                  │
│ Clock Source: Internal Clock                     │
│                                                  │
│ Channel Configurations:                          │
│   Channel 1: PWM Generation CH1  ✓              │
│   Channel 2: PWM Generation CH2  ✓              │
│   Channel 3: Disabled                           │
│   Channel 4: Disabled                           │
│                                                  │
│ PWM Parameters:                                  │
│   Prescaler: 83  (168MHz / 84 = 2MHz)           │
│   Auto-reload value (ARR): 1999                 │
│                                                  │
│   频率 = 2MHz / 2000 = 1kHz                     │
│   周期 = 1ms                                     │
│                                                  │
│ Channel 1 (LED):                                 │
│   Pulse: 0    (初始占空比 0%)                   │
│   Polarity: High                                 │
│                                                  │
│ Channel 2 (Motor):                               │
│   Pulse: 0    (初始占空比 0%)                   │
│   Polarity: High                                 │
│                                                  │
└──────────────────────────────────────────────────┘
```

**引脚配置：**

```
在 Pinout 标签页:

找到 PD12 引脚 (LED_PWM):
├─ 左键单击 PD12
├─ 弹出菜单: TIM4_CH1  ← 选中
├─ PD12 变为紫色，标注为 "TIM4_CH1"
└─ ✓ 配置完成

找到 PD13 引脚 (MOTOR_PWM):
├─ 左键单击 PD13
├─ 弹出菜单: TIM4_CH2  ← 选中
├─ PD13 变为紫色，标注为 "TIM4_CH2"
└─ ✓ 配置完成
```

**计算PWM参数的推导：**

```
需求: 1kHz PWM频率 (LED闪烁不可见，马达平滑)

给定:
├─ PCLK1时钟 = 42 MHz (APB1)
├─ 由于TIM4在APB1下，且APB1分频器 ≠ 1，所以TIM时钟 = 42 MHz × 2 = 84 MHz
└─ 欲产生1kHz: 需要 84 MHz / 84000 = 1000次计数

选择Prescaler:
├─ TIM_CLK = 84 MHz
├─ Prescaler = 84 - 1 = 83 (内部自增1)
├─ 分频后时钟 = 84 MHz / 84 = 1 MHz
└─ 产生1次脉冲需 1 μs

选择ARR (自动重装载寄存器):
├─ 欲频率 = 1kHz = 1000Hz
├─ 周期 = 1 / 1000 = 1ms
├─ ARR = 1MHz × 1ms - 1 = 1000 - 1 = 999 ✗ (不对)
├─ 重新: 分频后时钟 = 84MHz / 84 = 1MHz
├─ ARR = 1000 - 1 = 999 或 2000 - 1 = 1999
├─ 推荐 ARR = 1999 (提升精度)
└─ 实际频率 = 1MHz / 2000 = 500 Hz (不对!)

重新计算:
├─ 欲1kHz，需要APB1时钟供2MHz
├─ Prescaler = 84 / 2 - 1 = 41
├─ ARR = 2000 - 1 = 1999
├─ 频率 = 84MHz / 42 / 2000 = 1kHz ✓ 正确!

或更简单的参数:
├─ Prescaler: 83 (实际分频 84)
├─ ARR: 1999
├─ 时钟 = 84 / 84 = 1 MHz
├─ 频率 = 1 MHz / 2000 = 500 Hz ✗

最终确认参数:
├─ Prescaler: 41
├─ ARR: 1999
├─ 分频后 = 84MHz / 42 = 2MHz
├─ 频率 = 2MHz / 2000 = 1kHz ✓
```

**中断配置：**

```
菜单树: System Core → NVIC

找到:
├─ TIM4 global interrupt
│  └─ ☑ Enable
```

---

### 2.6 第六步：SPI 配置 (W25Q16 Flash)

```
菜单树: Pinout & Configuration → Connectivity → SPI1
```

**参数配置：**

```
┌──────────────────────────────────────────────────┐
│ SPI1 Configuration                               │
├──────────────────────────────────────────────────┤
│                                                  │
│ Mode: Full-Duplex Master                         │
│                                                  │
│ Hardware Settings:                               │
│   Data Size: 8 Bits                              │
│   First Bit: MSB First                           │
│   Clock Polarity (CPOL): Low                     │
│   Clock Phase (CPHA): 1 Edge                     │
│   NSS Signal: Software (手动控制CS)             │
│   Baud Rate Prescaler: /32                       │
│     └─ 频率 = 168MHz / 32 = 5.25MHz            │
│                                                  │
│ 说明:                                            │
│ W25Q16支持最高104MHz，但5.25MHz足够安全        │
│                                                  │
└──────────────────────────────────────────────────┘
```

**关键：关闭JTAG释放PB3**

```
菜单树: System Core → SYS

Debug 选项:
├─ ○ Disabled
├─ ○ SWD (Host-Device Interface)  ✓ (选中)
│  └─ 说明: 仅使用PA13(SWDIO)和PA14(SWCLK)
│  └─ 这样释放PB3使其可用于SPI时钟
└─ ○ Serial Wire and JTAG (会占用PB3)
```

**SPI1 引脚配置：**

```
在 Pinout 标签页:

PB3 → SPI1_SCK:
├─ 左键单击 PB3
├─ 弹出菜单: SPI1_SCK  ← 选中 (需要先关闭JTAG!)
├─ 如果无此选项，说明JTAG未关闭，返回上步骤
└─ ✓ 配置完成

PB4 → SPI1_MISO:
├─ 左键单击 PB4
├─ 弹出菜单: SPI1_MISO  ← 选中
└─ ✓ 配置完成

PB5 → SPI1_MOSI:
├─ 左键单击 PB5
├─ 弹出菜单: SPI1_MOSI  ← 选中
└─ ✓ 配置完成

PB0 → GPIO_Output (CS片选):
├─ 左键单击 PB0
├─ 弹出菜单: GPIO_Output  ← 选中
├─ PB0 变为黄色
└─ ✓ 配置完成

配置PB0属性:
├─ GPIO mode: Output Push-Pull
├─ GPIO level: High (CS默认高电平，无选中)
├─ GPIO speed: Medium
└─ GPIO Pull-up/Pull-down: No Pull
```

---

### 2.7 第七步：UART 配置 (调试串口)

```
菜单树: Pinout & Configuration → Connectivity → USART1
```

**参数配置：**

```
┌──────────────────────────────────────────────────┐
│ USART1 Configuration                             │
├──────────────────────────────────────────────────┤
│                                                  │
│ Mode: Asynchronous                               │
│                                                  │
│ Baud Rate: 115200  (标准调试波特率)             │
│ Word Length: 8 Bits                              │
│ Parity: None                                     │
│ Stop Bits: 1                                     │
│ Over Sampling: 16x                               │
│                                                  │
│ 说明:                                            │
│ 115200 bps 可达 11.52 KB/s，足够调试            │
│                                                  │
└──────────────────────────────────────────────────┘
```

**引脚配置：**

```
在 Pinout 标签页:

PA9 → USART1_TX:
├─ 左键单击 PA9
├─ 弹出菜单: USART1_TX  ← 选中
└─ ✓ 配置完成

PA10 → USART1_RX:
├─ 左键单击 PA10
├─ 弹出菜单: USART1_RX  ← 选中
└─ ✓ 配置完成
```

**中断配置：**

```
菜单树: System Core → NVIC

找到:
├─ USART1 global interrupt
│  └─ ☑ Enable
```

---

### 2.8 第八步：GPIO 配置 (按键和马达)

```
菜单树: Pinout & Configuration → GPIO
```

**PE1 和 PE2 (按键)：**

```
在 Pinout 标签页:

PE1 → GPIO_Input + EXTI:
├─ 左键单击 PE1
├─ 弹出菜单: GPIO_Input + Ext Interrupt  ← 选中
├─ PE1 变为红色，标注为 "GPIO_EXTI1"
└─ ✓ 配置完成

PE2 → GPIO_Input + EXTI:
├─ 左键单击 PE2
├─ 弹出菜单: GPIO_Input + Ext Interrupt  ← 选中
├─ PE2 变为红色，标注为 "GPIO_EXTI2"
└─ ✓ 配置完成
```

**PE1/PE2 属性配置：**

```
菜单树: Pinout & Configuration → GPIO → GPIOE

找到PE1和PE2行:

PE1:
├─ GPIO mode: Input
├─ GPIO Pull-up/Pull-down: No Pull (或 Pull-down)
└─ EXTI Line Trigger: Falling edge (按键低电平有效)

PE2:
├─ GPIO mode: Input
├─ GPIO Pull-up/Pull-down: No Pull (或 Pull-down)
└─ EXTI Line Trigger: Falling edge
```

**PD14 (马达方向)：**

```
在 Pinout 标签页:

PD14 → GPIO_Output:
├─ 左键单击 PD14
├─ 弹出菜单: GPIO_Output  ← 选中
├─ PD14 变为黄色
└─ ✓ 配置完成

配置PD14属性:
├─ GPIO mode: Output Push-Pull
├─ GPIO level: Low (初始正转)
├─ GPIO speed: Medium
└─ GPIO Pull-up/Pull-down: No Pull
```

**EXTI 中断配置：**

```
菜单树: System Core → NVIC

找到:
├─ EXTI line[1:0] interrupt (PE1)
│  └─ ☑ Enable
│
├─ EXTI line[9:5] interrupt (PE2属于此范围)
│  └─ ☑ Enable
```

---

### 2.9 第九步：调试器配置

```
菜单树: System Core → SYS

Debug:
├─ ○ Disabled
├─ ○ Serial Wire  ✓ (选中)
│  └─ 引脚: PA13(SWDIO), PA14(SWCLK)
└─ ○ Serial Wire and JTAG
   └─ 占用PB3, 不选
```

---

## 第三部分：内存布局可视化

### 3.1 Flash 内存分配

```
STM32F407VET6 Flash 总容量: 512 KB (0x08000000 - 0x08080000)

┌────────────────────────────────────┐
│ 0x08000000 - 0x08010000            │  FBL区域 (64 KB)
│                                    │
│  ┌──────────────────────────────┐  │
│  │ FBL代码                      │  │  包含:
│  │ ├─ 启动代码                  │  │  - 中断向量表
│  │ ├─ MCAL驱动 (CAN/Flash等)    │  │  - FBL逻辑
│  │ ├─ Flash擦写程序            │  │  - CRC校验
│  │ └─ 诊断服务 (0x34/0x36/0x37)│  │  - 安全验证
│  └──────────────────────────────┘  │
│                                    │
│ 预留: 0x08010000 分界线            │
├────────────────────────────────────┤
│ 0x08010000 - 0x08080000            │  APP区域 (448 KB)
│                                    │
│ ┌──────────────────────────────┐   │
│ │ APP有效性标志位 (4字节)      │   │  0x55AA55AA = 有效
│ │ 0x08010000: [0x55][0xAA][0x55]   │
│ │            [0xAA]             │   │
│ └──────────────────────────────┘   │
│                                    │
│ ┌──────────────────────────────┐   │
│ │ APP代码                      │   │  包含:
│ │ ├─ Simulink模型代码          │   │  - RTE接口
│ │ ├─ BSW层 (DCM/DEM等)         │   │  - 应用算法
│ │ ├─ 应用层 (Motor/LED等)      │   │  - 业务逻辑
│ │ └─ 常数和初始化数据          │   │
│ └──────────────────────────────┘   │
│                                    │
│ 已用: ~250KB (取决于代码量)        │
│ 剩余: ~198KB (可用扩展)            │
│                                    │
└────────────────────────────────────┘
```

### 3.2 RAM 内存分配

```
STM32F407VET6 RAM 总容量: 192 KB (0x20000000 - 0x20030000)

┌────────────────────────────────────┐
│ 0x20000000 - 0x20030000            │  SRAM (192 KB)
│                                    │
│ ┌──────────────────────────────┐   │
│ │ 数据段 (.data)               │   │  包含:
│ │ ├─ 全局变量初始值            │   │  - CAN缓冲区
│ │ ├─ 配置参数                  │   │  - COM信号缓冲
│ │ └─ 常数                      │   │  - DTC存储
│ │ 大小: ~20 KB                 │   │
│ └──────────────────────────────┘   │
│                                    │
│ ┌──────────────────────────────┐   │
│ │ 未初始化数据段 (.bss)        │   │  包含:
│ │ ├─ 全局数组 (缓冲区)         │   │  - NVM缓冲
│ │ ├─ 应用变量                  │   │  - 状态机变量
│ │ └─ 驱动内部状态              │   │  - 中断处理上下文
│ │ 大小: ~60 KB                 │   │
│ └──────────────────────────────┘   │
│                                    │
│ ┌──────────────────────────────┐   │
│ │ 堆 (Heap)                    │   │  可选，通常不用
│ │ 大小: 8 KB                   │   │
│ └──────────────────────────────┘   │
│                                    │
│ ┌──────────────────────────────┐   │
│ │ 栈 (Stack)                   │   │  包含:
│ │ 大小: 12 KB                  │   │  - 局部变量
│ │ 顶端: 0x2002FF00             │   │  - 函数返回地址
│ └──────────────────────────────┘   │  - 中断上下文
│ 栈顶 (SP): 0x20030000             │
│                                    │
│ 总使用: 100KB, 剩余: 92KB         │
│                                    │
└────────────────────────────────────┘

CCRAM (核心耦合RAM):
┌────────────────────────────────────┐
│ 0x10000000 - 0x10010000            │  CCRAM (64 KB)
│                                    │
│ 用途: 快速缓冲区或FBL临时存储      │
│                                    │
└────────────────────────────────────┘
```

### 3.3 NVM (外部Flash W25Q16) 分配

```
W25Q16 总容量: 2 MB (0x00000000 - 0x00200000)

推荐分配方案:

┌────────────────────────────────┐
│ 0x000000 - 0x001000 (4 KB)     │  系统配置区
│  ├─ 版本信息                   │
│  ├─ 序列号                     │
│  └─ 时间戳                     │
├────────────────────────────────┤
│ 0x001000 - 0x002000 (4 KB)     │  DID存储区 (数据标识)
│  ├─ F190: VIN (17 字节)        │
│  ├─ F200: 温度阈值 (2 字节)    │
│  ├─ F201: 作者名称             │
│  └─ F300: 公钥 (64 字节)       │
├────────────────────────────────┤
│ 0x002000 - 0x003000 (4 KB)     │  DTC存储区
│  ├─ 0x010001 按键卡滞          │
│  ├─ 0x020002 报文丢失          │
│  ├─ 0x030003 温度过高          │
│  ├─ 0x040004 按键短路          │
│  ├─ 0x050005 总线关闭          │
│  └─ 0x060006 CRC错误           │
├────────────────────────────────┤
│ 0x003000 - 0x004000 (4 KB)     │  计数器区 (刷写次数)
│  ├─ F501: 刷写计数器 (16bit)   │
│  │  当前值: 1000次 → 拒绝0x34   │
│  └─ 预留: 清零密钥存储         │
├────────────────────────────────┤
│ 0x004000 - 0x200000 (2MB-16KB) │  FBL数据区 (临时)
│  ├─ 固件包缓冲区               │
│  ├─ 固件签名验证               │
│  └─ 预留扩展                   │
└────────────────────────────────┘
```

---

## 第四部分：CubeMX生成代码及集成

### 4.1 生成代码

```
菜单: Project → Generate Code

生成完毕后:
├─ Core/Src/main.c        ← 更新
├─ Core/Src/stm32f4xx_it.c ← 更新
├─ Drivers/                ← 更新
└─ STM32F407VETx_FLASH.ld  ← 自动生成
```

### 4.2 核心修改

#### main.c 集成应用代码

```c
int main(void) {
    /* CubeMX 生成的初始化 (保留) */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_CAN1_Init();
    MX_ADC1_Init();
    MX_TIM4_Init();
    MX_SPI1_Init();
    MX_USART1_Init();
    
    /* 启动 PWM 和 ADC */
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
    HAL_ADC_Start(&hadc1);
    
    /* 你的应用初始化 (新增) */
    App_System_Init();
    App_TaskScheduler_Init();
    
    /* 主循环 */
    while (1) {
        /* 非RTOS轮询任务 */
        if (s_task10msFlag) {
            App_Task_10ms();
        }
        if (s_task100msFlag) {
            App_Task_100ms();
        }
        if (s_task1000msFlag) {
            App_Task_1000ms();
        }
    }
}
```

#### stm32f4xx_it.c 中断处理集成

```c
/* SysTick 中断: 1ms基准时钟 */
void SysTick_Handler(void) {
    HAL_IncTick();              // CubeMX保留
    App_SysTick_ISR();          // 应用层处理
    App_Watchdog_Feed();        // 喂狗
}

/* CAN RX0 中断 */
void CAN1_RX0_IRQHandler(void) {
    Mcal_Can_RxISR();           // 调用MCAL驱动
}

/* 外部中断: 按键 */
void EXTI1_IRQHandler(void) {   // PE1 (K0)
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
    App_Button_K0_Handler();
}

void EXTI2_IRQHandler(void) {   // PE2 (K1)
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
    App_Button_K1_Handler();
}

/* HardFault 异常处理 */
void HardFault_Handler(void) {
    printf("HardFault at PC=0x%08X\n", __get_PC());
    while (1) {
        App_Watchdog_Feed();    // 防止复位
    }
}
```

---

## 总结：完整配置顺序

```
步骤1: 新建工程
  └─ File → New → STM32 Project → STM32F407VET6

步骤2: 打开CubeMX
  └─ 双击 .ioc 文件

步骤3: 配置 (按此顺序)
  ① RCC       (时钟源 8MHz HSE + 32.768kHz LSE)
  ② Clock     (PLL配置 168MHz SYSCLK)
  ③ SYS       (SWD调试 + SysTick 1ms)
  ④ CAN1      (500kbps + PA8控制)
  ⑤ ADC1      (PE0温度采样)
  ⑥ TIM4      (LED和马达PWM)
  ⑦ SPI1      (W25Q16 Flash, 注意PB3)
  ⑧ USART1    (调试串口 115200)
  ⑨ GPIO      (PE1/PE2按键 + PD14马达方向)
  ⑩ NVIC      (所有中断使能)

步骤4: 生成代码
  └─ Project → Generate Code

步骤5: 集成应用
  └─ 修改main.c和stm32f4xx_it.c
```

所有参数和配置已在上文详细说明，按步骤执行即可完成硬件配置！
