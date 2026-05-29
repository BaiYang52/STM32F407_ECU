/**
 * @file main.c
 * @brief 应用主程序入口点
 * @version 1.0.0
 * @date $(date +%Y-%m-%d)
 */

#include "types.h"
#include "app_init.h"
#include "app_task_scheduler.h"
#include "bsw_dcm.h"

/**
 * @brief 主程序入口
 */
int main(void)
{
    /* 初始化系统 */
    App_System_Init();
    
    /* 初始化调度器 */
    App_TaskScheduler_Init();
    
    /* 主循环 */
    while (1) {
        /* 执行周期任务 */
        App_Task_10ms();
        App_Task_100ms();
        App_Task_1000ms();
    }
    
    return 0;
}
