# STM32F407 AUTOSAR 架构迁移完整指南

## 第一部分：DS18B20 单线协议驱动

### 1.1 DS18B20 通信原理

**单线协议 (One-Wire Protocol)：**

```
特点:
├─ 只需要一条数据线 (PE0)
├─ 信号电压: 0-3.3V (开漏输出)
├─ 通信速率: 9.6kbps-115.2kbps
├─ 需要外部上拉电阻 (4.7k ~ 10k)
└─ 工作温度范围: -55℃ ~ +125℃

时序:
├─ 初始化: MCU拉低至少480μs，然后释放，等待从设备响应
├─ 写"0": 拉低60μs，释放至少15μs
├─ 写"1": 拉低1-15μs，释放45-60μs
├─ 读"0": 检查拉低后15μs内的电平
└─ 读"1": 检查拉低后15μs内的电平
```

**主要命令：**

```
0x33: READ ROM              (读取64位ROM码)
0xCC: SKIP ROM              (跳过ROM搜索，单设备模式)
0x44: CONVERT T             (启动温度转换)
0xBE: READ SCRATCHPAD       (读取温度寄存器)
```

**温度转换公式：**

```
原始值 (16-bit):
Byte 0 (LSB): 位7-0 = 温度小数位 2^-4 ~ 2^-1
Byte 1 (MSB): 位7-0 = 温度整数位

实际温度 = (MSB << 8 | LSB) >> 4

例:
原始值: 0x016F (367)
温度 = 367 / 16 = 22.9375℃

原始值: 0xFF90 (负温度)
温度 = -16.0℃ (符号扩展)
```

---

### 1.2 MCAL 层 - DS18B20 驱动实现

**文件：src/mcal/ds18b20/ds18b20_driver.c**

