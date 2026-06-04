/*
 * app_task_scheduler.h
 *
 *  Created on: May 30, 2026
 *      Author: BaiYang
 */

#ifndef __APP_TASK_SCHEDULER_H
#define __APP_TASK_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 外部任务标志 (由SysTick中断设置) */
extern uint8_t s_task10msFlag;
extern uint8_t s_task100msFlag;
extern uint8_t s_task1000msFlag;

void App_System_Init(void);
void App_TaskScheduler_Init(void);
void HAL_SYSTICK_Callback(void);

#ifdef __cplusplus
}
#endif

#endif
