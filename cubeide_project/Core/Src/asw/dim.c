/**
 * @file dim.c
 * @brief DIM (DIgital iMage) SWC — LED 控制, 按键采集实现
 * @version 1.0.0
 *
 * Runnable 调用链:
 *   10ms:  DIM_Run_LED_Breath()          → LED 呼吸灯
 *   100ms: DIM_Run_KeyAndVehDetect()     → 按键采集 + 车速检测
 *   1000ms:DIM_Run_IGNDetect()           → IGN 丢失检测
 *
 * 需求映射:
 *   #1 Com_ReadSignal LED_Brightness_Level → 设置 LED PWM duty, on/off → Pwm_SetDutyPercent → Com_WriteSignal LED_PWM_Duty
 *   #4 IGN_Status → 0x210 丢失前置条件 → 丢失事件
 *   #5 Veh_Speed → 31 01 02 03 前置条件 → 通过才能进 10 02 boot
 *   #6 KEY1/KEY0 按键状态 (0:未按, 1:按下, 2:无效/长按>30s)
 */

#include "E:\Project\Github\STM32F407_ECU\cubeide_project\Core\Inc\asw\dim.h"
#include "E:\Project\Github\STM32F407_ECU\cubeide_project\Core\Inc\rte\rte_interface.h"
#include "gpio_driver.h"
#include "common.h"

/* ==================== 全局变量 ==================== */

DIM_KeyStateType g_DIM_Key1State = DIM_KEY_NOT_PRESSED;
DIM_KeyStateType g_DIM_Key2State = DIM_KEY_NOT_PRESSED;
uint8           g_DIM_IGNStatus  = 0U;
boolean         g_DIM_VehPrecondition = FALSE;

/* ==================== 按键去抖/计时 (内部) ==================== */

static uint32 s_key1PressTick = 0U;
static uint32 s_key2PressTick = 0U;
static uint32 s_ignLostTick   = 0U;

/* ==================== Runnable 实现 ==================== */

/**
 * @brief DIM — LED 呼吸灯 (10ms)
 *
 * 1. 通过 Com_ReadSignal 读取 LED_Brightness_Level (0-10 级)
 * 2. 读取 LED_Switch_Cmd (0=OFF, 1=ON)
 * 3. 若 ON: 计算占空比 = Brightness_Level * 10 (%)
 * 4. 写入 PWM 并反馈 LED_PWM_Duty 到 Com
 */
void Dim_MainFunction(void)
{
    uint8 brightlev = 0U;
    CmdType switchCmd  = cmd_off;
    uint8 pwmDuty    = 0U;

    //if not receive vehicle control message, turn off LED
     if (Read_LED_Switch_Cmd_From_RxMessage(&switchCmd) != STD_OK) {
         switchCmd = cmd_off;
         printf("lost\n");
     }
     else
     {
        /* 读取 LED 亮度等级 (0-10) */
        (void)Read_LED_Brightness_Level_From_RxMessage(&brightlev);
        /* 读取 LED 开关命令 */
        (void)Read_LED_Switch_Cmd_From_RxMessage(&switchCmd);
     }
     printf("brightlev%d switchCmd%d\n",brightlev,switchCmd);
    if (switchCmd == cmd_on) {
        /* 亮度等级 → 百分比 */
        brightlev = brightlev > 10U? 10U : brightlev;
        pwmDuty = (uint8)((uint16)brightlev * 10U); /* 0-10 → 0-100% */
    } else {
        pwmDuty = 0U;
    }

    /* 设置 PWM (通过 RTE interface) */
    Rte_Pwm_SetDutyPercent_LED(pwmDuty);

    /* 反馈实际占空比到 Com */
    (void)Write_LED_PWM_Duty_To_TxMessage(pwmDuty);
}

