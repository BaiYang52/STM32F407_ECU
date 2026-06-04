# DS18B20 + AUTOSAR 快速集成指南

## 第一部分：DS18B20 驱动核心实现

### 1.1 创建必要的头文件

**文件：inc/types.h（标准类型定义）**

```c
/**
 * @file types.h
 * @brief AUTOSAR标准类型定义
 */

#ifndef __TYPES_H
#define __TYPES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 标准返回值类型 */
typedef uint8_t Std_ReturnType;

#define STD_OK      0x00U
#define STD_NOT_OK  0x01U

/* 布尔类型 */
typedef uint8_t boolean;
#define TRUE        1U
#define FALSE       0U

/* 标准整数类型 */
typedef int8_t      sint8;
typedef uint8_t     uint8;
typedef int16_t     sint16;
typedef uint16_t    uint16;
typedef int32_t     sint32;
typedef uint32_t    uint32;
typedef int64_t     sint64;
typedef uint64_t    uint64;

/* 浮点类型 */
typedef float       float32;
typedef double      float64;

#ifdef __cplusplus
}
#endif

#endif /* __TYPES_H */
```

**文件：inc/mcal/ds18b20/ds18b20_driver.h**

```c
/**
 * @file ds18b20_driver.h
 * @brief DS18B20 单线温度传感器驱动头文件
 * @version 1.0.0
 * 
 * 硬件接线:
 * ├─ DQ (PE0)  → STM32 PE0 (开漏输出)
 * ├─ GND       → GND
 * ├─ VCC       → +3.3V (或+5V)
 * └─ 上拉电阻  → 4.7k ~ 10k Ω (连接DQ和VCC)
 */

#ifndef __DS18B20_DRIVER_H
#define __DS18B20_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* ==================== 函数声明 ==================== */

/**
 * @brief DS18B20初始化和复位
 * @details 执行复位时序，检测设备是否存在
 * @return 
 *   @retval STD_OK    检测到DS18B20
 *   @retval STD_NOT_OK 未检测到设备
 */
Std_ReturnType Mcal_DS18B20_Init(void);

/**
 * @brief 启动温度转换
 * @details 发送转换命令，设备开始测量温度
 * @return 
 *   @retval STD_OK    命令发送成功
 *   @retval STD_NOT_OK 发送失败
 * 
 * 转换时间:
 *   12位精度: 最长750ms
 *   推荐轮询周期: 100ms (足够用于多次采样)
 */
Std_ReturnType Mcal_DS18B20_StartConversion(void);

/**
 * @brief 读取温度原始值 (16位)
 * @param[out] TemperatureRaw 温度原始值指针
 * @return 
 *   @retval STD_OK    读取成功
 *   @retval STD_NOT_OK 读取失败
 * 
 * 原始值说明:
 *   Byte1[7:0] | Byte0[7:4] 组成13位温度值
 *   每个LSB代表1/16℃ (0.0625℃)
 *   范围: -55℃ ~ +125℃
 */
Std_ReturnType Mcal_DS18B20_ReadTemperatureRaw(int16_t *TemperatureRaw);

/**
 * @brief 读取温度并转换为摄氏度
 * @param[out] Temperature 温度指针 (单位: ℃)
 * @return 
 *   @retval STD_OK    读取成功
 *   @retval STD_NOT_OK 读取失败
 */
Std_ReturnType Mcal_DS18B20_ReadTemperature(float *Temperature);

/**
 * @brief 获取最后一次读取的温度值（不重新读取）
 * @return 温度值 (℃)
 */
float Mcal_DS18B20_GetLastTemperature(void);

/**
 * @brief 获取初始化状态
 * @return 初始化状态 (1=已初始化, 0=未初始化)
 */
uint8_t Mcal_DS18B20_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* __DS18B20_DRIVER_H */
```

---

### 1.2 DS18B20 驱动实现

**文件：src/mcal/ds18b20/ds18b20_driver.c**

