/**
 * @file fan.h
 * @brief FAN (FAN control) SWC — 电机 PWM 控制, 方向切换
 * @version 1.0.0
 *
 * Runnable:
 *   FAN_Run_MotorPWM()     — 10ms, 电机 PWM 更新
 *   FAN_Run_MotorDir()     — 100ms, 电机方向控制
 *
 * SR/CS 接口:
 *   CS: Rte_Read_VehicleCtrl_Port_Motor_Switch_Cmd    (电机开关命令)
 *   CS: Rte_Read_VehicleCtrl_Port_Veh_Speed           (车速, 影响方向)
 *   SR: Pwm_SetDutyPercent(PWM_CH_MOTOR, duty)         (电机 PWM)
 *   SR: Pwm_SetMotorDirection(Forward)                 (电机方向)
 *   SR: Dio_WriteChannel(DIO_CH_MOTOR_DIR, level)      (电机方向 GPIO)
 *
 * 对应需求:
 *   #2 Motor_Switch_Cmd → 设置电机 PWM on/off
 */

#ifndef FAN_H
#define FAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* ==================== FAN 状态枚举 ==================== */

/** 电机开关命令 */
typedef enum
{
    FAN_CMD_OFF   = 0U,    /**< 关闭 */
    FAN_CMD_ON    = 1U,    /**< 开启 */
    FAN_CMD_BRAKE = 2U     /**< 抱闸 */
} FAN_MotorCmdType;

/* ==================== FAN 公开变量声明 ==================== */

/** 电机当前命令 (来自 0x210 RX) */
extern FAN_MotorCmdType g_FAN_MotorCmd;

/** 电机当前占空比 (0-100%) */
extern uint8 g_FAN_MotorDuty;

/* ==================== Runnable 声明 ==================== */

void FAN_Run_MotorPWM(void);
void FAN_Run_MotorDir(void);

#ifdef __cplusplus
}
#endif

#endif /* FAN_H */