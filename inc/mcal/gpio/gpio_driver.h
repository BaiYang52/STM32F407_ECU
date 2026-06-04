/**
 * @file gpio_driver.h
 * @brief AUTOSAR Dio Driver 接口头文件
 * @version 1.0.0
 *
 * 遵循 AUTOSAR_SWS_DIO 规范。
 * 提供 GPIO 读/写/配置的抽象接口，包装 CubeMX HAL_GPIO 函数。
 */

#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "types.h"
#include "compiler.h"

/* ==================== 类型定义 ==================== */

/**
 * @enum Dio_ChannelType
 * @brief DIO 通道 (单个引脚) 类型
 */
typedef uint8 Dio_ChannelType;

/**
 * @enum Dio_PortType
 * @brief DIO 端口 (GPIO 组) 类型
 */
typedef uint8 Dio_PortType;

/**
 * @enum Dio_LevelType
 * @brief DIO 电平类型
 */
typedef enum
{
    DIO_LVL_LOW  = 0U,      /**< 低电平 (0V) */
    DIO_LVL_HIGH = 1U       /**< 高电平 (3.3V) */
} Dio_LevelType;

/**
 * @enum Dio_PortLevelType
 * @brief DIO 端口电平 (整个 GPIO 口)
 */
typedef uint16 Dio_PortLevelType;

/* ==================== 通道 ID 宏定义 ==================== */

/**
 * @defgroup 引脚映射 (对应 CubeMX 配置)
 * @{
 */

/** PE0 — DS18B20 温度传感器 */
#define DIO_CH_DS18B20_DQ        0U
/** PE3 — KEY1 按键输入 */
#define DIO_CH_KEY1              1U
/** PE4 — KEY0 按键输入 (本地唤醒) */
#define DIO_CH_KEY0              2U
/** PD12 — LED PWM (TIM4_CH1) 暂作 GPIO，实际 PWM 由 Timer 驱动 */
#define DIO_CH_LED_PWM           3U
/** PD13 — 马达 PWM (TIM4_CH2) 暂作 GPIO */
#define DIO_CH_MOTOR_PWM         4U
/** PD14 — 马达方向控制 */
#define DIO_CH_MOTOR_DIR         5U
/** PB0  — W25Q16 Flash 片选 */
#define DIO_CH_FLASH_CS          6U
/** PA8  — CAN1_STBY (SIT1042T 待机控制) */
#define DIO_CH_CAN1_STBY         7U

/** @} */

/* ==================== 公开函数声明 ==================== */

/**
 * @brief DIO 驱动初始化
 * @note CubeMX 已完成 GPIO 时钟和模式的初始化。
 *       此函数仅用于记录已初始化的 DIO 通道列表。
 */
FUNC(void, MCAL_CODE)
Dio_Init(void);

/**
 * @brief 读取单个引脚电平
 * @param[in] ChannelId DIO 通道 ID
 * @return Dio_LevelType   DIO_LVL_HIGH 或 DIO_LVL_LOW
 */
FUNC(Dio_LevelType, MCAL_CODE)
Dio_ReadChannel(
    Dio_ChannelType ChannelId
);

/**
 * @brief 写入单个引脚电平
 * @param[in] ChannelId DIO 通道 ID
 * @param[in] Level     DIO_LVL_LOW 或 DIO_LVL_HIGH
 */
FUNC(void, MCAL_CODE)
Dio_WriteChannel(
    Dio_ChannelType  ChannelId,
    Dio_LevelType    Level
);

/**
 * @brief 读取整个端口电平
 * @param[in] PortId 端口 ID (0=GPIOA, 1=GPIOB, 2=GPIOC, ...)
 * @return Dio_PortLevelType 端口各引脚电平 (bitmask)
 */
FUNC(Dio_PortLevelType, MCAL_CODE)
Dio_ReadPort(
    Dio_PortType PortId
);

/**
 * @brief 写入整个端口电平
 * @param[in] PortId 端口 ID
 * @param[in] Level  端口电平值 (bitmask)
 */
FUNC(void, MCAL_CODE)
Dio_WritePort(
    Dio_PortType       PortId,
    Dio_PortLevelType  Level
);

/**
 * @brief 翻转单个引脚电平
 * @param[in] ChannelId DIO 通道 ID
 */
FUNC(void, MCAL_CODE)
Dio_FlipChannel(
    Dio_ChannelType ChannelId
);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_DRIVER_H */