```c
/**
 * @file ds18b20_driver.c
 * @brief DS18B20 单线温度传感器驱动实现
 * @version 1.0.0
 */

#include "main.h"
#include "ds18b20_driver.h"
#include "stdio.h"

/* ==================== 时序定义 ==================== */

#define DS18B20_PORT            GPIOE
#define DS18B20_PIN             GPIO_PIN_0

/* DS18B20命令 */
#define DS18B20_CMD_READ_ROM        0x33  /* 读ROM码 */
#define DS18B20_CMD_SKIP_ROM        0xCC  /* 跳过ROM搜索 */
#define DS18B20_CMD_CONVERT_T       0x44  /* 启动温度转换 */
#define DS18B20_CMD_READ_SCRATCH    0xBE  /* 读暂存器 */

/* 时序时间 (单位: μs) */
#define DS18B20_INIT_LOW_TIME       480   /* 初始化拉低时间 */
#define DS18B20_INIT_RELEASE_TIME   100   /* 初始化释放后等待时间 */

/* ==================== 驱动状态结构体 ==================== */

typedef struct {
    uint8_t initialized;
    int16_t last_temp_raw;
    float last_temp_celsius;
} DS18B20_StateType;

static DS18B20_StateType ds18b20_state = {
    .initialized = 0,
    .last_temp_raw = 0,
    .last_temp_celsius = 0.0f,
};

/* ==================== 内部函数实现 ==================== */

/**
 * @brief 微秒级延迟 (近似)
 * @note 使用简单的循环延迟，不够精确但可用于单线协议
 */
static void Mcal_DS18B20_DelayUs(uint32_t us)
{
    volatile uint32_t count = us * 15;  /* 在168MHz下的近似值 */
    while (count--) {
        __NOP();
    }
}

/**
 * @brief 设置PE0为输入模式 (释放总线)
 */
static void Mcal_DS18B20_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

/**
 * @brief 设置PE0为输出模式 (开漏输出)
 */
static void Mcal_DS18B20_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  /* 开漏 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

/**
 * @brief 拉低PE0 (驱动线为低)
 */
static void Mcal_DS18B20_PullLow(void)
{
    Mcal_DS18B20_SetOutput();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
}

/**
 * @brief 释放PE0 (让上拉电阻拉高)
 */
static void Mcal_DS18B20_Release(void)
{
    Mcal_DS18B20_SetOutput();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET);
}

/**
 * @brief 读取PE0电平
 * @return GPIO_PIN_SET (高) 或 GPIO_PIN_RESET (低)
 */
static GPIO_PinState Mcal_DS18B20_ReadPin(void)
{
    Mcal_DS18B20_SetInput();
    return HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN);
}

/* ==================== 公开函数实现 ==================== */

/**
 * @brief DS18B20初始化
 * @details 执行复位时序，检测设备是否存在
 */
Std_ReturnType Mcal_DS18B20_Init(void)
{
    printf("[DS18B20] 初始化中...\n");
    
    /* 步骤1: 主机拉低至少480μs */
    Mcal_DS18B20_PullLow();
    Mcal_DS18B20_DelayUs(DS18B20_INIT_LOW_TIME);
    
    /* 步骤2: 主机释放总线，等待从设备响应 */
    Mcal_DS18B20_Release();
    Mcal_DS18B20_DelayUs(30);  /* 等待从设备准备 */
    
    /* 步骤3: 检查从设备是否拉低了总线 (表示响应) */
    GPIO_PinState pin_level = Mcal_DS18B20_ReadPin();
    
    /* 继续等待从设备释放 */
    Mcal_DS18B20_DelayUs(DS18B20_INIT_RELEASE_TIME);
    
    if (pin_level == GPIO_PIN_RESET) {
        printf("[DS18B20] ✓ 设备检测成功\n");
        ds18b20_state.initialized = 1;
        return STD_OK;
    } else {
        printf("[DS18B20] ✗ 未检测到设备，检查接线和上拉电阻\n");
        ds18b20_state.initialized = 0;
        return STD_NOT_OK;
    }
}

/**
 * @brief 写一个字节 (LSB先发)
 */
static void Mcal_DS18B20_WriteByte(uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        if (byte & 0x01) {
            /* 写1: 拉低1-15μs，然后释放 */
            Mcal_DS18B20_PullLow();
            Mcal_DS18B20_DelayUs(10);
            Mcal_DS18B20_Release();
            Mcal_DS18B20_DelayUs(50);
        } else {
            /* 写0: 拉低60μs，然后释放 */
            Mcal_DS18B20_PullLow();
            Mcal_DS18B20_DelayUs(60);
            Mcal_DS18B20_Release();
            Mcal_DS18B20_DelayUs(10);
        }
        byte >>= 1;
    }
}

/**
 * @brief 读一个字节 (LSB先读)
 */
static uint8_t Mcal_DS18B20_ReadByte(void)
{
    uint8_t byte = 0;
    
    for (int i = 0; i < 8; i++) {
        /* 每个时隙: 拉低，读取，释放 */
        Mcal_DS18B20_PullLow();
        Mcal_DS18B20_DelayUs(2);   /* 拉低2μs */
        Mcal_DS18B20_Release();
        Mcal_DS18B20_DelayUs(12);  /* 等待12μs后读 */
        
        if (Mcal_DS18B20_ReadPin() == GPIO_PIN_SET) {
            byte |= (0x01 << i);   /* 读到1 */
        }
        
        Mcal_DS18B20_DelayUs(45);  /* 完成时隙 */
    }
    
    return byte;
}

/**
 * @brief 启动温度转换
 */
Std_ReturnType Mcal_DS18B20_StartConversion(void)
{
    if (!ds18b20_state.initialized) {
        printf("[DS18B20] 设备未初始化，自动初始化...\n");
        if (Mcal_DS18B20_Init() != STD_OK) {
            return STD_NOT_OK;
        }
    }
    
    /* 重新初始化 */
    if (Mcal_DS18B20_Init() != STD_OK) {
        return STD_NOT_OK;
    }
    
    /* 发送命令序列 */
    Mcal_DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);   /* 跳过ROM搜索 */
    Mcal_DS18B20_WriteByte(DS18B20_CMD_CONVERT_T);  /* 启动转换 */
    
    printf("[DS18B20] 温度转换已启动\n");
    return STD_OK;
}

/**
 * @brief 读取温度原始值
 */
Std_ReturnType Mcal_DS18B20_ReadTemperatureRaw(int16_t *TemperatureRaw)
{
    uint8_t scratchpad[9];
    
    if (TemperatureRaw == NULL) {
        return STD_NOT_OK;
    }
    
    if (!ds18b20_state.initialized) {
        printf("[DS18B20] 设备未初始化\n");
        return STD_NOT_OK;
    }
    
    /* 初始化 */
    if (Mcal_DS18B20_Init() != STD_OK) {
        return STD_NOT_OK;
    }
    
    /* 读取暂存器命令 */
    Mcal_DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);      /* 0xCC */
    Mcal_DS18B20_WriteByte(DS18B20_CMD_READ_SCRATCH);  /* 0xBE */
    
    /* 读取9个字节 */
    for (int i = 0; i < 9; i++) {
        scratchpad[i] = Mcal_DS18B20_ReadByte();
    }
    
    /* 温度值在 Byte0[7:4] + Byte1[7:0] */
    int16_t raw = (scratchpad[1] << 8) | scratchpad[0];
    
    ds18b20_state.last_temp_raw = raw;
    *TemperatureRaw = raw;
    
    return STD_OK;
}

/**
 * @brief 原始值转换为摄氏度
 */
static float Mcal_DS18B20_RawToCelsius(int16_t temp_raw)
{
    /* 温度 = raw / 16
       每个LSB代表1/16℃ (0.0625℃)
       
       例:
       raw = 0x0170 (368) → 368/16 = 23.0℃
       raw = 0xFFC0 (-64) → -64/16 = -4.0℃
    */
    return (float)temp_raw / 16.0f;
}

/**
 * @brief 读取温度并转换为℃
 */
Std_ReturnType Mcal_DS18B20_ReadTemperature(float *Temperature)
{
    int16_t temp_raw;
    
    if (Mcal_DS18B20_ReadTemperatureRaw(&temp_raw) != STD_OK) {
        return STD_NOT_OK;
    }
    
    ds18b20_state.last_temp_celsius = Mcal_DS18B20_RawToCelsius(temp_raw);
    
    if (Temperature != NULL) {
        *Temperature = ds18b20_state.last_temp_celsius;
    }
    
    return STD_OK;
}

/**
 * @brief 获取最后一次读取的温度
 */
float Mcal_DS18B20_GetLastTemperature(void)
{
    return ds18b20_state.last_temp_celsius;
}

/**
 * @brief 获取初始化状态
 */
uint8_t Mcal_DS18B20_IsInitialized(void)
{
    return ds18b20_state.initialized;
}
```

