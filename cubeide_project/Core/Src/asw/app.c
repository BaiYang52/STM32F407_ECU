/**
 * @file app.c
 * @brief APP (Application) SWC — 应用逻辑实现
 * @version 1.0.0
 *
 * Runnable 调用链:
 *   10ms:  APP_Run_Application()          → 应用逻辑	
 *   100ms: APP_Run_KeyAndVehDetect()     → 按键采集 + 车速检测
 *   1000ms:APP_Run_IGNDetect()           → IGN 丢失检测
 *
 * 需求映射:
 *   #1 Com_ReadSignal LED_Brightness_Level → 设置 LED PWM duty, on/off → Pwm_SetDutyPercent → Com_WriteSignal LED_PWM_Duty
 *   #4 IGN_Status → 0x210 丢失前置条件 → 丢失事件
 *   #5 Veh_Speed → 31 01 02 03 前置条件 → 通过才能进 10 02 boot
 *   #6 KEY1/KEY0 按键状态 (0:未按, 1:按下, 2:无效/长按>30s)
 */

#include <rte/rte_interface.h>
#include <asw/app.h>
#include "common.h"
#include "stdio.h"

/* ==================== 全局变量 ==================== */

// APP_KeyStateType g_Key1State;
// APP_KeyStateType g_Key2State;
// uint8           g_IGNStatus;
// boolean         g_VehPrecondition = FALSE;

void APP_MainFunction(void);
void APP_Run_KeyState(void);
APP_KeyStateType APP_Get_Key1DebounceState(void);
APP_KeyStateType APP_Get_Key0DebounceState(void);
#define KEY_STUCK_DEBOUNCE 30000U
#define KEY_UNPRESSED_STATE 0x07U /* 0b 0000 0111 30ms debounce time */
/* ==================== Runnable 实现 ==================== */

/**
 * @brief APP — 应用逻辑 (10ms)
 *
 * 1. 通过 Com_ReadSignal 读取 LED_Brightness_Level (0-10 级)
 * 2. 读取 LED_Switch_Cmd (0=OFF, 1=ON)
 * 3. 若 ON: 计算占空比 = Brightness_Level * 10 (%)
 * 4. 写入 PWM 并反馈 LED_PWM_Duty 到 Com
 */
void APP_MainFunction(void)
{
    APP_Run_KeyState();
}

/**
 * @brief APP — 按键采集 + 车速检测 (100ms)
 *
 * 1. 读取 KEY1/KEY0 引脚状态
 * 2. 去抖/长按判定 (30s → 无效)
 * 3. 写入 Button_1_Status / Button_2_Status 到 Com
 * 4. 读取 Veh_Speed 检查编程前置条件 (车速<5km/h)
 */
void APP_Run_KeyState(void)
{
   (void)Write_Button_1_Status_To_TxMessage((uint8)APP_Get_Key1DebounceState());
   (void)Write_Button_2_Status_To_TxMessage((uint8)APP_Get_Key0DebounceState());
   
}

/**
 * @brief K1/K0 按键状态消抖
 *
 * 
 */
APP_KeyStateType APP_Get_Key1DebounceState(void)
{
	static uint16 u_KeyPressTime = 0U; /* 初始按压时间: 0*10ms =0ms */
	static uint8 u_KeyBitState = KEY_UNPRESSED_STATE; /* 初始状态: 0b00000111 (未按) */
	static APP_KeyStateType lastKeyState = e_KEY_NOT_PRESSED;
	uint8 xorval = 0x01;
	u_KeyBitState <<= 1;
	xorval = (uint8)Rte_Read_Key1_State();
	u_KeyBitState = (u_KeyBitState|xorval) & KEY_UNPRESSED_STATE; /* 读 KEY1 状态 */
	//u_KeyBitState = (u_KeyBitState|(uint8)Rte_Read_Key1_State()) & 0x07U; /* 读 KEY1 状态 */

	if (u_KeyBitState == 0x00U) {
		u_KeyPressTime++;
		if (u_KeyPressTime >= KEY_STUCK_DEBOUNCE) { /* 长按判定: 100*10ms = 1000ms = 1s */
			u_KeyPressTime = KEY_STUCK_DEBOUNCE; /* 防止溢出 */
			return lastKeyState = e_KEY_INVALID; /* 长按超过 1s，判定为无效 */
		}
		else {
			return lastKeyState = e_KEY_PRESSED; /* 按下 */
		}
	}
	else if (u_KeyBitState == KEY_UNPRESSED_STATE) {
		u_KeyPressTime = 0U; /* 释放，复位按压时间 */
		return lastKeyState = e_KEY_NOT_PRESSED;
	}
	else {
		return lastKeyState;
	}
}

APP_KeyStateType APP_Get_Key0DebounceState(void)
{
	static uint16 u_KeyPressTime = 0U; /* 初始按压时间: 0*10ms =0ms */
	static uint8 u_KeyBitState = KEY_UNPRESSED_STATE; /* 初始状态: 0b00000111 (未按) */
	static APP_KeyStateType lastKeyState = e_KEY_NOT_PRESSED;
	uint8 xorval = 0x01;
	u_KeyBitState <<= 1;
	xorval = (uint8)Rte_Read_Key0_State();
	u_KeyBitState = (u_KeyBitState|xorval) & KEY_UNPRESSED_STATE; /* 读 KEY1 状态 */
	//u_KeyBitState = (u_KeyBitState|(uint8)Rte_Read_Key0_State()) & 0x07U; /* 读 KEY1 状态 */

	if (u_KeyBitState == 0x00U) {
		u_KeyPressTime++;
		if (u_KeyPressTime >= KEY_STUCK_DEBOUNCE) { /* 长按判定: 100*10ms = 1000ms = 1s */
			u_KeyPressTime = KEY_STUCK_DEBOUNCE; /* 防止溢出 */
			printf("u_KeyPressTime:%d",u_KeyPressTime);
			return lastKeyState = e_KEY_INVALID; /* 长按超过 1s，判定为无效 */
		}
		else {
			return lastKeyState = e_KEY_PRESSED; /* 按下 */
		}
	}
	else if (u_KeyBitState == KEY_UNPRESSED_STATE) {
		u_KeyPressTime = 0U; /* 释放，复位按压时间 */
		return lastKeyState = e_KEY_NOT_PRESSED;
	}
	else {
		return lastKeyState;
	}
}

/**
 * @brief DIM — IGN 离线事件检测 (1000ms)
 *
 * 需求 #4: IGN_Status → 0x210 丢失前置条件 → 丢失事件
 */
//void DIM_Run_IGNDetect(void)
//{
//    uint8 ignStatus = 0U;
//    uint32 now = HAL_GetTick();
//
//    if (Rte_Read_VehicleCtrl_Port_IGN_Status(&ignStatus) != STD_OK) {
//        /* IGN 信号丢失 → 记录事件 */
//        s_ignLostTick = now;
//        g_DIM_IGNStatus = DIM_IGN_LOST_FRAME;
//    } else {
//        g_DIM_IGNStatus = (uint8)DIM_IGN_NO_EVENT;
//        s_ignLostTick = 0U;
//    }
//
//    /* 如果是编程 boot 场景，还需要车速检测通过 */
//    if (g_DIM_IGNStatus == DIM_IGN_NO_EVENT && !g_DIM_VehPrecondition) {
//        g_DIM_IGNStatus = (uint8)DIM_IGN_LOST_VEH_SPEED;
//    }
//}
