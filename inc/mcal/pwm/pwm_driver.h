/**
 * @file pwm_driver.h
 * @brief AUTOSAR PWM Driver 接口头文件
 * @version 1.0.0
 *
 * 遵循 AUTOSAR_SWS_PWM 规范。
 * 基于 CubeMX TIM4 实现：
 *   - TIM4_CH1 (PD12) → LED PWM
 *   - TIM4_CH2 (PD13) → 马达 PWM
 *   - PD14 → 马达方向 GPIO
 *
 * CubeMX 已配置：
 *   TIM4 预分频器 = 167, ARR = 999 → 频率 = 168MHz/(168*1000) = 1kHz
 *   PWM 模式 1，极性高电平有效
 *   PD14 为 GPIO 输出
 */

#ifndef PWM_DRIVER_H
#define PWM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "types.h"
#include "compiler.h"

/* ==================== 常量定义 ==================== */

/**
 * @brief PWM 通道总数
 */
#define PWM_CHANNEL_COUNT       2U

/**
 * @brief TIM4 ARR 值 (来自 CubeMX 配置)
 */
#define PWM_ARR_VALUE           999U

/**
 * @brief 最大占空比百分比
 */
#define PWM_MAX_DUTY_PERCENT    100U

/* ==================== 枚举类型 ==================== */

/**
 * @enum Pwm_ChannelType
 * @brief PWM 通道枚举
 */
typedef enum
{
    PWM_CH_LED    = 0U,        /**< TIM4_CH1 (PD12) — LED */
    PWM_CH_MOTOR = 1U,         /**< TIM4_CH2 (PD13) — 马达 */
    PWM_CH_MAX                 /**< 通道总数 */
} Pwm_ChannelType;

/**
 * @enum Pwm_StateType
 * @brief PWM 通道状态
 */
typedef enum
{
    PWM_STATE_UNINIT = 0U,     /**< 未初始化 */
    PWM_STATE_STOPPED,         /**< 停止 */
    PWM_STATE_RUNNING          /**< 运行中 */
} Pwm_StateType;

/**
 * @enum Pwm_OutputStateType
 * @brief PWM 输出电平 (当禁用时)
 */
typedef enum
{
    PWM_OUTPUT_LOW  = 0U,      /**< 输出低电平 */
    PWM_OUTPUT_HIGH = 1U       /**< 输出高电平 */
} Pwm_OutputStateType;

/* ==================== 结构体类型 ==================== */

/**
 * @struct Pwm_Config
 * @brief PWM 通道配置
 */
typedef struct
{
    Pwm_ChannelType    channel;         /**< 通道号 */
    uint16             defaultDuty;     /**< 默认占空比 (0-1000, 对应 ARR) */
    Pwm_OutputStateType idleState;      /**< 空闲电平 */
} Pwm_Config;

/* ==================== 公开函数声明 ==================== */

/**
 * @brief PWM 驱动初始化
 *
 * 启动 TIM4 PWM 输出。
 * CubeMX 已配置 TIM4 基本参数，此处调用 HAL_TIM_PWM_Start。
 *
 * @param[in] Config  PWM 配置数组指针
 * @param[in] Num     配置条目数
 * @return Std_ReturnType
 */
FUNC(Std_ReturnType, MCAL_CODE)
Pwm_Init(
    CONSTP2VAR(Pwm_Config, AUTOMATIC, MCAL_APPL_CONST) Config,
    uint8                                              Num
);

/**
 * @brief 设置 PWM 占空比
 *
 * @param[in] Channel   通道号
 * @param[in] DutyCycle 占空比值 (0 ~ PWM_ARR_VALUE)
 *                       0 = 0%, PWM_ARR_VALUE = 100%
 */
FUNC(void, MCAL_CODE)
Pwm_SetDuty(
    Pwm_ChannelType  Channel,
    uint16           DutyCycle
);

/**
 * @brief 设置 PWM 占空比 (百分比)
 *
 * @param[in] Channel   通道号
 * @param[in] Percent   占空比百分比 (0-100)
 */
FUNC(void, MCAL_CODE)
Pwm_SetDutyPercent(
    Pwm_ChannelType  Channel,
    uint8            Percent
);

/**
 * @brief 启动 PWM 输出
 *
 * @param[in] Channel 通道号
 * @return Std_ReturnType
 */
FUNC(Std_ReturnType, MCAL_CODE)
Pwm_Start(
    Pwm_ChannelType Channel
);

/**
 * @brief 停止 PWM 输出
 *
 * 停止后输出电平由 idleState 决定。
 *
 * @param[in] Channel 通道号
 */
FUNC(void, MCAL_CODE)
Pwm_Stop(
    Pwm_ChannelType Channel
);

/**
 * @brief 获取 PWM 通道状态
 */
FUNC(Pwm_StateType, MCAL_CODE)
Pwm_GetState(
    Pwm_ChannelType Channel
);

/**
 * @brief 设置马达方向
 *
 * @param[in] Forward  TRUE = 正转 (LOW), FALSE = 反转 (HIGH)
 *                     PD14 引脚电平控制
 */
FUNC(void, MCAL_CODE)
Pwm_SetMotorDirection(
    boolean Forward
);

/**
 * @brief 获取当前马达方向
 *
 * @return boolean TRUE = 正转, FALSE = 反转
 */
FUNC(boolean, MCAL_CODE)
Pwm_GetMotorDirection(void);

#ifdef __cplusplus
}
#endif

#endif /* PWM_DRIVER_H */