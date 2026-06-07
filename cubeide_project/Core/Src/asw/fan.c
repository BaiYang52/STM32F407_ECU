/**
 * @file fan.c
 * @brief FAN (FAN control) SWC — 电机 PWM 控制, 方向切换实现
 * @version 1.0.0
 *
 * Runnable 调用链:
 *   10ms:  FAN_Run_MotorPWM()     → 根据 Motor_Switch_Cmd 更新电机 PWM
 *   100ms: FAN_Run_MotorDir()     → 根据 Veh_Speed 更新电机方向
 *
 * 需求映射:
 *   #2 Motor_Switch_Cmd → 设置电机 PWM on/off → Pwm_SetDutyPercent
 */

#include <asw/fan.h>
#include <rte/rte_interface.h>
#include "pwm_driver.h"
#include "gpio_driver.h"
#include "common.h"

/* ==================== 全局变量 ==================== */

FAN_MotorCmdType g_FAN_MotorCmd  = FAN_CMD_OFF;
uint8            g_FAN_MotorDuty = 0U;

/* ==================== Runnable 实现 ==================== */

/**
 * @brief FAN — 电机 PWM 更新 (10ms)
 *
 * 需求 #2:
 *   读取 Motor_Switch_Cmd (0:OFF, 1:ON, 2:Brake)
 *   OFF/Brake → PWM 0%
 *   ON        → 固定 80% 占空比
 */
void FAN_Run_MotorPWM(void)
{
    uint8 motorCmd = 0U;

    (void)Rte_Read_VehicleCtrl_Port_Motor_Switch_Cmd(&motorCmd);

    if (motorCmd == 1U) {
        /* 开机 */
        g_FAN_MotorDuty = 80U;
    } else {
        /* 关闭 / 抱闸 */
        g_FAN_MotorDuty = 0U;
    }

    g_FAN_MotorCmd = (FAN_MotorCmdType)motorCmd;

    /* 设置 PWM */
    Pwm_SetDutyPercent(PWM_CH_MOTOR, g_FAN_MotorDuty);
}

/**
 * @brief FAN — 电机方向控制 (100ms)
 *
 * 根据 Veh_Speed 方向:
 *   若车速 > 0: 正转
 *   若车速 < 0: 反转 (实际应用中车速为非负)
 *   此处简化为 Motor_Switch_Cmd 的 Brake 时切换方向
 */
void FAN_Run_MotorDir(void)
{
    if (g_FAN_MotorCmd == FAN_CMD_BRAKE) {
        /* 抱闸时反转 */
        Pwm_SetMotorDirection(FALSE);
    } else {
        Pwm_SetMotorDirection(TRUE);
    }
}
