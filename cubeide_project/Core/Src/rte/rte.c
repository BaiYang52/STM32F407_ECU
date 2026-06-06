/**
 * @file rte.c
 * @brief AUTOSAR RTE (Runtime Environment) 实现
 * @version 1.0.0
 *
 * 实现 ASW Runnable 调度器。
 * 映射关系:
 *   10ms  → DIM_LED_Breath, FAN_MotorPWM
 *   100ms → DIM_KeyAndVehDetect, FAN_MotorDir
 *   1000ms→ DIM_IGNDetect, HEATM_Temperature
 */

#include <rte/rte.h>
#include "common.h"

FUNC(void, RTE_CODE)
Rte_Init(void)
{
    /* ASW 组件直接调用 BSW API, 无需额外注册 */
}

FUNC(void, RTE_CODE)
Rte_MainFunction_10ms(void)
{
    Dim_MainFunction();
    FAN_Run_MotorPWM();
}

FUNC(void, RTE_CODE)
Rte_MainFunction_100ms(void)
{
    DIM_Run_KeyAndVehDetect();
    FAN_Run_MotorDir();
}

FUNC(void, RTE_CODE)
Rte_MainFunction_1000ms(void)
{
    DIM_Run_IGNDetect();
    HEATM_Run_Temperature();
}