---

## 第二部分：AUTOSAR 快速集成步骤

### 2.1 创建BSW温度监控模块

**文件：src/bsw/dem/dem_temp_monitor.c**

```c
/**
 * @file dem_temp_monitor.c
 * @brief 温度故障监控模块
 */

#include "main.h"
#include "ds18b20_driver.h"
#include "stdio.h"

/* 温度阈值配置 */
#define TEMP_THRESHOLD_HIGH     80.0f   /* 高温报警阈值 */
#define TEMP_THRESHOLD_LOW      78.0f   /* 高温恢复阈值 */
#define TEMP_THRESHOLD_LOW_TEMP -10.0f  /* 低温报警阈值 */
#define TEMP_DEBOUNCE_TIME      10      /* 去抖时间 (100ms单位) */

typedef struct {
    float current_temp;
    uint8_t high_temp_active;
    uint32_t high_temp_counter;
    uint8_t low_temp_active;
    uint32_t low_temp_counter;
} TempMonitorStateType;

static TempMonitorStateType temp_monitor_state = {0};

/**
 * @brief 初始化温度监控
 */
void Bsw_Dem_TempMonitor_Init(void)
{
    temp_monitor_state.current_temp = 0.0f;
    temp_monitor_state.high_temp_active = 0;
    temp_monitor_state.high_temp_counter = 0;
    printf("[DEM] 温度监控初始化完成\n");
}

/**
 * @brief 温度监控主处理函数 (100ms周期)
 */
void Bsw_Dem_TempMonitor_MainFunction(void)
{
    float temperature;
    
    /* 从MCAL读取温度 */
    if (Mcal_DS18B20_ReadTemperature(&temperature) == STD_OK) {
        temp_monitor_state.current_temp = temperature;
        
        /* 高温监控 */
        if (temperature > TEMP_THRESHOLD_HIGH) {
            temp_monitor_state.high_temp_counter++;
            
            if (temp_monitor_state.high_temp_counter >= TEMP_DEBOUNCE_TIME) {
                if (!temp_monitor_state.high_temp_active) {
                    temp_monitor_state.high_temp_active = 1;
                    printf("[DEM] ⚠ 检测到高温: %.1f℃\n", temperature);
                    printf("[DEM] DTC 0x030003 已设置\n");
                }
            }
        } else if (temperature < TEMP_THRESHOLD_LOW) {
            /* 温度恢复 */
            if (temp_monitor_state.high_temp_active) {
                temp_monitor_state.high_temp_active = 0;
                temp_monitor_state.high_temp_counter = 0;
                printf("[DEM] ✓ 温度恢复正常: %.1f℃\n", temperature);
            }
        }
        
        /* 低温监控 */
        if (temperature < TEMP_THRESHOLD_LOW_TEMP) {
            temp_monitor_state.low_temp_counter++;
            
            if (temp_monitor_state.low_temp_counter >= TEMP_DEBOUNCE_TIME) {
                if (!temp_monitor_state.low_temp_active) {
                    temp_monitor_state.low_temp_active = 1;
                    printf("[DEM] ⚠ 检测到低温: %.1f℃\n", temperature);
                }
            }
        } else {
            temp_monitor_state.low_temp_active = 0;
            temp_monitor_state.low_temp_counter = 0;
        }
    } else {
        printf("[DEM] 温度读取失败\n");
    }
}

/**
 * @brief 获取当前温度
 */
float Bsw_Dem_TempMonitor_GetTemperature(void)
{
    return temp_monitor_state.current_temp;
}

/**
 * @brief 获取高温故障状态
 */
uint8_t Bsw_Dem_TempMonitor_GetHighTempStatus(void)
{
    return temp_monitor_state.high_temp_active;
}
```

