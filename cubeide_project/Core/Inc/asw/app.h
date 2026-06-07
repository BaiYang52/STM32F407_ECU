/**
 * @file dim.h
 * @brief DIM (DIgital iMage) SWC — LED 控制, 按键采集
 * @version 1.0.0
 *
 * Runnable:
 *   DIM_Run_LED_Breath()          — 10ms, LED 呼吸灯
 *   DIM_Run_KeyAndVehDetect()     — 100ms, 按键状态采集 + 车速信号检测
 *   DIM_Run_IGNDetect()           — 1000ms, IGN 丢失事件检测
 *
 * SR/CS 接口:
 *   CS: Rte_Read_VehicleCtrl_Port_LED_Brightness_Level  (LED 亮度等级)
 *   CS: Rte_Read_VehicleCtrl_Port_LED_Switch_Cmd         (LED 开关)
 *   CS: Rte_Read_VehicleCtrl_Port_IGN_Status             (IGN 状态)
 *   CS: Rte_Read_VehicleCtrl_Port_Veh_Speed              (车速, 前置条件)
 *   SR: Dio_ReadChannel(DIO_CH_KEY1)                      (按键 1)
 *   SR: Dio_ReadChannel(DIO_CH_KEY0)                      (按键 2)
 *   SR: Rte_Write_EcuStatus_Port_Button_1_Status          (按键 1 状态)
 *   SR: Rte_Write_EcuStatus_Port_Button_2_Status          (按键 2 状态)
 *   SR: Rte_Write_EcuStatus_Port_LED_PWM_Duty             (LED 实际占空比)
 *   SR: Pwm_SetDutyPercent(PWM_CH_LED, duty)              (LED PWM 输出)
 *
 * 对应需求:
 *   #4 IGN_Status → 0x210 丢失前置条件 → 丢失事件
 *   #5 Veh_Speed → 31 01 02 03 前置条件 → 通过才能进 10 02 boot
 *   #6 KEY1/KEY0 按键状态 (0:未按, 1:按下, 2:无效/长按>30s)
 */

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* ==================== DIM 状态枚举 ==================== */

/** 按键状态 */
typedef enum
{
    e_KEY_NOT_PRESSED = 0U,   /**< 未按下 */
    e_KEY_PRESSED     = 1U,   /**< 按下 */
    e_KEY_INVALID     = 2U    /**< 无效 (卡滞/长按超过 30s) */
} APP_KeyStateType;

// typedef enum
// {
//     e_LVL_LOW  = 0U,      /**< 低电平 (0V) */
//     e_LVL_HIGH = 1U       /**< 高电平 (3.3V) */
// } Key_LevelType;

/** IGN 丢失事件原因 */
typedef enum
{
    e_IGN_NO_EVENT       = 0U,  /**< 无事件 */
    e_IGN_LOST_VEH_SPEED = 1U,  /**< 车速丢失 */
    e_IGN_LOST_FRAME     = 2U   /**< 报文丢失 */
} DIM_IgnEventType;

typedef enum
{
    e_KEY1 = 0U,
    e_KEY0 = 1U
}Key_Type;

/* ==================== DIM 公开变量声明 ==================== */

/* ==================== Runnable 声明 ==================== */

void APP_MainFunction(void);

/* switch cmd type of Received Messages */
typedef enum
{
    cmd_off = 0U,
    cmd_on = 1U
}CmdType;

#define Read_LED_Brightness_Level_From_RxMessage Rte_Read_VehicleCtrl_Port_LED_Brightness_Level
#define Read_LED_Switch_Cmd_From_RxMessage Rte_Read_VehicleCtrl_Port_LED_Switch_Cmd
#define Write_LED_PWM_Duty_To_TxMessage Rte_Write_EcuStatus_Port_LED_PWM_Duty
#define Set_LED_PWM_DutyPercent_ByPwmDriver Rte_Pwm_SetDutyPercent_LED
#define Get_Signal_State_ByCom Rte_Read_VehicleCtrl_Signal_State
// #define Read_
#define Write_Button_1_Status_To_TxMessage Rte_Write_EcuStatus_Port_Button_1_Status
#define Write_Button_2_Status_To_TxMessage Rte_Write_EcuStatus_Port_Button_2_Status
#ifdef __cplusplus
}
#endif

#endif /* APP_H */