```c
/**
 * @file ds18b20_driver.c
 * @brief DS18B20 单线温度传感器驱动
 * @version 1.0.0
 */

#include "main.h"
#include "ds18b20_driver.h"
#include "string.h"

/* DS18B20 命令定义 */
#define DS18B20_CMD_READ_ROM        0x33
#define DS18B20_CMD_SKIP_ROM        0xCC
#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_READ_SCRATCH    0xBE

/* 时序定义 (单位: μs) */
#define DS18B20_INIT_LOW_TIME       480
#define DS18B20_INIT_RELEASE_TIME   70
#define DS18B20_WRITE_0_LOW_TIME    60
#define DS18B20_WRITE_1_LOW_TIME    5
#define DS18B20_READ_WAIT_TIME      10
#define DS18B20_READ_TIMEOUT        100

/* DS18B20 驱动状态 */
typedef struct {
    uint8_t initialized;
    int16_t last_temp_raw;
    float last_temp_celsius;
    uint8_t conversion_in_progress;
    uint32_t last_conversion_time;
} DS18B20_StateType;

static DS18B20_StateType ds18b20_state = {0};

/**
 * @brief 微秒延迟函数
 * @param[in] us 延迟时间 (微秒)
 */
static void Mcal_DS18B20_DelayUs(uint32_t us)
{
    /* 使用MCU的定时器或busy-wait实现 */
    uint32_t start = HAL_GetTick() * 1000;  /* 转换为微秒 */
    while ((HAL_GetTick() * 1000 - start) < us) {
        /* busy wait */
    }
}

/**
 * @brief 设置PE0为输入模式 (读操作)
 */
static void Mcal_DS18B20_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/**
 * @brief 设置PE0为输出模式 (写操作)
 */
static void Mcal_DS18B20_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  /* 开漏输出 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/**
 * @brief 拉低PE0引脚
 */
static void Mcal_DS18B20_PullLow(void)
{
    Mcal_DS18B20_SetOutput();
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);  /* 拉低 */
}

/**
 * @brief 释放PE0引脚 (上拉电阻拉高)
 */
static void Mcal_DS18B20_Release(void)
{
    Mcal_DS18B20_SetOutput();
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);    /* 释放，由上拉电阻拉高 */
}

/**
 * @brief 读取PE0引脚电平
 * @return GPIO_PIN_SET (高电平) 或 GPIO_PIN_RESET (低电平)
 */
static GPIO_PinState Mcal_DS18B20_ReadPin(void)
{
    Mcal_DS18B20_SetInput();
    return HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_0);
}

/**
 * @brief DS18B20 初始化 (复位时序)
 * @return Std_ReturnType
 *   @retval STD_OK    芯片响应成功
 *   @retval STD_NOT_OK 未检测到芯片
 */
Std_ReturnType Mcal_DS18B20_Init(void)
{
    /* 步骤1: 拉低至少480μs */
    Mcal_DS18B20_PullLow();
    Mcal_DS18B20_DelayUs(DS18B20_INIT_LOW_TIME);
    
    /* 步骤2: 释放引脚，等待从设备拉低 (从设备响应) */
    Mcal_DS18B20_Release();
    Mcal_DS18B20_DelayUs(30);  /* 等待30μs */
    
    /* 步骤3: 检查PE0是否被拉低 (从设备响应) */
    GPIO_PinState level = Mcal_DS18B20_ReadPin();
    
    if (level == GPIO_PIN_RESET) {
        /* 检测到从设备响应 */
        Mcal_DS18B20_DelayUs(DS18B20_INIT_RELEASE_TIME);
        ds18b20_state.initialized = 1;
        return STD_OK;
    } else {
        /* 未检测到从设备 */
        ds18b20_state.initialized = 0;
        return STD_NOT_OK;
    }
}

/**
 * @brief 写一个字节
 * @param[in] byte 要写入的字节
 */
static void Mcal_DS18B20_WriteByte(uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        if (byte & 0x01) {
            /* 写"1": 拉低1-15μs，释放45-60μs */
            Mcal_DS18B20_PullLow();
            Mcal_DS18B20_DelayUs(5);
            Mcal_DS18B20_Release();
            Mcal_DS18B20_DelayUs(55);
        } else {
            /* 写"0": 拉低60μs，释放至少15μs */
            Mcal_DS18B20_PullLow();
            Mcal_DS18B20_DelayUs(60);
            Mcal_DS18B20_Release();
            Mcal_DS18B20_DelayUs(15);
        }
        byte >>= 1;
    }
}

/**
 * @brief 读一个字节
 * @return 读取的字节
 */
static uint8_t Mcal_DS18B20_ReadByte(void)
{
    uint8_t byte = 0;
    
    for (int i = 0; i < 8; i++) {
        /* 拉低1μs后释放 */
        Mcal_DS18B20_PullLow();
        Mcal_DS18B20_DelayUs(1);
        Mcal_DS18B20_Release();
        
        /* 等待10μs后读取电平 */
        Mcal_DS18B20_DelayUs(DS18B20_READ_WAIT_TIME);
        
        if (Mcal_DS18B20_ReadPin() == GPIO_PIN_SET) {
            byte |= (0x80 >> i);  /* 读到"1" */
        }
        
        /* 完成时隙，至少48μs */
        Mcal_DS18B20_DelayUs(40);
    }
    
    return byte;
}

/**
 * @brief 启动温度转换
 * @return Std_ReturnType
 */
Std_ReturnType Mcal_DS18B20_StartConversion(void)
{
    if (!ds18b20_state.initialized) {
        if (Mcal_DS18B20_Init() != STD_OK) {
            return STD_NOT_OK;
        }
    }
    
    /* 发送跳过ROM命令 + 转换命令 */
    Mcal_DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);      /* 0xCC */
    Mcal_DS18B20_WriteByte(DS18B20_CMD_CONVERT_T);     /* 0x44 */
    
    ds18b20_state.conversion_in_progress = 1;
    ds18b20_state.last_conversion_time = HAL_GetTick();
    
    return STD_OK;
}

/**
 * @brief 读取温度原始值 (13位)
 * @param[out] temp_raw 温度原始值指针
 * @return Std_ReturnType
 */
Std_ReturnType Mcal_DS18B20_ReadTemperatureRaw(int16_t *temp_raw)
{
    uint8_t scratchpad[9];
    
    if (!ds18b20_state.initialized) {
        return STD_NOT_OK;
    }
    
    /* 初始化 */
    if (Mcal_DS18B20_Init() != STD_OK) {
        return STD_NOT_OK;
    }
    
    /* 发送读暂存器命令 */
    Mcal_DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);      /* 0xCC */
    Mcal_DS18B20_WriteByte(DS18B20_CMD_READ_SCRATCH);  /* 0xBE */
    
    /* 读取9字节暂存器 */
    for (int i = 0; i < 9; i++) {
        scratchpad[i] = Mcal_DS18B20_ReadByte();
    }
    
    /* 提取温度值 (Byte0[7:4] + Byte1[7:0]) */
    int16_t raw = (scratchpad[1] << 8) | scratchpad[0];
    
    ds18b20_state.last_temp_raw = raw;
    *temp_raw = raw;
    
    return STD_OK;
}

/**
 * @brief 将原始值转换为摄氏度
 * @param[in] temp_raw 温度原始值
 * @return 温度 (℃)
 */
static float Mcal_DS18B20_RawToCelsius(int16_t temp_raw)
{
    /* 温度 = raw / 16 (每个单位代表0.0625℃) */
    float celsius = (float)temp_raw / 16.0f;
    return celsius;
}

/**
 * @brief 读取温度并转换为℃
 * @param[out] temperature 温度指针 (℃)
 * @return Std_ReturnType
 */
Std_ReturnType Mcal_DS18B20_ReadTemperature(float *temperature)
{
    int16_t temp_raw;
    
    if (Mcal_DS18B20_ReadTemperatureRaw(&temp_raw) != STD_OK) {
        return STD_NOT_OK;
    }
    
    ds18b20_state.last_temp_celsius = Mcal_DS18B20_RawToCelsius(temp_raw);
    
    if (temperature) {
        *temperature = ds18b20_state.last_temp_celsius;
    }
    
    return STD_OK;
}

/**
 * @brief 获取最后一次读取的温度
 * @return 温度 (℃)
 */
float Mcal_DS18B20_GetLastTemperature(void)
{
    return ds18b20_state.last_temp_celsius;
}

/**
 * @brief 获取初始化状态
 * @return 初始化状态
 */
uint8_t Mcal_DS18B20_IsInitialized(void)
{
    return ds18b20_state.initialized;
}
```

