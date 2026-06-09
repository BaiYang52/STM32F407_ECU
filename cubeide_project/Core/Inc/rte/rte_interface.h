/**
 * @file rte_interface.h
 * @brief RTE Rte_Read/Rte_Write接口定义
 * @version 1.0.0
 * @date 2024-01-01
 */

#ifndef RTE_INTERFACE_H
#define RTE_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "types.h"
#include "gpio_driver.h"

    /* ============= MCAL Actuator Interfaces (SR ports) ============= */

    /**
     * @brief 设置 LED PWM 占空比 (RTE → MCAL Pwm)
     * @param[in] duty 占空比 (0-100%)
     */
    void Rte_Pwm_SetDutyPercent_LED(uint8 duty);

    /**
     * @brief 设置电机 PWM 占空比 (RTE → MCAL Pwm)
     * @param[in] duty 占空比 (0-100%)
     */
    void Rte_Pwm_SetDutyPercent_Motor(uint8 duty);

    /* ============= Vehicle Control Signals (RX from VCU) ============= */

    /**
     * @brief 读取车辆控制信号 - 车速
     * @param[out] Data 车速数据指针 (单位: 0.01 km/h)
     * @return Std_ReturnType STD_OK或STD_NOT_OK
     */
    Std_ReturnType Rte_Read_VehicleCtrl_Port_Veh_Speed(float32 *Data);

    /**
     * @brief 读取车辆控制信号 - 点火状态
     * @param[out] Data 点火状态 (0:OFF, 1:ACC, 2:ON, 3:CRANK)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Read_VehicleCtrl_Port_IGN_Status(uint8 *Data);

    /**
     * @brief 读取车辆控制信号 - 马达开关命令
     * @param[out] Data 马达命令 (0:关, 1:开, 2:抱闸)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Read_VehicleCtrl_Port_Motor_Switch_Cmd(uint8 *Data);

    /**
     * @brief 读取车辆控制信号 - LED开关命令
     * @param[out] Data LED命令 (0:关, 1:开)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Read_VehicleCtrl_Port_LED_Switch_Cmd(uint8 *Data);

    /**
     * @brief 读取车辆控制信号 - LED亮度等级
     * @param[out] Data LED亮度 (0-10级)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Read_VehicleCtrl_Port_LED_Brightness_Level(uint8 *Data);

    /* ============= ECU Status Signals (TX to VCU) ============= */

    /**
     * @brief 写入ECU状态信号 - 按键1状态
     * @param[in] Data 按键状态 (0:未按, 1:按下, 2:无效)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Write_EcuStatus_Port_Button_1_Status(uint8 Data);

    /**
     * @brief 写入ECU状态信号 - 按键2状态
     * @param[in] Data 按键状态
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Write_EcuStatus_Port_Button_2_Status(uint8 Data);

    /**
     * @brief 写入ECU状态信号 - 系统电压
     * @param[in] Data 系统电压 (单位: V, 公式: Y = X * 0.1)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Write_EcuStatus_Port_Sys_Voltage(uint8 Data);

    /**
     * @brief 写入ECU状态信号 - ECU温度
     * @param[in] Data ECU温度 (单位: ℃, 公式: Y = X * 1 - 40)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Write_EcuStatus_Port_ECU_Temperature(uint8 Data);

    /**
     * @brief 写入ECU状态信号 - LED PWM占空比
     * @param[in] Data LED PWM占空比 (单位: %, 公式: Y = X * 0.4)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Write_EcuStatus_Port_LED_PWM_Duty(uint8 Data);

    /**
     * @brief 写入ECU生命周期信号 - Flash刷写次数
     * @param[in] Data Flash计数器 (0-65535)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Write_EcuLifeCycle_Port_Flash_Counter(uint16 Data);

    /**
     * @brief 写入ECU生命周期信号 - ECU错误码
     * @param[in] Data ECU错误码 (0:按键卡滞, 1:Busoff, 2:温度过高)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Write_EcuLifeCycle_Port_ECU_Error_Code(uint8 Data);

    /* ============= Network Management Signals ============= */

    /**
     * @brief 读取网络管理信号 - 唤醒原因
     * @param[out] Data 唤醒原因 (0:无效, 1:NM唤醒, 2:IGN唤醒, 3:按键唤醒)
     * @return Std_ReturnType
     */
    Std_ReturnType Rte_Read_EcuNM_Port_NM_Wakeup_Reason(uint8 *Data);

    Std_ReturnType Rte_Read_VehicleCtrl_Signal_State(uint8 *state);

    Dio_LevelType Rte_Read_Key1_State(void);
    Dio_LevelType Rte_Read_Key0_State(void);

    /* ============= MCAL Random Number Generator Interface ============= */

    /**
     * @brief 通过HAL RNG生成32位硬件随机数 (RTE → MCAL RNG)
     * 
     * 该接口封装STM32硬件随机数发生器, 为安全种子生成、密钥协商等
     * 提供真随机数源。
     * 
     * @param[out] RandomValue_Ptr  指向存放32位随机数的内存
     * @return Std_ReturnType  STD_OK 生成成功, STD_NOT_OK 生成失败
     */
    Std_ReturnType Rte_Hal_Rng_GenerateRandomNumber(uint32 *RandomValue_Ptr);

    /* ============= MCAL System Tick Interface ============= */

    /**
     * @brief 获取系统Tick计数值 (RTE → MCAL SysTick)
     * 
     * 封装HAL_GetTick(), 返回系统自启动以来的毫秒计数值。
     * 用于超时判断、时间戳记录等场景。
     * 
     * @return uint32  系统毫秒计数值 (自启动起的ms数)
     */
    uint32 Rte_Hal_GetTick(void);

    /* ============= MCAL System Control Interface ============= */

    /**
     * @brief 触发MCU系统复位 (RTE → MCAL NVIC_SystemReset)
     * 
     * 封装CMSIS NVIC_SystemReset(), 执行后MCU立即复位。
     * 该函数不会返回。
     */
    void Rte_Hal_SystemReset(void);

#ifdef __cplusplus
}
#endif

#endif /* RTE_INTERFACE_H */