**文件：inc/bsw/dem/dem_temp_monitor.h**

```c
#ifndef __DEM_TEMP_MONITOR_H
#define __DEM_TEMP_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

void Bsw_Dem_TempMonitor_Init(void);
void Bsw_Dem_TempMonitor_MainFunction(void);
float Bsw_Dem_TempMonitor_GetTemperature(void);
uint8_t Bsw_Dem_TempMonitor_GetHighTempStatus(void);

#ifdef __cplusplus
}
#endif

#endif
```

---

### 2.2 创建RTE接口

**文件：src/rte/rte_interface.c**

```c
/**
 * @file rte_interface.c
 * @brief RTE运行时环境接口
 */

#include "main.h"
#include "ds18b20_driver.h"
#include "dem_temp_monitor.h"
#include "stdio.h"

/* 内部缓冲 */
typedef struct {
    float temperature;
    uint8_t temp_valid;
} RTE_DataType;

static RTE_DataType rte_data = {0};

/**
 * @brief RTE初始化
 */
void Rte_Init(void)
{
    rte_data.temperature = 0.0f;
    rte_data.temp_valid = 0;
    printf("[RTE] 初始化完成\n");
}

/**
 * @brief RTE读取温度数据 (应用调用)
 */
Std_ReturnType Rte_Read_TemperatureSensor_Value(float *Temperature)
{
    if (Temperature == NULL) {
        return STD_NOT_OK;
    }
    
    *Temperature = rte_data.temperature;
    return STD_OK;
}

/**
 * @brief RTE写入LED亮度 (应用调用)
 */
Std_ReturnType Rte_Write_LedControl_Brightness(uint8_t Brightness)
{
    /* 计算PWM占空比 (0-100%) */
    uint32_t pulse = (Brightness * 1999) / 100;
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);
    return STD_OK;
}

/**
 * @brief RTE主函数 (100ms周期)
 */
void Rte_MainFunction(void)
{
    /* 从BSW读取缓存的温度 */
    rte_data.temperature = Bsw_Dem_TempMonitor_GetTemperature();
    rte_data.temp_valid = 1;
}
```