**文件：inc/mcal/ds18b20/ds18b20_driver.h**

```c
/**
 * @file ds18b20_driver.h
 * @brief DS18B20 单线温度传感器驱动头文件
 */

#ifndef __DS18B20_DRIVER_H
#define __DS18B20_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* 函数声明 */
Std_ReturnType Mcal_DS18B20_Init(void);
Std_ReturnType Mcal_DS18B20_StartConversion(void);
Std_ReturnType Mcal_DS18B20_ReadTemperatureRaw(int16_t *temp_raw);
Std_ReturnType Mcal_DS18B20_ReadTemperature(float *temperature);
float Mcal_DS18B20_GetLastTemperature(void);
uint8_t Mcal_DS18B20_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* __DS18B20_DRIVER_H */
```

---

## 第二部分：AUTOSAR 分层架构完整迁移

### 2.1 项目目录结构（最终版）

```
STM32F407_ECU_Project/
│
├── src/
│   ├── mcal/                    # MCAL 微控制器抽象层
│   │   ├── can/
│   │   │   ├── can_driver.c
│   │   │   └── can_interrupt.c
│   │   ├── uart/
│   │   │   ├── uart_driver.c
│   │   │   └── uart_debug.c
│   │   ├── spi/
│   │   │   ├── spi_driver.c
│   │   │   └── spi_flash.c
│   │   ├── timer/
│   │   │   ├── timer_driver.c
│   │   │   └── timer_systick.c
│   │   ├── pwm/
│   │   │   ├── pwm_driver.c
│   │   │   ├── pwm_led.c
│   │   │   └── pwm_motor.c
│   │   ├── gpio/
│   │   │   ├── gpio_driver.c
│   │   │   └── gpio_key.c
│   │   ├── ds18b20/              # 新增：DS18B20驱动
│   │   │   └── ds18b20_driver.c
│   │   ├── nvm/
│   │   │   ├── flash_driver.c
│   │   │   └── eeprom_driver.c
│   │   └── clock/
│   │       └── clock_driver.c
│   │
│   ├── bsw/                     # BSW 基础软件层
│   │   ├── com/                 # 通信管理
│   │   │   ├── com_manager.c
│   │   │   ├── com_signal_encode.c
│   │   │   └── com_signal_decode.c
│   │   ├── canif/               # CAN接口
│   │   │   ├── canif_driver.c
│   │   │   ├── canif_handler.c
│   │   │   └── canif_tx_buffer.c
│   │   ├── dcm/                 # 诊断服务
│   │   │   ├── dcm_main.c
│   │   │   ├── dcm_service_0x22.c
│   │   │   ├── dcm_service_0x2E.c
│   │   │   ├── dcm_service_0x27.c
│   │   │   └── dcm_security.c
│   │   ├── dem/                 # 故障管理
│   │   │   ├── dem_main.c
│   │   │   ├── dem_monitor.c
│   │   │   ├── dem_dtc_manager.c
│   │   │   └── dem_temp_monitor.c
│   │   ├── cannm/               # 网络管理
│   │   │   ├── cannm_main.c
│   │   │   ├── cannm_sleep.c
│   │   │   └── cannm_wakeup.c
│   │   ├── nvm/                 # NVM管理
│   │   │   ├── nvm_manager.c
│   │   │   └── nvm_scheduler.c
│   │   └── pdur/                # PDU路由
│   │       └── pdur_router.c
│   │
│   ├── rte/                     # RTE 运行时环境
│   │   ├── rte_main.c
│   │   ├── rte_interface.c
│   │   └── rte_scheduler.c
│   │
│   ├── asw/                     # ASW 应用软件层
│   │   ├── app_motor_control.c
│   │   ├── app_led_control.c
│   │   ├── app_temp_monitor.c    # 温度监控应用
│   │   └── app_key_handler.c
│   │
│   ├── app/                     # 应用主程序
│   │   ├── main.c
│   │   ├── app_init.c
│   │   ├── app_task_scheduler.c
│   │   └── app_systick_handler.c
│   │
│   ├── fbl/                     # FBL Bootloader
│   │   ├── fbl_main.c
│   │   ├── fbl_flash_driver.c
│   │   └── fbl_dcm_service.c
│   │
│   └── config/                  # 配置文件
│       ├── can_matrix.c
│       ├── dcm_config.c
│       ├── dem_config.c
│       ├── pin_config.c
│       └── system_config.c
│
├── inc/                         # 头文件（目录结构同src）
│   ├── mcal/...
│   ├── bsw/...
│   ├── rte/...
│   ├── asw/...
│   ├── app/...
│   ├── fbl/...
│   └── config/...
│
├── Core/                        # CubeIDE生成的代码
│   ├── Src/
│   │   ├── main.c               # 已修改：集成AUTOSAR初始化
│   │   └── stm32f4xx_it.c        # 已修改：中断处理
│   └── Inc/
│       └── main.h
│
├── CMakeLists.txt               # 编译配置（支持APP/FBL双编译）
├── .gitignore
└── README.md
```

