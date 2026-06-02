/**
 * @file app_task_scheduler.c
 * @brief 任务调度器 (由SysTick驱动)
 */

#include "main.h"

/* 任务标志变量 (在main.c中声明) */
uint8_t s_task10msFlag = 0;
uint8_t s_task100msFlag = 0;
uint8_t s_task1000msFlag = 0;

static uint32_t s_task10msCounter = 0;
static uint32_t s_task100msCounter = 0;
static uint32_t s_task1000msCounter = 0;

/**
 * @brief App系统初始化
 * @note 由main.c调用
 */
void App_System_Init(void)
{
    s_task10msCounter = 0;
    s_task100msCounter = 0;
    s_task1000msCounter = 0;
}

/**
 * @brief App任务调度器初始化
 */
void App_TaskScheduler_Init(void)
{
    // 初始化已在App_System_Init中完成
}

/**
 * @brief SysTick中断处理 (1ms调用一次)
 * @note 由HAL库自动调用，从SysTick_Handler中调用
 */
void HAL_SYSTICK_Callback(void)
{
    // 这个函数会在每个HAL_IncTick()后自动调用
    // 默认1ms调用一次

    s_task10msCounter++;
    if (s_task10msCounter%10==0) {
        s_task10msFlag = 1;  // 标记10ms任务可以运行
    }

    s_task100msCounter++;
    if (s_task100msCounter%100==0) {
        s_task100msFlag = 1;  // 标记100ms任务可以运行
    }

    s_task1000msCounter++;
    if (s_task1000msCounter%1000==0) {
        s_task1000msFlag = 1;  // 标记1000ms任务可以运行
    }
}
