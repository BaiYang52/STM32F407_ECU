/**
 * @file pwm_driver.c
 * @brief AUTOSAR PWM Driver 实现
 * @version 1.0.0
 *
 * 基于 CubeMX TIM4 实现 LED 和马达 PWM。
 * 使用 extern htim4 句柄。
 *
 * 引脚映射:
 *   TIM4_CH1 (PD12) → LED PWM  (输出比较通道1)
 *   TIM4_CH2 (PD13) → 马达 PWM (输出比较通道2)
 *   PD14           → 马达方向 (GPIO)
 */

#include "pwm_driver.h"
#include "stm32f4xx_hal.h"

/* ==================== 外部引用：CubeMX 句柄 ==================== */
extern TIM_HandleTypeDef htim4;

/* ==================== 私有全局变量 ==================== */

/** PWM 通道状态 */
static VAR(Pwm_StateType, MCAL_APPL_DATA) s_pwmState[PWM_CHANNEL_COUNT] = {
    PWM_STATE_UNINIT, PWM_STATE_UNINIT
};

/* ==================== 公开函数实现 ==================== */

/**
 * @brief PWM 驱动初始化
 *
 * CubeMX 已配置 TIM4 模式、预分频器、ARR 和通道输出。
 * 此处启动 PWM 输出。
 */
FUNC(Std_ReturnType, MCAL_CODE)
Pwm_Init(
    CONSTP2VAR(Pwm_Config, AUTOMATIC, MCAL_APPL_CONST) Config,
    uint8                                              Num
)
{
    uint8 i;

    /* 启动 TIM4 所有已配置的 PWM 通道 */
    for (i = 0U; i < Num; i++) {
        Pwm_ChannelType ch = Config[i].channel;
        uint32 halChannel;

        /* 映射到 HAL TIM 通道 */
        if (ch == PWM_CH_LED) {
            halChannel = TIM_CHANNEL_1;
        } else if (ch == PWM_CH_MOTOR) {
            halChannel = TIM_CHANNEL_2;
        } else {
            continue;
        }

        /* 设置默认占空比 */
        __HAL_TIM_SET_COMPARE(&htim4, halChannel, Config[i].defaultDuty);

        /* 启动 PWM 输出 */
        if (HAL_TIM_PWM_Start(&htim4, halChannel) == HAL_OK) {
            s_pwmState[ch] = PWM_STATE_RUNNING;
        }
    }

    /* 初始化马达方向 */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);  /* 默认正转 */

    return STD_OK;
}

/**
 * @brief 设置 PWM 占空比 (原始值)
 *
 * @param[in] Channel   通道号
 * @param[in] DutyCycle 0 ~ PWM_ARR_VALUE (999)
 */
FUNC(void, MCAL_CODE)
Pwm_SetDuty(
    Pwm_ChannelType  Channel,
    uint16           DutyCycle
)
{
    uint32 halChannel;
    uint32 ccVal;

    if (Channel == PWM_CH_LED) {
        halChannel = TIM_CHANNEL_1;
    } else if (Channel == PWM_CH_MOTOR) {
        halChannel = TIM_CHANNEL_2;
    } else {
        return;
    }

    /* 限制占空比范围 */
    ccVal = (DutyCycle > PWM_ARR_VALUE) ? PWM_ARR_VALUE : DutyCycle;

    __HAL_TIM_SET_COMPARE(&htim4, halChannel, ccVal);
}

/**
 * @brief 设置 PWM 占空比 (百分比)
 *
 * @param[in] Channel   通道号
 * @param[in] Percent   0-100
 */
FUNC(void, MCAL_CODE)
Pwm_SetDutyPercent(
    Pwm_ChannelType  Channel,
    uint8            Percent
)
{
    uint32 halChannel;
    uint32 ccVal;

    if (Channel == PWM_CH_LED) {
        halChannel = TIM_CHANNEL_1;
    } else if (Channel == PWM_CH_MOTOR) {
        halChannel = TIM_CHANNEL_2;
    } else {
        return;
    }

    /* 限制百分比范围 */
    if (Percent > PWM_MAX_DUTY_PERCENT) {
        Percent = PWM_MAX_DUTY_PERCENT;
    }

    /* 计算 CCR 值: Percent / 100 * ARR */
    ccVal = ((uint32)Percent * (uint32)PWM_ARR_VALUE) / 100U;

    __HAL_TIM_SET_COMPARE(&htim4, halChannel, ccVal);
}

/**
 * @brief 启动 PWM 输出
 */
FUNC(Std_ReturnType, MCAL_CODE)
Pwm_Start(
    Pwm_ChannelType Channel
)
{
    uint32 halChannel;

    if (Channel == PWM_CH_LED) {
        halChannel = TIM_CHANNEL_1;
    } else if (Channel == PWM_CH_MOTOR) {
        halChannel = TIM_CHANNEL_2;
    } else {
        return STD_NOT_OK;
    }

    if (HAL_TIM_PWM_Start(&htim4, halChannel) == HAL_OK) {
        s_pwmState[Channel] = PWM_STATE_RUNNING;
        return STD_OK;
    }

    return STD_NOT_OK;
}

/**
 * @brief 停止 PWM 输出
 */
FUNC(void, MCAL_CODE)
Pwm_Stop(
    Pwm_ChannelType Channel
)
{
    uint32 halChannel;

    if (Channel == PWM_CH_LED) {
        halChannel = TIM_CHANNEL_1;
    } else if (Channel == PWM_CH_MOTOR) {
        halChannel = TIM_CHANNEL_2;
    } else {
        return;
    }

    HAL_TIM_PWM_Stop(&htim4, halChannel);
    s_pwmState[Channel] = PWM_STATE_STOPPED;
}

/**
 * @brief 获取 PWM 通道状态
 */
FUNC(Pwm_StateType, MCAL_CODE)
Pwm_GetState(
    Pwm_ChannelType Channel
)
{
    if (Channel >= PWM_CHANNEL_COUNT) {
        return PWM_STATE_UNINIT;
    }
    return s_pwmState[Channel];
}

/**
 * @brief 设置马达方向
 *
 * PD14 GPIO:
 *   LOW  = 正转 (Forward)
 *   HIGH = 反转 (Reverse)
 */
FUNC(void, MCAL_CODE)
Pwm_SetMotorDirection(
    boolean Forward
)
{
    GPIO_PinState level = Forward ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, level);
}

/**
 * @brief 获取当前马达方向
 */
FUNC(boolean, MCAL_CODE)
Pwm_GetMotorDirection(void)
{
    return (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_14) == GPIO_PIN_RESET) ? TRUE : FALSE;
}