/**
 * @brief DIM — 按键采集 + 车速检测 (100ms)
 *
 * 1. 读取 KEY1/KEY0 引脚状态
 * 2. 去抖/长按判定 (30s → 无效)
 * 3. 写入 Button_1_Status / Button_2_Status 到 Com
 * 4. 读取 Veh_Speed 检查编程前置条件 (车速<5km/h)
 */
void DIM_Run_KeyAndVehDetect(void)
{
    uint32 now = HAL_GetTick();

    /* ── KEY1 ── */
    if (Dio_ReadChannel(DIO_CH_KEY1) == DIO_LVL_LOW) {
        if (g_DIM_Key1State == DIM_KEY_NOT_PRESSED) {
            /* 第一次按下 */
            s_key1PressTick = now;
            g_DIM_Key1State = DIM_KEY_PRESSED;
        } else if (g_DIM_Key1State == DIM_KEY_PRESSED) {
            /* 长按检查: >30s → 无效 */
            if ((now - s_key1PressTick) >= 30000U) {
                g_DIM_Key1State = DIM_KEY_INVALID;
            }
        }
    } else {
        /* 释放 */
        if (g_DIM_Key1State == DIM_KEY_PRESSED || g_DIM_Key1State == DIM_KEY_INVALID) {
            g_DIM_Key1State = DIM_KEY_NOT_PRESSED;
            s_key1PressTick = 0U;
        }
    }

    /* ── KEY2 ── */
    if (Dio_ReadChannel(DIO_CH_KEY0) == DIO_LVL_LOW) {
        if (g_DIM_Key2State == DIM_KEY_NOT_PRESSED) {
            s_key2PressTick = now;
            g_DIM_Key2State = DIM_KEY_PRESSED;
        } else if (g_DIM_Key2State == DIM_KEY_PRESSED) {
            if ((now - s_key2PressTick) >= 30000U) {
                g_DIM_Key2State = DIM_KEY_INVALID;
            }
        }
    } else {
        if (g_DIM_Key2State == DIM_KEY_PRESSED || g_DIM_Key2State == DIM_KEY_INVALID) {
            g_DIM_Key2State = DIM_KEY_NOT_PRESSED;
            s_key2PressTick = 0U;
        }
    }

    /* ── 写入 Com ── */
    (void)Rte_Write_EcuStatus_Port_Button_1_Status((uint8)g_DIM_Key1State);
    (void)Rte_Write_EcuStatus_Port_Button_2_Status((uint8)g_DIM_Key2State);

    /* ── Veh_Speed 前置条件检查 ── */
    float32 vehSpeed = 0.0f;
    if (Rte_Read_VehicleCtrl_Port_Veh_Speed(&vehSpeed) == STD_OK) {
        /* 车速 < 5km/h → 允许编程 */
        g_DIM_VehPrecondition = (vehSpeed < 5.0f) ? TRUE : FALSE;
    } else {
        g_DIM_VehPrecondition = FALSE;
    }
}

/**
 * @brief DIM — IGN 离线事件检测 (1000ms)
 *
 * 需求 #4: IGN_Status → 0x210 丢失前置条件 → 丢失事件
 */
void DIM_Run_IGNDetect(void)
{
    uint8 ignStatus = 0U;
    uint32 now = HAL_GetTick();

    if (Rte_Read_VehicleCtrl_Port_IGN_Status(&ignStatus) != STD_OK) {
        /* IGN 信号丢失 → 记录事件 */
        s_ignLostTick = now;
        g_DIM_IGNStatus = DIM_IGN_LOST_FRAME;
    } else {
        g_DIM_IGNStatus = (uint8)DIM_IGN_NO_EVENT;
        s_ignLostTick = 0U;
    }

    /* 如果是编程 boot 场景，还需要车速检测通过 */
    if (g_DIM_IGNStatus == DIM_IGN_NO_EVENT && !g_DIM_VehPrecondition) {
        g_DIM_IGNStatus = (uint8)DIM_IGN_LOST_VEH_SPEED;
    }
}
