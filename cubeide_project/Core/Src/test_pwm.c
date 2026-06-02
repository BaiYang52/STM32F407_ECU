/**
 * @file test_pwm.c
 * @brief PWM输出测试 (LED + 马达)
 * @version 1.0.0
 */

#include "main.h"
#include "stdio.h"

/* PWM 测试状态 */
typedef enum {
    PWM_TEST_IDLE = 0,
    PWM_TEST_LED_BREATH,      /* LED呼吸效果 */
    PWM_TEST_MOTOR_SPEED,     /* 马达速度扫描 */
    PWM_TEST_MOTOR_REVERSE,   /* 马达正反转 */
} PWM_TestStateType;

static PWM_TestStateType pwm_state = PWM_TEST_IDLE;
static uint32_t pwm_counter = 0;

/**
 * @brief PWM初始化测试
 * @details 启动PWM输出
 */
void Test_PWM_Init(void)
{
    printf("[PWM] 启动PWM输出...\n");

    printf("[PWM] TIM4启动状态:%d...\n",HAL_TIM_PWM_GetState(&htim4));
    /* 启动LED PWM (TIM4_CH1) */
    if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1) == HAL_OK) {
        printf("[PWM] LED_PWM (PD12) 启动成功\n");
    } else {
        printf("[PWM] LED_PWM 启动失败!\n");
        return;
    }

    /* 启动马达PWM (TIM4_CH2) */
    if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2) == HAL_OK) {
        printf("[PWM] MOTOR_PWM (PD13) 启动成功\n");
    } else {
        printf("[PWM] MOTOR_PWM 启动失败!\n");
        return;
    }

    /* 初始化马达方向 (PD14) */
    HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, GPIO_PIN_RESET);
    printf("[PWM] 马达方向设置为: 正转 (LOW)\n");
    printf("[PWM] 初始化完成！\n");
    Test_PWM_SetMode(PWM_TEST_LED_BREATH);

}

/**
 * @brief LED呼吸效果测试
 * @details 占空比从0-100%-0循环，模拟呼吸
 */
void Test_PWM_LED_Breath(void)
{
    static uint16_t brightness = 0;
    static int8_t direction = 1;  /* 1: 增加, -1: 减少 */

    /* ARR = 999, 所以PWM范围是 0-999 */
    uint32_t pulse = (brightness * 999) / 100;

    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);

    brightness += direction;

    if (brightness >= 100) {
        brightness = 100;
        direction = -1;
    } else if (brightness <= 0) {
        brightness = 0;
        direction = 1;
    }
}

/**
 * @brief 马达速度扫描测试
 * @details 占空比从0-100%, 每50ms增加5%
 */
void Test_PWM_Motor_Speed_Scan(void)
{
    static uint16_t speed = 0;

    /* ARR = 999 */
    uint32_t pulse = (speed * 999) / 100;
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, pulse);

    speed += 1;
    if (speed > 100) {
        speed = 0;
        printf("[PWM] 马达速度扫描完成，重新开始\n");
    }

    printf("[PWM] 马达速度: %d%%\n", speed);
}

/**
 * @brief 马达正反转测试
 * @details 马达每2秒正转或反转
 */
void Test_PWM_Motor_Reverse(void)
{
    static uint32_t toggle_count = 0;
    static uint8_t direction = 0;  /* 0: 正转, 1: 反转 */

    toggle_count++;

    /* 每 20 个 100ms = 2秒 切换一次 */
    if (toggle_count >= 20) {
        toggle_count = 0;
        direction = !direction;

        if (direction == 0) {
            HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, GPIO_PIN_RESET);
            printf("[PWM] 马达方向: 正转 (LOW)\n");
        } else {
            HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, GPIO_PIN_SET);
            printf("[PWM] 马达方向: 反转 (HIGH)\n");
        }
    }

    /* 设置马达速度为50% */
    uint32_t pulse = (50 * 1999) / 100;
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, pulse);
}

/**
 * @brief PWM 10ms 周期任务
 * @details 根据测试状态执行不同的PWM控制
 */
void Test_PWM_10ms_Task(void)
{
    switch (pwm_state) {
        case PWM_TEST_LED_BREATH:
            Test_PWM_LED_Breath();
            break;
        case PWM_TEST_MOTOR_SPEED:
            Test_PWM_Motor_Speed_Scan();
            break;
        case PWM_TEST_MOTOR_REVERSE:
            Test_PWM_Motor_Reverse();
            break;
        default:
            break;
    }
}

/**
 * @brief 设置PWM测试模式
 * @param[in] state 测试状态
 */
void Test_PWM_SetMode(PWM_TestStateType state)
{
    pwm_state = state;
    printf("[PWM] 设置测试模式: %d\n", state);
}

/**
 * @brief 获取当前PWM测试状态
 * @return PWM_TestStateType
 */
PWM_TestStateType Test_PWM_GetMode(void)
{
    return pwm_state;
}