---

### 2.2 迁移步骤详解

#### Step 1: 从测试代码提取MCAL驱动

**test_can.c → src/mcal/can/can_driver.c**

```c
/* 保留关键的HAL封装和缓冲区管理 */
/* 删除测试打印和模式切换逻辑 */

Std_ReturnType Mcal_Can_Init(const Can_ConfigType *Config)
{
    /* 调用HAL库初始化CAN */
    /* 配置过滤器 */
    /* 启动CAN和中断 */
    return STD_OK;
}

Std_ReturnType Mcal_Can_Send(uint32_t CanId, const uint8_t *Data, uint8_t Dlc)
{
    /* 直接调用HAL_CAN_AddTxMessage */
}

void Mcal_Can_RxISR(void)
{
    /* CAN接收中断处理 */
    /* 调用BSW层回调函数 */
}
```

#### Step 2: 添加BSW层Com通信管理

**src/bsw/com/com_manager.c**

```c
/**
 * @brief Com初始化
 */
void Bsw_Com_Init(void)
{
    /* 初始化信号缓冲区 */
    /* 初始化报文定时器 */
}

/**
 * @brief Com主处理函数 (由RTE调用)
 */
void Bsw_Com_MainFunction(void)
{
    /* 处理接收的报文 */
    /* 处理发送队列 */
    /* 调用信号处理函数 */
}

/**
 * @brief CAN RX回调 (由MCAL调用)
 */
void Bsw_Com_RxIndication(uint32_t CanId, const uint8_t *Data, uint8_t Dlc)
{
    /* 解析报文 */
    /* 更新信号值 */
    /* 调用RTE */
}
```

