/**
 * @file app_task_scheduler.c
 * @brief 应用任务调度器实现
 * @version 1.0.0
 * @date 2024-01-01
 */

#include "app_task_scheduler.h"
#include "bsw_cannm.h"
#include "bsw_dcm.h"
#include "bsw_dem.h"
#include "bsw_nvm.h"
#include "common.h"
#include "rte.h"

/* ============= Defines ============= */

#define APP_TASK_PERIOD_10MS 10U
#define APP_TASK_PERIOD_100MS 100U
#define APP_TASK_PERIOD_1000MS 1000U

/* ============= Private Global Variables ============= */

/** 10ms任务计数器 */
static uint32 s_task10msCounter = 0U;

/** 100ms任务计数器 */
static uint32 s_task100msCounter = 0U;

/** 1s任务计数器 */
static uint32 s_task1000msCounter = 0U;

/** 10ms任务执行标志 */
static uint8 s_task10msFlag = 0U;

/** 100ms任务执行标志 */
static uint8 s_task100msFlag = 0U;

/** 1s任务执行标志 */
static uint8 s_task1000msFlag = 0U;

/* ============= Function Implementations ============= */

/**
 * @brief 初始化任务调度器
 */
void App_TaskScheduler_Init(void)
{
    s_task10msCounter = 0U;
    s_task100msCounter = 0U;
    s_task1000msCounter = 0U;
    s_task10msFlag = 0U;
    s_task100msFlag = 0U;
    s_task1000msFlag = 0U;
}

/**
 * @brief SysTick中断处理 (1ms调用一次)
 * @details 由硬件定时器中断触发，维护任务计数器
 */
void App_TaskScheduler_SystickHandler(void)
{
    /* 10ms任务计数 */
    s_task10msCounter++;
    if (s_task10msCounter >= APP_TASK_PERIOD_10MS)
    {
        s_task10msCounter = 0U;
        s_task10msFlag = 1U;
    }

    /* 100ms任务计数 */
    s_task100msCounter++;
    if (s_task100msCounter >= APP_TASK_PERIOD_100MS)
    {
        s_task100msCounter = 0U;
        s_task100msFlag = 1U;
    }

    /* 1s任务计数 */
    s_task1000msCounter++;
    if (s_task1000msCounter >= APP_TASK_PERIOD_1000MS)
    {
        s_task1000msCounter = 0U;
        s_task1000msFlag = 1U;
    }
}

/**
 * @brief 10ms周期任务
 * @details
 *   - 运行底层ADC采样（温度、按键电平滤波）
 *   - 调用RTE接口读写信号
 *   - 执行ASW模型步进函数
 */
void App_Task_10ms(void)
{
    uint8 adcTemp;
    uint8 buttonStatus;
    uint8 motorCmd;
    uint8 ledCmd;
    uint8 ledBrightness;
    float32 vehSpeed;
    uint8 ignStatus;

    if (s_task10msFlag == 0U)
    {
        return;
    }
    s_task10msFlag = 0U;

    /* ===== 输入采样 ===== */

    /* ADC采样温度和按键 */
    adcTemp = Mcal_Adc_GetChannelValue(ADC_CHANNEL_TEMP);
    buttonStatus = Mcal_Gpio_ReadPin(GPIO_PORT_KEY0);

    /* 通过RTE读取来自VCU的控制信号 */
    Rte_Read_VehicleCtrl_Port_Motor_Switch_Cmd(&motorCmd);
    Rte_Read_VehicleCtrl_Port_LED_Switch_Cmd(&ledCmd);
    Rte_Read_VehicleCtrl_Port_LED_Brightness_Level(&ledBrightness);
    Rte_Read_VehicleCtrl_Port_Veh_Speed(&vehSpeed);
    Rte_Read_VehicleCtrl_Port_IGN_Status(&ignStatus);

    /* ===== Simulink模型步进 ===== */

    /* 执行10ms应用模型 (由Simulink自动生成) */
    App_MotorControl_Step_10ms();
    App_OverspeedMonitor_Step_10ms();

    /* ===== 输出写入 ===== */

    /* 将计算结果通过RTE写回到报文缓冲区 */
    Rte_Write_EcuStatus_Port_Button_1_Status(buttonStatus);
    Rte_Write_EcuStatus_Port_ECU_Temperature(adcTemp);
    Rte_Write_EcuStatus_Port_LED_PWM_Duty((uint8)(ledBrightness * 8)); /* 线性映射到PWM */
}

/**
 * @brief 100ms周期任务
 * @details
 *   - 执行故障状态机Dem诊断监控逻辑
 *   - 检测按键卡滞、温度过高状态转移
 *   - 周期性状态上报
 */
void App_Task_100ms(void)
{
    uint8 buttonRaw;
    uint8 tempRaw;
    uint32 canMsgTimeout;

    if (s_task100msFlag == 0U)
    {
        return;
    }
    s_task100msFlag = 0U;

    /* ===== DEM故障监控 ===== */

    /* 监控按键卡滞故障 (保持按下>=30s) */
    buttonRaw = Mcal_Gpio_ReadPin(GPIO_PORT_KEY0);
    Bsw_Dem_ButtonStuckMonitor(buttonRaw);

    /* 监控温度过高故障 (>80℃) */
    tempRaw = Mcal_Adc_GetChannelValue(ADC_CHANNEL_TEMP);
    Bsw_Dem_TemperatureMonitor(tempRaw);

    /* 监控CAN报文丢失 (IGN_ON时超过2s未收到Vehicle_Ctrl报文) */
    Bsw_Dem_MessageLossMonitor();

    /* 监控CAN总线关闭 (BusOff) */
    Bsw_Dem_BusoffMonitor();

    /* ===== RTE和通信处理 ===== */

    /* 调用RTE主处理 (信号缓冲同步) */
    Rte_MainFunction();

    /* 处理诊断请求 */
    Bsw_Dcm_MainFunction();

    /* 处理网络管理 */
    Bsw_CanNm_MainFunction();
}

/**
 * @brief 1s周期任务
 * @details
 *   - NVM数据异步写入
 *   - 系统运行日志存储
 *   - 健康管理监控
 */
void App_Task_1000ms(void)
{
    if (s_task1000msFlag == 0U)
    {
        return;
    }
    s_task1000msFlag = 0U;

    /* ===== NVM处理 ===== */

    /* 异步写入DTC和DID到外部NVM (W25Q16) */
    Bsw_Nvm_MainFunction();

    /* ===== 健康管理 ===== */

    /* 定期检查系统健康状态 */
    /* 可在此添加运行日志、计时器等管理 */

    /* ===== 喂狗 ===== */

    /* 看门狗保护 */
    App_Watchdog_Feed();
}

/**
 * @brief 获取10ms任务标志
 */
uint8 App_TaskScheduler_Get10msFlag(void)
{
    return s_task10msFlag;
}

/**
 * @brief 获取100ms任务标志
 */
uint8 App_TaskScheduler_Get100msFlag(void)
{
    return s_task100msFlag;
}

/**
 * @brief 获取1s任务标志
 */
uint8 App_TaskScheduler_Get1000msFlag(void)
{
    return s_task1000msFlag;
}