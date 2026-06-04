/**
 * @file heatm.h
 * @brief HEATM (HEAT Manager) SWC — DS18B20 温度采集与上报
 * @version 1.0.0
 *
 * Runnable:
 *   HEATM_Run_Temperature()  — 1000ms, 温度采集 + Com 写入
 *
 * SR/CS 接口:
 *   SR: Mcal_DS18B20_ReadTemperature(&temp)           (读取 DS18B20)
 *   SR: Rte_Write_EcuStatus_Port_ECU_Temperature(raw)  (温度写入 Com)
 *
 * 对应需求:
 *   #3 DS18B20 读取温度 → 设置 ECU_Temperature
 */

#ifndef HEATM_H
#define HEATM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* ==================== HEATM 公开变量声明 ==================== */

/** 最后一次温度读数 (Celsius) */
extern float32 g_HEATM_TemperatureC;

/* ==================== Runnable 声明 ==================== */

void HEATM_Run_Temperature(void);

#ifdef __cplusplus
}
#endif

#endif /* HEATM_H */