#### Step 3: 添加BSW层故障管理（DEM）

**src/bsw/dem/dem_temp_monitor.c**

```c
/**
 * @brief 温度过高监控
 * @param[in] temperature 当前温度 (℃)
 */
void Bsw_Dem_TemperatureMonitor(float temperature)
{
    static uint32_t high_temp_counter = 0;
    
    /* 配置：温度阈值 = 80℃，回差 = 2℃ */
    #define TEMP_THRESHOLD_HIGH  80.0f
    #define TEMP_THRESHOLD_LOW   78.0f
    
    if (temperature > TEMP_THRESHOLD_HIGH) {
        high_temp_counter++;
        
        if (high_temp_counter >= 10) {  /* 1秒持续高温 (100ms×10) */
            /* 设置DTC 0x030003 */
            Bsw_Dem_SetEventStatus(0x030003, DEM_EVENT_STATUS_PASSED);
            
            /* 触发应用响应 (关闭马达或降速) */
            App_TempHighAction();
        }
    } else if (temperature < TEMP_THRESHOLD_LOW) {
        /* 温度回到安全范围，恢复正常 */
        high_temp_counter = 0;
        Bsw_Dem_SetEventStatus(0x030003, DEM_EVENT_STATUS_FAILED);
    }
}
```

#### Step 4: 添加RTE层接口

**src/rte/rte_interface.c**

```c
/**
 * @brief RTE读取温度值 (应用调用)
 */
Std_ReturnType Rte_Read_TempSensor_Temperature(float *Temperature)
{
    /* 从缓冲区读取最新温度值 */
    if (Temperature == NULL) {
        return STD_NOT_OK;
    }
    
    *Temperature = g_temp_sensor_data.temperature;
    return STD_OK;
}

/**
 * @brief RTE写入LED亮度 (应用调用)
 */
Std_ReturnType Rte_Write_LedControl_Brightness(uint8_t Brightness)
{
    /* 设置LED PWM占空比 */
    /* 调用MCAL */
    Mcal_Pwm_SetDuty(LED_PWM_CHANNEL, Brightness);
    return STD_OK;
}

/**
 * @brief RTE写入马达命令 (应用调用)
 */
Std_ReturnType Rte_Write_MotorControl_Command(uint8_t Command)
{
    /* 设置马达速度和方向 */
    Mcal_Pwm_SetDuty(MOTOR_PWM_CHANNEL, Command & 0x7F);
    Mcal_Gpio_WritePin(MOTOR_DIR_PORT, MOTOR_DIR_PIN, (Command >> 7) & 0x01);
    return STD_OK;
}
```

#### Step 5: 添加ASW层应用逻辑

**src/asw/app_temp_monitor.c**

