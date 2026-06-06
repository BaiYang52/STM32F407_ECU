/**
 * @file test_key.c
 * @brief 按键测试
 * @version 1.0.0
 */

#include "main.h"
#include "stdio.h"

/* 按键状态 */
typedef enum {
    KEY_NOT_PRESSED = 0,
    KEY_PRESSED = 1,
    KEY_INVALID = 2,
} KeyStateType;

/* 按键去抖参数 */
#define KEY_DEBOUNCE_TIME 5  /* 50ms (5 * 10ms) */
#define KEY_STICK_TIME 300   /* 3000ms (300 * 10ms) 判断卡滞 */

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint32_t press_time;    /* 按下时长(10ms计) */
    KeyStateType state;
    uint8_t debounce_count; /* 去抖计数 */
    uint8_t is_stuck;       /* 卡滞标志 */
} KeyInfoType;

/* K0 和 K1 按键信息 */
static KeyInfoType key0 = {
    .port = KEY0_GPIO_Port,
    .pin = KEY0_Pin,
};

static KeyInfoType key1 = {
    .port = KEY1_GPIO_Port,
    .pin = KEY1_Pin,
};

/**
 * @brief 按键初始化
 */
void Test_Key_Init(void)
{
    printf("[KEY] 按键初始化完成\n");
    printf("[KEY] K0: PE4 (本地唤醒)\n");
    printf("[KEY] K1: PE3 (普通按键)\n");
}

/**
 * @brief 按键去抖处理
 * @param[in] key_info 按键信息指针
 * @param[in] raw_level 按键原始电平
 */
static void Test_Key_Debounce(KeyInfoType *key_info, GPIO_PinState raw_level)
{
    /* 原始电平为LOW = 按下, HIGH = 未按 */
    KeyStateType raw_state = (raw_level == GPIO_PIN_RESET) ? KEY_PRESSED : KEY_NOT_PRESSED;

    if (raw_state == key_info->state) {
        /* 状态一致，复位去抖计数 */
        key_info->debounce_count = 0;
    }
    else {
        /* 状态不一致，计数 */
        key_info->debounce_count++;

        if (key_info->debounce_count >= KEY_DEBOUNCE_TIME) {
            /* 去抖成功，状态改变 */
            key_info->state = raw_state;
            key_info->debounce_count = 0;

            if (key_info->state == KEY_PRESSED) {
                key_info->press_time = 0;
            }
        }
    }
}

/**
 * @brief 按键 10ms 周期处理
 */
void Test_Key_10ms_Task(void)
{
    /* 处理 K0 */
    GPIO_PinState k0_level = HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin);
    Test_Key_Debounce(&key0, k0_level);

    /* 处理 K1 */
    GPIO_PinState k1_level = HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin);
    Test_Key_Debounce(&key1, k1_level);

    /* 计算按压时长 */
    if (key0.state == KEY_PRESSED) {
        key0.press_time++;

        /* 超过3秒判断为卡滞 */
        if (key0.press_time >= KEY_STICK_TIME) {
            if (!key0.is_stuck) {
                key0.is_stuck = 1;
                printf("[KEY] K0 卡滞! 已按下 %ldms\n", key0.press_time * 10);
            }
        }
    }
    else {
        /* 按键释放 */
        if (key0.press_time > 0 && key0.press_time < KEY_STICK_TIME) {
            printf("[KEY] K0 按下 %ldms\n", key0.press_time * 10);
        }
        key0.press_time = 0;
        key0.is_stuck = 0;
    }

    if (key1.state == KEY_PRESSED) {
        key1.press_time++;

        if (key1.press_time >= KEY_STICK_TIME) {
            if (!key1.is_stuck) {
                key1.is_stuck = 1;
                printf("[KEY] K1 卡滞! 已按下 %ldms\n", key1.press_time * 10);
            }
        }
    }
    else {
        if (key1.press_time > 0 && key1.press_time < KEY_STICK_TIME) {
            printf("[KEY] K1 按下 %ldms\n", key1.press_time * 10);
        }
        key1.press_time = 0;
        key1.is_stuck = 0;
    }
}

/**
 * @brief 外部中断回调 (由 stm32f4xx_it.c 调用)
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY0_Pin) {
        printf("[KEY] K0 中断触发\n");
    } else if (GPIO_Pin == KEY1_Pin) {
        printf("[KEY] K1 中断触发\n");
    }
}

/**
 * @brief 获取按键状态
 */
KeyStateType Test_Key_GetState(uint8_t key_id)
{
    if (key_id == 0) {
        return key0.state;
    } else if (key_id == 1) {
        return key1.state;
    }
    return KEY_INVALID;
}

/**
 * @brief 获取按键按下时长
 */
uint32_t Test_Key_GetPressTime(uint8_t key_id)
{
    if (key_id == 0) {
        return key0.press_time * 10;  /* 单位: ms */
    } else if (key_id == 1) {
        return key1.press_time * 10;
    }
    return 0;
}