**文件：inc/rte/rte_interface.h**

```c
#ifndef __RTE_INTERFACE_H
#define __RTE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

void Rte_Init(void);
Std_ReturnType Rte_Read_TemperatureSensor_Value(float *Temperature);
Std_ReturnType Rte_Write_LedControl_Brightness(uint8_t Brightness);
void Rte_MainFunction(void);

#ifdef __cplusplus
}
#endif

#endif
```

---

### 2.3 创建ASW应用层

**文件：src/asw/app_temp_monitor.c**

```c
/**
 * @file app_temp_monitor.c
 * @brief 温度监控应用
 */

#include "main.h"
#include "rte_interface.h"
#include "dem_temp_monitor.h"
#include "stdio.h"

/* 应用状态 */
typedef struct {
    uint32_t update_count;
    float last_temp;
} AppTempStateType;

static AppTempStateType app_temp_state = {0};

/**
 * @brief 温度监控应用初始化
 */
void App_TempMonitor_Init(void)
{
    printf("[APP] 温度监控应用初始化\n");
}

/**
 * @brief 温度监控主任务 (100ms周期)
 */
void App_TempMonitor_MainFunction(void)
{
    float temperature;
    uint8_t temp_fault;
    
    /* 读取RTE缓存的温度 */
    if (Rte_Read_TemperatureSensor_Value(&temperature) == STD_OK) {
        app_temp_state.last_temp = temperature;
        app_temp_state.update_count++;
        
        /* 每10次周期 (1秒) 输出一次 */
        if (app_temp_state.update_count % 10 == 0) {
            printf("[APP] 温度: %.2f℃\n", temperature);
        }
        
        /* 检查故障状态 */
        temp_fault = Bsw_Dem_TempMonitor_GetHighTempStatus();
        
        if (temp_fault) {
            /* 高温时设置LED为全亮作为告警 */
            Rte_Write_LedControl_Brightness(100);
        } else {
            /* 正常时LED为中亮度显示状态 */
            Rte_Write_LedControl_Brightness(50);
        }
    }
}
```