```c
/**
 * @brief 温度监控应用 (100ms周期)
 */
void App_TempMonitor_100ms(void)
{
    float current_temp;
    
    /* 读取DS18B20温度 */
    Mcal_DS18B20_ReadTemperature(&current_temp);
    
    /* 通过RTE读取 (如果有缓存) */
    Rte_Read_TempSensor_Temperature(&current_temp);
    
    /* 调用BSW故障监控 */
    Bsw_Dem_TemperatureMonitor(current_temp);
    
    /* 应用自己的逻辑 */
    if (current_temp > 85.0f) {
        /* 危险温度，强制停止马达 */
        Rte_Write_MotorControl_Command(0);
    }
}

/**
 * @brief 温度过高时的应用响应
 */
void App_TempHighAction(void)
{
    /* 关闭LED (或设置告警) */
    Rte_Write_LedControl_Brightness(100);  /* 亮度100%=告警 */
    
    /* 降速或停止马达 */
    Rte_Write_MotorControl_Command(50);    /* 50%速度 */
    
    /* 发送诊断消息到VCU */
    uint8_t diag_msg[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    Mcal_Can_Send(0x641, diag_msg, 8);
}
```

---

### 2.3 任务调度重新设计

**src/app/app_task_scheduler.c**

```c
/**
 * @brief AUTOSAR兼容的任务调度器
 */

/* 任务周期定义 */
#define TASK_10MS_PERIOD    10      /* ms */
#define TASK_100MS_PERIOD   100     /* ms */
#define TASK_1000MS_PERIOD  1000    /* ms */

static uint32_t g_10ms_counter = 0;
static uint32_t g_100ms_counter = 0;
static uint32_t g_1000ms_counter = 0;

/**
 * @brief 1ms系统时钟 (由SysTick中断调用)
 */
void HAL_SYSTICK_Callback(void)
{
    g_10ms_counter++;
    g_100ms_counter++;
    g_1000ms_counter++;
}

/**
 * @brief 10ms周期任务 (高速任务)
 */
void App_Task_10ms(void)
{
    if (g_10ms_counter >= TASK_10MS_PERIOD) {
        g_10ms_counter = 0;
        
        /* MCAL: 底层采样 */
        Mcal_Gpio_ReadInput();      /* 读按键原始电平 */
        
        /* RTE: 信号接收 */
        Rte_MainFunction_RxPath();  /* 处理接收的报文 */
        
        /* ASW: 应用处理 */
        App_KeyDebounce();          /* 按键去抖 */
        App_SignalFilter();         /* 信号滤波 */
    }
}

/**
 * @brief 100ms周期任务 (中速任务)
 */
void App_Task_100ms(void)
{
    if (g_100ms_counter >= TASK_100MS_PERIOD) {
        g_100ms_counter = 0;
        
        /* Com: 信号处理 */
        Bsw_Com_MainFunction();     /* 处理通信信号 */
        
        /* DEM: 故障检测 */
        Bsw_Dem_MainFunction();     /* 故障监控逻辑 */
        
        /* ASW: 应用控制 */
        App_TempMonitor_100ms();    /* 温度监控 */
        App_MotorControl_100ms();   /* 马达控制 */
    }
}

/**
 * @brief 1000ms周期任务 (低速任务)
 */
void App_Task_1000ms(void)
{
    if (g_1000ms_counter >= TASK_1000MS_PERIOD) {
        g_1000ms_counter = 0;
        
        /* NVM: 数据存储 */
        Bsw_Nvm_MainFunction();     /* 异步NVM写入 */
        
        /* DCM: 诊断处理 */
        Bsw_Dcm_MainFunction();     /* 诊断服务处理 */
        
        /* CanNm: 网络管理 */
        Bsw_CanNm_MainFunction();   /* 网络管理报文 */
        
        /* ASW: 日志和统计 */
        App_LoggingTask();          /* 系统日志 */
    }
}

/**
 * @brief 主循环 (非RTOS环境)
 */
void App_MainLoop(void)
{
    while (1) {
        App_Task_10ms();
        App_Task_100ms();
        App_Task_1000ms();
        
        /* 喂狗 */
        HAL_IWDG_Refresh(&hiwdg);
    }
}
```

