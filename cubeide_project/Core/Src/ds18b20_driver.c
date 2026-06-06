/**
 * @file ds18b20_driver.c
 * @brief DS18B20 单线温度传感器驱动实现
 * @version 1.0.0
 */

#include <ds18b20_driver.h>
#include "main.h"
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
    uint8_t conversion_pending;
    uint32_t conversion_start_tick;
} DS18B20_StateType;

static DS18B20_StateType ds18b20_state = {
    .initialized = 0,
    .last_temp_raw = 0,
    .last_temp_celsius = 0.0f,
    .conversion_pending = 0,
    .conversion_start_tick = 0,
};

static uint8_t ds18b20_init_printed = 0;

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
    GPIO_InitStruct.Pull = GPIO_PULLUP; /* 使用上拉以保持总线高电平（如无外部上拉） */
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
    /* 步骤1: 主机拉低至少480μs */
    Mcal_DS18B20_PullLow();
    Mcal_DS18B20_DelayUs(DS18B20_INIT_LOW_TIME);
    
    /* 步骤2: 主机释放总线，等待从设备响应 */
    Mcal_DS18B20_Release();
    Mcal_DS18B20_DelayUs(60);  /* 等待从设备准备 */
    
    /* 步骤3: 检查从设备是否拉低了总线 (表示响应) */
    GPIO_PinState pin_level = Mcal_DS18B20_ReadPin();
    
    /* 继续等待从设备释放 */
    Mcal_DS18B20_DelayUs(DS18B20_INIT_RELEASE_TIME);
    
    if (pin_level == GPIO_PIN_RESET) {
        ds18b20_state.initialized = 1;
        if (!ds18b20_init_printed) {
            printf("[DS18B20] ✓ 设备检测成功\n");
            ds18b20_init_printed = 1;
        }
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
    /* 每次发送转换命令前先做复位/检测（presence） */
    if (Mcal_DS18B20_Init() != STD_OK) {
        return STD_NOT_OK;
    }

    /* 发送命令序列 */
    Mcal_DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);   /* 跳过ROM搜索 */
    Mcal_DS18B20_WriteByte(DS18B20_CMD_CONVERT_T);  /* 启动转换 */

    /* 记录转换开始时间，非阻塞 */
    ds18b20_state.conversion_pending = 1;
    ds18b20_state.conversion_start_tick = HAL_GetTick();

    return STD_OK;
}

/**
 * @brief 读取温度原始值
 */
Std_ReturnType Mcal_DS18B20_ReadTemperatureRaw(int16_t *TemperatureRaw)
{
    uint8_t lsb = 0, msb = 0;

    if (TemperatureRaw == NULL) {
        return STD_NOT_OK;
    }

    /* 发送 reset 并读暂存器（需 presence） */
    if (Mcal_DS18B20_Init() != STD_OK) {
        return STD_NOT_OK;
    }

    Mcal_DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    Mcal_DS18B20_WriteByte(DS18B20_CMD_READ_SCRATCH);

    lsb = Mcal_DS18B20_ReadByte();
    msb = Mcal_DS18B20_ReadByte();

    int16_t raw = (int16_t)((msb << 8) | lsb);

    ds18b20_state.last_temp_raw = raw;
    *TemperatureRaw = raw;

    /* 清除 pending 标志 */
    ds18b20_state.conversion_pending = 0;
    ds18b20_state.conversion_start_tick = 0;

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

    /* 非阻塞：如果没有进行中的转换则启动转换并立即返回 */
    if (!ds18b20_state.conversion_pending) {
        if (Mcal_DS18B20_StartConversion() != STD_OK) {
            return STD_NOT_OK;
        }
        return STD_NOT_OK; /* 还未完成 */
    }

    /* 检查转换是否完成（最大750ms） */
    if ((HAL_GetTick() - ds18b20_state.conversion_start_tick) < 750U) {
        return STD_NOT_OK;
    }

    /* 读取结果 */
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