**文件：inc/asw/app_temp_monitor.h**

```c
#ifndef __APP_TEMP_MONITOR_H
#define __APP_TEMP_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

void App_TempMonitor_Init(void);
void App_TempMonitor_MainFunction(void);

#ifdef __cplusplus
}
#endif

#endif
```

---

### 2.4 修改主程序集成

**修改 Core/Src/main.c**

```c
/* USER CODE BEGIN Includes */
#include "types.h"
#include "ds18b20_driver.h"
#include "dem_temp_monitor.h"
#include "rte_interface.h"
#include "app_temp_monitor.h"
#include "stdio.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */

/* 任务标志 */
volatile uint8_t task_10ms_flag = 0;
volatile uint8_t task_100ms_flag = 0;
volatile uint8_t task_1000ms_flag = 0;

/* 任务计数器 */
static uint32_t task_10ms_counter = 0;
static uint32_t task_100ms_counter = 0;
static uint32_t task_1000ms_counter = 0;

/* USER CODE END PV */

/**
 * @brief SysTick回调 (1ms周期)
 */
void HAL_SYSTICK_Callback(void)
{
    task_10ms_counter++;
    task_100ms_counter++;
    task_1000ms_counter++;
    
    if (task_10ms_counter >= 10) {
        task_10ms_counter = 0;
        task_10ms_flag = 1;
    }
    
    if (task_100ms_counter >= 100) {
        task_100ms_counter = 0;
        task_100ms_flag = 1;
    }
    
    if (task_1000ms_counter >= 1000) {
        task_1000ms_counter = 0;
        task_1000ms_flag = 1;
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    
    MX_GPIO_Init();
    MX_CAN1_Init();
    MX_SPI1_Init();
    MX_TIM4_Init();
    MX_USART1_UART_Init();
    
    /* USER CODE BEGIN 2 */
    
    /* 启动PWM */
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
    
    printf("\n\n===== STM32F407 AUTOSAR ECU 启动 =====\n");
    printf("[BOOT] MCAL初始化...\n");
    
    /* MCAL: 初始化DS18B20 */
    if (Mcal_DS18B20_Init() != STD_OK) {
        printf("[BOOT] 错误: DS18B20初始化失败！\n");
        while (1) {
            HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);  /* LED闪烁表示错误 */
            HAL_Delay(100);
        }
    }
    
    printf("[BOOT] BSW初始化...\n");
    /* BSW: 初始化温度监控 */
    Bsw_Dem_TempMonitor_Init();
    
    printf("[BOOT] RTE初始化...\n");
    /* RTE: 初始化运行时环境 */
    Rte_Init();
    
    printf("[BOOT] APP初始化...\n");
    /* ASW: 初始化应用层 */
    App_TempMonitor_Init();
    
    printf("[BOOT] ✓ 系统启动完成！\n");
    printf("=======================================\n\n");
    
    /* USER CODE END 2 */
    
    while (1)
    {
        /* 10ms任务 */
        if (task_10ms_flag) {
            task_10ms_flag = 0;
            /* 快速采样、去抖等 */
        }
        
        /* 100ms任务 */
        if (task_100ms_flag) {
            task_100ms_flag = 0;
            
            /* BSW: 故障检测 */
            Bsw_Dem_TempMonitor_MainFunction();
            
            /* RTE: 数据转运 */
            Rte_MainFunction();
            
            /* ASW: 应用逻辑 */
            App_TempMonitor_MainFunction();
        }
        
        /* 1000ms任务 */
        if (task_1000ms_flag) {
            task_1000ms_flag = 0;
            /* NVM写入、诊断等 */
        }
        
        /* 喂狗 */
        HAL_IWDG_Refresh(&hiwdg);
    }
}
```