---

### 2.4 AUTOSAR配置文件

**src/config/system_config.c**

```c
/**
 * @brief AUTOSAR系统配置
 */

#include "types.h"
#include "system_config.h"

/* CAN配置 */
const Can_ConfigType g_can1_config = {
    .baudrate = 500,        /* 500kbps */
    .channel = 0,           /* CAN1 */
    .rxFilterMode = 0,      /* 接收所有消息 */
};

/* COM信号配置 */
const Com_SignalConfig g_com_signals[] = {
    /* 接收信号 */
    {
        .id = 0x210,        /* Vehicle_Ctrl */
        .signal_name = "Motor_Cmd",
        .dlc = 8,
        .is_tx = FALSE,
    },
    
    /* 发送信号 */
    {
        .id = 0x1A0,        /* ECU_Status */
        .signal_name = "Temp_Celsius",
        .dlc = 8,
        .is_tx = TRUE,
    },
};

/* DEM配置 */
const Dem_DtcConfig g_dem_dtc_config[] = {
    {
        .dtc_id = 0x030003,
        .event_name = "TemperatureHigh",
        .debounce_time = 100,    /* 100ms */
        .threshold = 80,         /* 80℃ */
        .recovery_offset = 2,    /* 2℃回差 */
    },
};

const uint8_t g_dem_dtc_count = sizeof(g_dem_dtc_config) / sizeof(g_dem_dtc_config[0]);
```

---

### 2.5 主程序集成

**Core/Src/main.c 修改部分**

```c
/* USER CODE BEGIN Includes */
#include "ds18b20_driver.h"
#include "can_driver.h"
#include "com_manager.h"
#include "dem_monitor.h"
#include "rte_interface.h"
#include "app_task_scheduler.h"
#include "stdio.h"
/* USER CODE END Includes */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    /* 初始化所有外设 */
    MX_GPIO_Init();
    MX_CAN1_Init();
    MX_SPI1_Init();
    MX_TIM4_Init();
    MX_USART1_UART_Init();
    
    /* USER CODE BEGIN 2 */
    
    /* 启动PWM */
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
    
    /* MCAL初始化 */
    printf("[MCAL] 初始化DS18B20...\n");
    if (Mcal_DS18B20_Init() == STD_OK) {
        printf("[MCAL] DS18B20初始化成功\n");
    } else {
        printf("[MCAL] DS18B20初始化失败\n");
    }
    
    printf("[MCAL] 初始化CAN1...\n");
    Can_ConfigType can_config = {
        .baudrate = 500,
        .channel = 0,
        .rxFilterMode = 0,
    };
    Mcal_Can_Init(&can_config);
    
    /* BSW初始化 */
    printf("[BSW] 初始化Com...\n");
    Bsw_Com_Init();
    
    printf("[BSW] 初始化DEM...\n");
    Bsw_Dem_Init();
    
    printf("[BSW] 初始化DCM...\n");
    Bsw_Dcm_Init();
    
    printf("[BSW] 初始化CanNm...\n");
    Bsw_CanNm_Init();
    
    /* RTE初始化 */
    printf("[RTE] 初始化RTE...\n");
    Rte_Init();
    
    /* 应用初始化 */
    printf("[APP] 初始化应用层...\n");
    App_System_Init();
    
    printf("\n[SYSTEM] AUTOSAR ECU启动完成！\n");
    
    /* USER CODE END 2 */
    
    /* 主循环 */
    while (1) {
        App_Task_10ms();
        App_Task_100ms();
        App_Task_1000ms();
        
        /* 喂狗 */
        HAL_IWDG_Refresh(&hiwdg);
    }
}
```

---

## 第三部分：编译和验证

### 3.1 CMakeLists.txt 更新

