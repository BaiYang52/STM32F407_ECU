/**
 * @file rte.h
 * @brief AUTOSAR RTE (Runtime Environment) — SWC调度接口
 * @version 1.0.0
 *
 * 负责对 ASW 组件暴露 Rte_Read/Rte_Write 接口，
 * 并提供定时 Runnable 主调度入口。
 *
 * SWC 组件映射:
 *   dim   (DIgital iMage)    — LED PWM 控制, 按键状态采集
 *   fan   (FAN control)      — 电机 PWM 控制, 风扇方向
 *   heatm (HEAT Manager)     — DS18B20 温度采集, 温度报告
 */

#ifndef RTE_H
#define RTE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "types.h"
#include "compiler.h"
#include "rte_interface.h"

/* ==================== 常量定义 ==================== */

/**
 * @brief 按键长按判定时间 (ms)
 */
#define RTE_KEY_LONG_PRESS_MS        30000U

/**
 * @brief 按键无效判定时间 (ms)
 */
#define RTE_KEY_INVALID_MS           (RTE_KEY_LONG_PRESS_MS + 1000U)

/* ==================== Runnable ID 枚举 ==================== */

typedef enum
{
    RTE_RUN_ID_DIM_10MS        = 0U,  /**< DIM 10ms 任务: LED呼吸更新 */
    RTE_RUN_ID_DIM_100MS       = 1U,  /**< DIM 100ms 任务: 按键采集+Veh_Speed检测 */
    RTE_RUN_ID_DIM_1000MS      = 2U,  /**< DIM 1000ms 任务: IGN检测 */
    RTE_RUN_ID_FAN_10MS        = 3U,  /**< FAN 10ms 任务: 电机PWM更新 */
    RTE_RUN_ID_FAN_100MS       = 4U,  /**< FAN 100ms 任务: 电机方向切换 */
    RTE_RUN_ID_HEATM_1000MS    = 5U,  /**< HEATM 1000ms 任务: 温度采集+上报 */
    RTE_RUN_ID_MAX
} Rte_RunIdType;

/* ==================== 公开函数声明 ==================== */

/**
 * @brief RTE 初始化
 *
 * 注册各 SWC 组件需要的回调链到 BSW (Com/CanIf)。
 */
FUNC(void, RTE_CODE)
Rte_Init(void);

/**
 * @brief RTE 10ms 调度
 *
 * 调用 DIM_10ms, FAN_10ms Runnable
 */
FUNC(void, RTE_CODE)
Rte_MainFunction_10ms(void);

/**
 * @brief RTE 100ms 调度
 *
 * 调用 DIM_100ms, FAN_100ms Runnable
 */
FUNC(void, RTE_CODE)
Rte_MainFunction_100ms(void);

/**
 * @brief RTE 1000ms 调度
 *
 * 调用 DIM_1000ms, HEATM_1000ms Runnable
 */
FUNC(void, RTE_CODE)
Rte_MainFunction_1000ms(void);

/* ==================== ASW Runnable 声明 (由各 SWC .c 实现) ==================== */

/**
 * @brief DIM — LED 呼吸灯 (10ms 周期)
 */
void DIM_Run_LED_Breath(void);

/**
 * @brief DIM — 按键状态采集 + 车速信号检测 (100ms 周期)
 */
void DIM_Run_KeyAndVehDetect(void);

/**
 * @brief DIM — IGN 离线事件检测 (1000ms 周期)
 */
void DIM_Run_IGNDetect(void);

/**
 * @brief FAN — 电机 PWM 更新 (10ms 周期)
 */
void FAN_Run_MotorPWM(void);

/**
 * @brief FAN — 电机方向控制 (100ms 周期)
 */
void FAN_Run_MotorDir(void);

/**
 * @brief HEATM — 温度采集与上报 (1000ms 周期)
 */
void HEATM_Run_Temperature(void);

#ifdef __cplusplus
}
#endif

#endif /* RTE_H */