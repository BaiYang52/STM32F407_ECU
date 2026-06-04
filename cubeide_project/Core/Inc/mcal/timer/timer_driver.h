/**
 * @file timer_driver.h
 * @brief AUTOSAR GPT Driver 接口头文件
 * @version 1.0.0
 *
 * 遵循 AUTOSAR_SWS_GPT 规范，提供：
 *   - 定时器通道的启动/停止
 *   - 计数值获取
 *   - 时间基准（基于 CubeMX SysTick）
 *
 * 当前实现使用 SysTick (1ms) 作为硬件定时器通道 0。
 * TIM4 作为额外的 PWM/定时器通道，在 PWM 驱动中使用。
 */

#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "types.h"
#include "compiler.h"

/* ==================== 常量定义 ==================== */

/**
 * @brief 系统定时器通道号 (SysTick 1ms)
 */
#define GPT_CH_SYSTICK     0U

/**
 * @brief 定时器通道总数
 */
#define GPT_CHANNEL_COUNT  1U

/* ==================== 类型定义 ==================== */

/**
 * @enum Gpt_ChannelType
 * @brief GPT 通道枚举
 */
typedef uint8 Gpt_ChannelType;

/**
 * @enum Gpt_ValueType
 * @brief GPT 计数值类型 (32-bit)
 */
typedef uint32 Gpt_ValueType;

/**
 * @enum Gpt_StateType
 * @brief GPT 通道状态
 */
typedef enum
{
    GPT_STATE_UNINIT = 0U,     /**< 未初始化 */
    GPT_STATE_STOPPED,         /**< 停止 */
    GPT_STATE_RUNNING          /**< 运行中 */
} Gpt_StateType;

/* ==================== 公开函数声明 ==================== */

/**
 * @brief GPT 驱动初始化
 *
 * 初始化 SysTick 定时器为 1ms 周期中断。
 * @note CubeMX 已通过 HAL_Init() 配置 SysTick 为 1ms。
 *       此函数仅记录驱动状态。
 */
FUNC(void, MCAL_CODE)
Gpt_Init(void);

/**
 * @brief 启动指定定时器通道
 * @param[in] Channel  GPT 通道号
 *
 * @note SysTick 通道 (0) 在 HAL_Init 时已自动启动，
 *       此处仅为接口一致性。
 */
FUNC(void, MCAL_CODE)
Gpt_StartTimer(
    Gpt_ChannelType Channel
);

/**
 * @brief 停止指定定时器通道
 * @param[in] Channel  GPT 通道号
 */
FUNC(void, MCAL_CODE)
Gpt_StopTimer(
    Gpt_ChannelType Channel
);

/**
 * @brief 获取当前定时器计数值
 * @param[in]  Channel  GPT 通道号
 * @return Gpt_ValueType  当前计数值 (ms)
 */
FUNC(Gpt_ValueType, MCAL_CODE)
Gpt_GetTimeElapsed(
    Gpt_ChannelType Channel
);

/**
 * @brief 获取 GPT 通道状态
 */
FUNC(Gpt_StateType, MCAL_CODE)
Gpt_GetState(
    Gpt_ChannelType Channel
);

/**
 * @brief 毫秒级延迟 (阻塞)
 * @param[in] ms  延迟毫秒数
 *
 * @note 仅用于初始化阶段的短时间延迟。
 *       运行时请勿使用，会阻塞其他任务。
 */
FUNC(void, MCAL_CODE)
Gpt_DelayMs(
    uint32 ms
);

/**
 * @brief 微秒级延迟 (阻塞忙等)
 * @param[in] us  延迟微秒数
 *
 * @note 基于指令循环的近似延迟，适用于 <100μs 的短时序，
 *       如 DS18B20 单线协议。
 *       精确度取决于系统时钟频率 (168MHz)。
 */
FUNC(void, MCAL_CODE)
Gpt_DelayUs(
    uint32 us
);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_DRIVER_H */