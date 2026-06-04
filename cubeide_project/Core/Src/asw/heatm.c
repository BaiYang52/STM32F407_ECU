/**
 * @file heatm.c
 * @brief HEATM (HEAT Manager) SWC — DS18B20 温度采集与上报实现
 * @version 1.0.0
 *
 * Runnable 调用链:
 *   1000ms: HEATM_Run_Temperature()  → 读取 DS18B20 → 写入 ECU_Temperature
 *
 * 需求映射:
 *   #3 DS18B20 读取温度 → 设置 ECU_Temperature
 */

#include "E:\Project\Github\STM32F407_ECU\cubeide_project\Core\Inc\asw\heatm.h"
#include "E:\Project\Github\STM32F407_ECU\cubeide_project\Core\Inc\rte\rte_interface.h"
#include "test_heat.h"     /* Mcal_DS18B20_ReadTemperature */
#include "common.h"

/* ==================== 全局变量 ==================== */

float32 g_HEATM_TemperatureC = 0.0f;

/* ==================== Runnable 实现 ==================== */

/**
 * @brief HEATM — 温度采集与上报 (1000ms)
 *
 * 需求 #3:
 *   1. 调用 Mcal_DS18B20_ReadTemperature 获取温度
 *   2. 转换为 ECU_Temperature 信号格式: Y = X * 1 - 40 (X in °C)
 *   3. 写入 Com (ECU_Status PDU, Byte4)
 */
void HEATM_Run_Temperature(void)
{
    float32 tempC = 0.0f;
    uint8   ecuTempRaw = 0U;
    sint16  tempInt;

    /* 读取 DS18B20 */
    if (Mcal_DS18B20_ReadTemperature(&tempC) == STD_OK) {
        g_HEATM_TemperatureC = tempC;

        /* 信号转换: Y = X - (-40), 但 DBC 中 ECU_Temperature 偏移量 -40, factor 1
         * 即物理值 = raw * 1 - 40
         * → raw = 物理值 + 40 */
        tempInt = (sint16)(tempC + 40.0f);
        if (tempInt < 0) {
            tempInt = 0;
        }
        if (tempInt > 255) {
            tempInt = 255;
        }
        ecuTempRaw = (uint8)tempInt;
    } else {
        /* 读取失败 → 保持上次值, 不更新 */
        return;
    }

    /* 写入 Com */
    (void)Rte_Write_EcuStatus_Port_ECU_Temperature(ecuTempRaw);
}
