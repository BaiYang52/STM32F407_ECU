/**
 * @file timer_driver.c
 * @brief AUTOSAR GPT Driver 实现
 * @version 1.0.0
 *
 * 基于 CubeMX SysTick (1ms) 实现 GPT 通道 0。
 * 提供 ms/us 级延迟函数。
 */

#include "timer_driver.h"
#include "stm32f4xx_hal.h"

/* ==================== 私有全局变量 ==================== */

/** GPT 驱动状态 */
static VAR(Gpt_StateType, MCAL_APPL_DATA) s_gptState = GPT_STATE_UNINIT;

/* ==================== 公开函数实现 ==================== */

/**
 * @brief GPT 驱动初始化
 */
FUNC(void, MCAL_CODE)
Gpt_Init(void)
{
    /* CubeMX 的 HAL_Init() 已配置 SysTick 为 1ms */
    s_gptState = GPT_STATE_RUNNING;
}

/**
 * @brief 启动定时器
 *
 * SysTick 在 HAL_Init 时已自动启动。
 * 如果处于 STOPPED 状态，可通过重新配置 SysTick 重启。
 */
FUNC(void, MCAL_CODE)
Gpt_StartTimer(
    Gpt_ChannelType Channel
)
{
    (void)Channel;   /* 当前仅支持 GPT_CH_SYSTICK */

    if (s_gptState == GPT_STATE_STOPPED) {
        /* 重新配置 SysTick */
        HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000U);  /* 1ms */
        HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
        s_gptState = GPT_STATE_RUNNING;
    }
}

/**
 * @brief 停止定时器
 *
 * 禁用 SysTick 中断，停止计数。
 */
FUNC(void, MCAL_CODE)
Gpt_StopTimer(
    Gpt_ChannelType Channel
)
{
    (void)Channel;

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    s_gptState = GPT_STATE_STOPPED;
}

/**
 * @brief 获取当前计数值 (ms)
 *
 * 直接返回 HAL_GetTick() 的值。
 * 注意：32-bit 无符号溢出时间为 2^32 ms ≈ 49.7 天。
 */
FUNC(Gpt_ValueType, MCAL_CODE)
Gpt_GetTimeElapsed(
    Gpt_ChannelType Channel
)
{
    (void)Channel;
    return HAL_GetTick();
}

/**
 * @brief 获取 GPT 通道状态
 */
FUNC(Gpt_StateType, MCAL_CODE)
Gpt_GetState(
    Gpt_ChannelType Channel
)
{
    (void)Channel;
    return s_gptState;
}

/**
 * @brief 毫秒级阻塞延迟
 *
 * 使用 HAL_Delay 实现，会阻塞 CPU。
 * 仅用于初始化和不影响时序的短延迟场景。
 */
FUNC(void, MCAL_CODE)
Gpt_DelayMs(
    uint32 ms
)
{
    /* 检查 SysTick 是否在运行 */
    if (s_gptState == GPT_STATE_RUNNING) {
        uint32 start = HAL_GetTick();
        while ((HAL_GetTick() - start) < ms) {
            /* 忙等 */
        }
    } else {
        /* SysTick 未运行，使用循环近似延迟 */
        volatile uint32 count;
        for (uint32 i = 0U; i < ms; i++) {
            count = 168000U;  /* 168MHz 下约 1ms */
            while (count--) {
                __NOP();
            }
        }
    }
}

/**
 * @brief 微秒级阻塞延迟 (忙等)
 *
 * 基于指令循环的近似延迟。
 * 适用于 <100μs 的短时序，如 DS18B20 协议时序。
 *
 * 在 168MHz 下：
 *   - 简单循环 + NOP 约 15 个周期/次
 *   - 1μs ≈ 15 次循环
 */
FUNC(void, MCAL_CODE)
Gpt_DelayUs(
    uint32 us
)
{
    volatile uint32 count = us * 15U;
    while (count--) {
        __NOP();
    }
}