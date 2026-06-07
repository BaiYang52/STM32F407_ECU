/**
 * @file rte_interface.c
 * @brief RTE Rte_Read/Rte_Write 实现
 * @version 1.0.0
 *
 * 每个 Rte_Read/Rte_Write 直接映射到 Com_ReadSignal/Com_WriteSignal。
 */

#include <rte/rte_interface.h>
#include "com.h"
#include "common.h"
#include "pwm_driver.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rng.h"

/* ============= MCAL Actuator Interfaces (SR ports) ============= */

void Rte_Pwm_SetDutyPercent_LED(uint8 duty)
{
    Pwm_SetDutyPercent(PWM_CH_LED, duty);
}

void Rte_Pwm_SetDutyPercent_Motor(uint8 duty)
{
    Pwm_SetDutyPercent(PWM_CH_MOTOR, duty);
}

/* ============= Vehicle Control Signals (RX from VCU) ============= */

Std_ReturnType Rte_Read_VehicleCtrl_Port_Veh_Speed(float32 *Data)
{
    if (Data == NULL_PTR) return STD_NOT_OK;
    return Com_ReadSignal(COM_SIG_VEH_SPEED, (void *)Data);
}

Std_ReturnType Rte_Read_VehicleCtrl_Port_IGN_Status(uint8 *Data)
{
    if (Data == NULL_PTR) return STD_NOT_OK;
    return Com_ReadSignal(COM_SIG_IGN_STATUS, (void *)Data);
}

Std_ReturnType Rte_Read_VehicleCtrl_Port_Motor_Switch_Cmd(uint8 *Data)
{
    if (Data == NULL_PTR) return STD_NOT_OK;
    return Com_ReadSignal(COM_SIG_MOTOR_SWITCH_CMD, (void *)Data);
}

Std_ReturnType Rte_Read_VehicleCtrl_Port_LED_Switch_Cmd(uint8 *Data)
{
    if (Data == NULL_PTR) return STD_NOT_OK;
    return Com_ReadSignal(COM_SIG_LED_SWITCH_CMD, (void *)Data);
}

Std_ReturnType Rte_Read_VehicleCtrl_Port_LED_Brightness_Level(uint8 *Data)
{
    if (Data == NULL_PTR) return STD_NOT_OK;
    return Com_ReadSignal(COM_SIG_LED_BRIGHTNESS_LEVEL, (void *)Data);
}

Std_ReturnType Rte_Read_VehicleCtrl_Signal_State(uint8 *state)
{
    if (state == NULL_PTR) return STD_NOT_OK;
    return Com_GetSignalState(COM_SIG_LED_SWITCH_CMD, (void *)state);
}

/* ============= ECU Status Signals (TX to VCU) ============= */

Std_ReturnType Rte_Write_EcuStatus_Port_Button_1_Status(uint8 Data)
{
    Com_WriteSignal(COM_SIG_BUTTON1_STATUS, (void *)&Data);
    return STD_OK;
}

Std_ReturnType Rte_Write_EcuStatus_Port_Button_2_Status(uint8 Data)
{
    Com_WriteSignal(COM_SIG_BUTTON2_STATUS, (void *)&Data);
    return STD_OK;
}

Std_ReturnType Rte_Write_EcuStatus_Port_Sys_Voltage(uint8 Data)
{
    Com_WriteSignal(COM_SIG_SYS_VOLTAGE, (void *)&Data);
    return STD_OK;
}

Std_ReturnType Rte_Write_EcuStatus_Port_ECU_Temperature(uint8 Data)
{
    Com_WriteSignal(COM_SIG_ECU_TEMP, (void *)&Data);
    return STD_OK;
}

Std_ReturnType Rte_Write_EcuStatus_Port_LED_PWM_Duty(uint8 Data)
{
    Com_WriteSignal(COM_SIG_LED_PWM_DUTY, (void *)&Data);
    return STD_OK;
}

/* ============= ECU Life Cycle Signals ============= */

Std_ReturnType Rte_Write_EcuLifeCycle_Port_Flash_Counter(uint16 Data)
{
    Com_WriteSignal(COM_SIG_FLASH_COUNTER, (void *)&Data);
    return STD_OK;
}

Std_ReturnType Rte_Write_EcuLifeCycle_Port_ECU_Error_Code(uint8 Data)
{
    Com_WriteSignal(COM_SIG_ECU_ERROR_CODE, (void *)&Data);
    return STD_OK;
}

/* ============= Network Management Signals ============= */

Std_ReturnType Rte_Read_EcuNM_Port_NM_Wakeup_Reason(uint8 *Data)
{
    if (Data == NULL_PTR) return STD_NOT_OK;
    return Com_ReadSignal(COM_SIG_NM_WAKE_REASON, (void *)Data);
}

Dio_LevelType Rte_Read_Key1_State(void)
{
    return Dio_ReadChannel(DIO_CH_KEY1) ;
}

Dio_LevelType Rte_Read_Key0_State(void)
{
    return Dio_ReadChannel(DIO_CH_KEY0) ;
}

/* ============= MCAL Random Number Generator Interface ============= */

Std_ReturnType Rte_Hal_Rng_GenerateRandomNumber(uint32 *RandomValue_Ptr)
{
    if (NULL_PTR == RandomValue_Ptr)
    {
        return STD_NOT_OK;
    }

    RNG_HandleTypeDef hrng;
    HAL_StatusTypeDef hal_status;

    /* 获取RNG HAL驱动默认实例 */
    hrng.Instance = RNG;

    /* 使能RNG时钟 */
    __HAL_RCC_RNG_CLK_ENABLE();

    /* 初始化RNG */
    hal_status = HAL_RNG_Init(&hrng);
    if (HAL_OK != hal_status)
    {
        return STD_NOT_OK;
    }

    /* 生成32位硬件随机数 */
    hal_status = HAL_RNG_GenerateRandomNumber(&hrng, RandomValue_Ptr);
    if (HAL_OK != hal_status)
    {
        HAL_RNG_DeInit(&hrng);
        return STD_NOT_OK;
    }

    return STD_OK;
}

/* ============= MCAL System Tick Interface ============= */

uint32 Rte_Hal_GetTick(void)
{
    return HAL_GetTick();
}