---

## 第三部分：编译和测试

### 3.1 在CubeIDE中添加文件

```
1. 在项目中创建以下目录:
   ├── Core/Inc/
   │   └── types.h
   ├── inc/
   │   ├── mcal/
   │   │   └── ds18b20/
   │   │       └── ds18b20_driver.h
   │   ├── bsw/
   │   │   └── dem/
   │   │       └── dem_temp_monitor.h
   │   ├── rte/
   │   │   └── rte_interface.h
   │   └── asw/
   │       └── app_temp_monitor.h
   └── src/
       ├── mcal/
       │   └── ds18b20/
       │       └── ds18b20_driver.c
       ├── bsw/
       │   └── dem/
       │       └── dem_temp_monitor.c
       ├── rte/
       │   └── rte_interface.c
       └── asw/
           └── app_temp_monitor.c

2. 在CubeIDE中: File → Import → 导入这些文件

3. 更新项目include路径:
   Project → Properties → C/C++ General → Paths and Symbols
   添加: inc, inc/mcal, inc/bsw, inc/rte, inc/asw
```

### 3.2 编译和烧写

```bash
# 在CubeIDE中
Build → Clean Project
Build → Build Project

# 应该看到:
[100%] Linking C executable firmware.elf
text	data	bss	dec	hex
60000	1200	3500	64700	fd7c
```

### 3.3 预期串口输出

```
===== STM32F407 AUTOSAR ECU 启动 =====
[BOOT] MCAL初始化...
[DS18B20] 初始化中...
[DS18B20] ✓ 设备检测成功
[BOOT] BSW初始化...
[DEM] 温度监控初始化完成
[BOOT] RTE初始化...
[RTE] 初始化完成
[BOOT] APP初始化...
[APP] 温度监控应用初始化
[BOOT] ✓ 系统启动完成！
=======================================

[APP] 温度: 25.34℃
[APP] 温度: 25.35℃
[APP] 温度: 25.34℃
...
[DEM] ⚠ 检测到高温: 81.25℃
[DEM] DTC 0x030003 已设置
...
[DEM] ✓ 温度恢复正常: 78.50℃
```

---

## 第四部分：故障排查

### 问题1: 未检测到DS18B20

```
现象: [DS18B20] ✗ 未检测到设备

原因检查:
☐ PE0是否配置为GPIO_MODE_OUTPUT_OD?
☐ 是否有4.7k-10k上拉电阻连接?
☐ DQ引脚是否正确连接到PE0?
☐ 电源是否正常?

解决方案:
1. 用万用表检查PE0和VCC之间是否有4.7k电阻
2. 用示波器观察PE0初始化时序
3. 断电5秒，重新上电测试
```

### 问题2: 温度值不合理

```
现象: 温度显示-55℃或125℃

原因:
☐ 单线时序不准确 (延迟时间不对)
☐ CRC校验失败

解决方案:
1. 降低系统时钟或增加延迟倍数
2. 添加调试打印，输出原始值检查
3. 多次采样取平均值
```

### 问题3: 编译错误

```
错误: "undefined reference to `Mcal_DS18B20_Init'"

原因: ds18b20_driver.c未被编译

解决方案:
1. 在CubeIDE中检查文件是否被添加到项目
2. 检查include路径设置
3. Clean和重新Build
```

---

## 下一步

```
✓ DS18B20驱动完成
✓ AUTOSAR分层架构完成
✓ 温度监控应用完成

下一步:
□ 添加CAN通信层
□ 添加诊断服务 (DCM)
□ 添加网络管理 (CanNm)
□ 添加NVM管理
□ 完整功能测试
```

**恭喜！你现在有了一套完整的AUTOSAR架构，包含DS18B20驱动！** 🎉