```cmake
cmake_minimum_required(VERSION 3.10)
project(STM32F407_ECU_AUTOSAR C ASM)

# 工具链配置
set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_C_FLAGS "-mcpu=cortex-m4 -mthumb -Wall -Wextra -Wpedantic -O2 -g3")

# 包含路径
include_directories(
    Core/Inc
    inc
    inc/mcal
    inc/bsw
    inc/rte
    inc/asw
    inc/app
    inc/config
)

# 源文件列表
file(GLOB_RECURSE MCAL_SOURCES "src/mcal/*.c")
file(GLOB_RECURSE BSW_SOURCES "src/bsw/*.c")
file(GLOB_RECURSE RTE_SOURCES "src/rte/*.c")
file(GLOB_RECURSE ASW_SOURCES "src/asw/*.c")
file(GLOB_RECURSE APP_SOURCES "src/app/*.c")
file(GLOB_RECURSE CONFIG_SOURCES "src/config/*.c")
file(GLOB_RECURSE CORE_SOURCES "Core/Src/*.c")

# 编译目标
add_executable(firmware_app.elf
    ${MCAL_SOURCES}
    ${BSW_SOURCES}
    ${RTE_SOURCES}
    ${ASW_SOURCES}
    ${APP_SOURCES}
    ${CONFIG_SOURCES}
    ${CORE_SOURCES}
)

# 链接
target_link_libraries(firmware_app.elf m)

# Post-build
add_custom_command(TARGET firmware_app.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex firmware_app.elf firmware_app.hex
    COMMAND arm-none-eabi-objcopy -O binary firmware_app.elf firmware_app.bin
    COMMAND arm-none-eabi-size firmware_app.elf
)
```

### 3.2 编译命令

```bash
mkdir -p build && cd build
cmake ..
make -j4

# 烧写
arm-none-eabi-gdb firmware_app.elf
(gdb) target remote localhost:4242
(gdb) load
(gdb) continue
```

---

## 第四部分：预期输出和验证

启动后，串口应该输出：

```
[MCAL] 初始化DS18B20...
[MCAL] DS18B20初始化成功
[MCAL] 初始化CAN1...
[BSW] 初始化Com...
[BSW] 初始化DEM...
[BSW] 初始化DCM...
[BSW] 初始化CanNm...
[RTE] 初始化RTE...
[APP] 初始化应用层...

[SYSTEM] AUTOSAR ECU启动完成！

[100ms] 温度: 25.3℃
[100ms] DEM状态检查: OK
[100ms] 马达状态: 正常

[1000ms] NVM数据写入
[1000ms] 网络管理报文发送
```

---

## 第五部分：迁移清单

```
MCAL层 (微控制器抽象层)
☐ DS18B20驱动 (单线协议)
☐ CAN驱动 (收发和缓冲)
☐ SPI Flash驱动 (W25Q16)
☐ PWM驱动 (LED和马达)
☐ GPIO驱动 (按键)
☐ UART驱动 (调试)
☐ Timer驱动 (系统时钟)

BSW层 (基础软件层)
☐ Com通信管理 (报文打包/解包)
☐ CanIf接口 (CAN抽象)
☐ DEM故障管理 (DTC和监控)
☐ Dem温度监控 (使用DS18B20)
☐ DCM诊断服务 (0x22/0x2E等)
☐ CanNm网络管理 (休眠/唤醒)
☐ NVM存储管理 (DID/DTC)

RTE层 (运行时环境)
☐ Rte_Read接口 (读信号)
☐ Rte_Write接口 (写信号)
☐ 信号缓冲管理
☐ 任务调度

ASW层 (应用软件层)
☐ 温度监控应用
☐ 马达控制应用
☐ LED控制应用
☐ 按键处理应用

应用主程序
☐ 主循环实现
☐ 任务调度器
☐ 中断处理
☐ 系统初始化

编译和测试
☐ CMakeLists.txt配置
☐ 编译无错误
☐ 烧写验证
☐ 功能测试
```

---

**AUTOSAR架构迁移完成！现在你有了一套专业的分层架构，可以轻松扩展和维护。** 🎉

