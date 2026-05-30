# STM32F407 AUTOSAR ECU 代码模板与示例

## 1. MCAL 层代码示例

### 1.1 CAN驱动头文件模板 (inc/mcal/can/can_driver.h)

```c
/**
 * @file can_driver.h
 * @brief CAN驱动程序头文件
 * @version 1.0.0
 * @date 2024-01-01
 */

#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============= Includes ============= */
#include "types.h"
#include "can_types.h"
#include "stm32f4xx_hal.h"

/* ============= Defines ============= */

/**
 * @defgroup CAN_ID CAN消息ID定义
 * @{
 */
#define MCAL_CAN1_CHANNEL           0U
#define MCAL_CAN2_CHANNEL           1U

#define MCAL_CAN_RX_BUFFER_SIZE     256U
#define MCAL_CAN_TX_BUFFER_SIZE     64U
#define MCAL_CAN_MAX_DLC            8U
/** @} */

/* ============= Enums ============= */

/**
 * @enum Can_ChannelType
 * @brief CAN通道枚举
 */
typedef enum {
    CAN_CHANNEL_1 = 0,     /**< CAN1通道 */
    CAN_CHANNEL_2,         /**< CAN2通道 */
    CAN_CHANNEL_MAX
} Can_ChannelType;

/**
 * @enum Can_MessageStateType
 * @brief CAN消息状态枚举
 */
typedef enum {
    CAN_MSG_IDLE = 0,
    CAN_MSG_SENDING,
    CAN_MSG_SENT,
    CAN_MSG_SEND_FAILED
} Can_MessageStateType;

/* ============= Structures ============= */

/**
 * @struct Can_FrameType
 * @brief CAN帧结构体
 */
typedef struct {
    uint32 id;                      /**< CAN消息ID */
    uint8 dlc;                      /**< 数据长度代码 (0-8) */
    uint8 data[MCAL_CAN_MAX_DLC];  /**< 数据字节数组 */
    uint8 ide;                      /**< IDE标志 (0: 11bit, 1: 29bit) */
    uint8 rtr;                      /**< RTR标志 */
} Can_FrameType;

/**
 * @struct Can_ConfigType
 * @brief CAN驱动配置结构体
 */
typedef struct {
    uint32 baudrate;        /**< 波特率 (kbps): 125, 250, 500, 1000 */
    uint8 channel;          /**< CAN通道号 */
    uint8 rxFilterMode;     /**< 接收过滤模式 */
} Can_ConfigType;

/* ============= Public Functions ============= */

/**
 * @brief CAN驱动初始化
 * @param[in] Config CAN配置指针
 * @return Std_ReturnType
 *   @retval STD_OK    初始化成功
 *   @retval STD_NOT_OK 初始化失败
 * @details
 *   - 配置CAN硬件参数
 *   - 初始化发送/接收缓冲区
 *   - 使能CAN中断
 */
Std_ReturnType Mcal_Can_Init(const Can_ConfigType *Config);

/**
 * @brief CAN消息发送
 * @param[in] Channel CAN通道号
 * @param[in] Frame CAN帧指针
 * @return Std_ReturnType
 *   @retval STD_OK        发送请求成功
 *   @retval STD_NOT_OK    发送缓冲区满或其他错误
 * @details
 *   - 将消息放入发送队列
 *   - 硬件自动发送
 */
Std_ReturnType Mcal_Can_Send(uint8 Channel, const Can_FrameType *Frame);

/**
 * @brief CAN消息接收
 * @param[in] Channel CAN通道号
 * @param[out] Frame 接收帧指针
 * @return Std_ReturnType
 *   @retval STD_OK        接收缓冲区有新数据
 *   @retval STD_NOT_OK    接收缓冲区为空
 * @details
 *   - 从接收缓冲区取出消息
 */
Std_ReturnType Mcal_Can_Receive(uint8 Channel, Can_FrameType *Frame);

/**
 * @brief 获取CAN驱动状态
 * @param[in] Channel CAN通道号
 * @return Can_MessageStateType CAN当前状态
 */
Can_MessageStateType Mcal_Can_GetState(uint8 Channel);

/**
 * @brief 使能CAN接收器
 * @param[in] Channel CAN通道号
 */
void Mcal_Can_EnableReceiver(uint8 Channel);

/**
 * @brief 禁能CAN接收器
 * @param[in] Channel CAN通道号
 */
void Mcal_Can_DisableReceiver(uint8 Channel);

/**
 * @brief CAN接收中断处理程序
 */
void Mcal_Can_RxISR(void);

/**
 * @brief CAN发送中断处理程序
 */
void Mcal_Can_TxISR(void);

/**
 * @brief CAN错误中断处理程序
 */
void Mcal_Can_ErrorISR(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_DRIVER_H */
```

### 1.2 CAN驱动实现文件示例 (src/mcal/can/can_driver.c)

```c
/**
 * @file can_driver.c
 * @brief CAN驱动程序实现
 * @version 1.0.0
 * @date 2024-01-01
 */

#include "can_driver.h"
#include "common.h"

/* ============= Private Defines ============= */

#define CAN_RX_BUFFER_SIZE   256U
#define CAN_TX_BUFFER_SIZE   64U

/* ============= Private Data Types ============= */

/**
 * @struct Can_RxBufferType
 * @brief CAN接收缓冲区结构体
 */
typedef struct {
    Can_FrameType buffer[CAN_RX_BUFFER_SIZE];
    uint16 writeIndex;
    uint16 readIndex;
    uint16 count;
} Can_RxBufferType;

/**
 * @struct Can_TxBufferType
 * @brief CAN发送缓冲区结构体
 */
typedef struct {
    Can_FrameType buffer[CAN_TX_BUFFER_SIZE];
    uint16 writeIndex;
    uint16 readIndex;
    uint16 count;
} Can_TxBufferType;

/**
 * @struct Can_DriverStateType
 * @brief CAN驱动状态结构体
 */
typedef struct {
    uint8 initialized;
    Can_MessageStateType txState;
    Can_MessageStateType rxState;
    uint32 rxErrors;
    uint32 txErrors;
    uint32 busoffCounter;
} Can_DriverStateType;

/* ============= Private Global Variables ============= */

/** CAN接收缓冲区数组 (每个通道一个) */
static Can_RxBufferType s_canRxBuffer[CAN_CHANNEL_MAX];

/** CAN发送缓冲区数组 (每个通道一个) */
static Can_TxBufferType s_canTxBuffer[CAN_CHANNEL_MAX];

/** CAN驱动状态数组 */
static Can_DriverStateType s_canDriverState[CAN_CHANNEL_MAX] = {
    {0, CAN_MSG_IDLE, CAN_MSG_IDLE, 0, 0, 0},
    {0, CAN_MSG_IDLE, CAN_MSG_IDLE, 0, 0, 0}
};

/* ============= Private Function Declarations ============= */

static void Mcal_Can_BufferInit(uint8 Channel);
static Std_ReturnType Mcal_Can_PushRxBuffer(uint8 Channel, const Can_FrameType *Frame);
static Std_ReturnType Mcal_Can_PopRxBuffer(uint8 Channel, Can_FrameType *Frame);
static Std_ReturnType Mcal_Can_PushTxBuffer(uint8 Channel, const Can_FrameType *Frame);
static Std_ReturnType Mcal_Can_PopTxBuffer(uint8 Channel, Can_FrameType *Frame);

/* ============= Public Function Implementations ============= */

/**
 * @brief CAN驱动初始化
 */
Std_ReturnType Mcal_Can_Init(const Can_ConfigType *Config)
{
    CAN_HandleTypeDef hcan;
    
    /* 参数检查 */
    if ((Config == NULL) || (Config->channel >= CAN_CHANNEL_MAX)) {
        return STD_NOT_OK;
    }
    
    /* 初始化缓冲区 */
    Mcal_Can_BufferInit(Config->channel);
    
    /* 配置HAL CAN结构体 */
    if (Config->channel == CAN_CHANNEL_1) {
        hcan.Instance = CAN1;
    } else {
        hcan.Instance = CAN2;
    }
    
    /* 根据波特率计算分频器 */
    /* STM32F407 APB1时钟 = 42MHz */
    switch (Config->baudrate) {
        case 125:
            hcan.Init.Prescaler = 42;   /* 42 * 16 = 672 => 42MHz / 336 = 125kHz */
            break;
        case 250:
            hcan.Init.Prescaler = 21;
            break;
        case 500:
            hcan.Init.Prescaler = 10;
            break;
        case 1000:
            hcan.Init.Prescaler = 5;
            break;
        default:
            return STD_NOT_OK;
    }
    
    hcan.Init.Mode = CAN_MODE_NORMAL;
    hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
    hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
    hcan.Init.TimeTriggeredMode = DISABLE;
    hcan.Init.AutoBusOff = ENABLE;
    hcan.Init.AutoWakeUp = ENABLE;
    hcan.Init.AutoRetransmission = ENABLE;
    hcan.Init.ReceiveFifoLocked = DISABLE;
    hcan.Init.TransmitFifoPriority = DISABLE;
    
    /* 初始化CAN硬件 */
    if (HAL_CAN_Init(&hcan) != HAL_OK) {
        return STD_NOT_OK;
    }
    
    /* 配置接收过滤器 (接收所有消息) */
    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    
    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK) {
        return STD_NOT_OK;
    }
    
    /* 启动CAN */
    if (HAL_CAN_Start(&hcan) != HAL_OK) {
        return STD_NOT_OK;
    }
    
    /* 启用接收中断 */
    if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        return STD_NOT_OK;
    }
    
    s_canDriverState[Config->channel].initialized = 1;
    
    return STD_OK;
}

/**
 * @brief CAN消息发送
 */
Std_ReturnType Mcal_Can_Send(uint8 Channel, const Can_FrameType *Frame)
{
    if ((Channel >= CAN_CHANNEL_MAX) || (Frame == NULL)) {
        return STD_NOT_OK;
    }
    
    /* 检查驱动是否初始化 */
    if (s_canDriverState[Channel].initialized == 0) {
        return STD_NOT_OK;
    }
    
    /* 将消息加入发送缓冲区 */
    return Mcal_Can_PushTxBuffer(Channel, Frame);
}

/**
 * @brief CAN消息接收
 */
Std_ReturnType Mcal_Can_Receive(uint8 Channel, Can_FrameType *Frame)
{
    if ((Channel >= CAN_CHANNEL_MAX) || (Frame == NULL)) {
        return STD_NOT_OK;
    }
    
    /* 从接收缓冲区取出消息 */
    return Mcal_Can_PopRxBuffer(Channel, Frame);
}

/* ============= Private Function Implementations ============= */

/**
 * @brief 初始化缓冲区
 */
static void Mcal_Can_BufferInit(uint8 Channel)
{
    s_canRxBuffer[Channel].writeIndex = 0;
    s_canRxBuffer[Channel].readIndex = 0;
    s_canRxBuffer[Channel].count = 0;
    
    s_canTxBuffer[Channel].writeIndex = 0;
    s_canTxBuffer[Channel].readIndex = 0;
    s_canTxBuffer[Channel].count = 0;
}

/**
 * @brief 将接收消息加入缓冲区
 */
static Std_ReturnType Mcal_Can_PushRxBuffer(uint8 Channel, const Can_FrameType *Frame)
{
    Can_RxBufferType *pBuffer = &s_canRxBuffer[Channel];
    
    /* 缓冲区满检查 */
    if (pBuffer->count >= CAN_RX_BUFFER_SIZE) {
        s_canDriverState[Channel].rxErrors++;
        return STD_NOT_OK;
    }
    
    /* 复制数据到缓冲区 */
    MEMCPY(&pBuffer->buffer[pBuffer->writeIndex], Frame, sizeof(Can_FrameType));
    
    /* 更新写指针 */
    pBuffer->writeIndex++;
    if (pBuffer->writeIndex >= CAN_RX_BUFFER_SIZE) {
        pBuffer->writeIndex = 0;
    }
    
    pBuffer->count++;
    
    return STD_OK;
}

/**
 * @brief 从缓冲区取出接收消息
 */
static Std_ReturnType Mcal_Can_PopRxBuffer(uint8 Channel, Can_FrameType *Frame)
{
    Can_RxBufferType *pBuffer = &s_canRxBuffer[Channel];
    
    /* 缓冲区空检查 */
    if (pBuffer->count == 0) {
        return STD_NOT_OK;
    }
    
    /* 复制数据 */
    MEMCPY(Frame, &pBuffer->buffer[pBuffer->readIndex], sizeof(Can_FrameType));
    
    /* 更新读指针 */
    pBuffer->readIndex++;
    if (pBuffer->readIndex >= CAN_RX_BUFFER_SIZE) {
        pBuffer->readIndex = 0;
    }
    
    pBuffer->count--;
    
    return STD_OK;
}

/**
 * @brief 将消息加入发送缓冲区
 */
static Std_ReturnType Mcal_Can_PushTxBuffer(uint8 Channel, const Can_FrameType *Frame)
{
    Can_TxBufferType *pBuffer = &s_canTxBuffer[Channel];
    
    /* 缓冲区满检查 */
    if (pBuffer->count >= CAN_TX_BUFFER_SIZE) {
        s_canDriverState[Channel].txErrors++;
        return STD_NOT_OK;
    }
    
    /* 复制数据到缓冲区 */
    MEMCPY(&pBuffer->buffer[pBuffer->writeIndex], Frame, sizeof(Can_FrameType));
    
    /* 更新写指针 */
    pBuffer->writeIndex++;
    if (pBuffer->writeIndex >= CAN_TX_BUFFER_SIZE) {
        pBuffer->writeIndex = 0;
    }
    
    pBuffer->count++;
    
    return STD_OK;
}

/**
 * @brief 从缓冲区取出发送消息
 */
static Std_ReturnType Mcal_Can_PopTxBuffer(uint8 Channel, Can_FrameType *Frame)
{
    Can_TxBufferType *pBuffer = &s_canTxBuffer[Channel];
    
    /* 缓冲区空检查 */
    if (pBuffer->count == 0) {
        return STD_NOT_OK;
    }
    
    /* 复制数据 */
    MEMCPY(Frame, &pBuffer->buffer[pBuffer->readIndex], sizeof(Can_FrameType));
    
    /* 更新读指针 */
    pBuffer->readIndex++;
    if (pBuffer->readIndex >= CAN_TX_BUFFER_SIZE) {
        pBuffer->readIndex = 0;
    }
    
    pBuffer->count--;
    
    return STD_OK;
}

/**
 * @brief CAN接收中断处理程序
 */
void Mcal_Can_RxISR(void)
{
    /* 中断处理逻辑 */
    /* 从硬件FIFO读取消息，加入缓冲区 */
}
```

---

## 2. BSW 层代码示例

### 2.1 DCM 诊断服务头文件 (inc/bsw/dcm/dcm_service.h)

```c
/**
 * @file dcm_service.h
 * @brief DCM诊断服务头文件
 * @version 1.0.0
 * @date 2024-01-01
 */

#ifndef DCM_SERVICE_H
#define DCM_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "dcm_types.h"

/* ============= UDS Service IDs ============= */

#define DCM_SERVICE_0x10        0x10U   /**< DiagnosticSessionControl */
#define DCM_SERVICE_0x11        0x11U   /**< ECUReset */
#define DCM_SERVICE_0x14        0x14U   /**< ClearDiagnosticInformation */
#define DCM_SERVICE_0x19        0x19U   /**< ReadDTCInformation */
#define DCM_SERVICE_0x22        0x22U   /**< ReadDataByIdentifier */
#define DCM_SERVICE_0x2E        0x2EU   /**< WriteDataByIdentifier */
#define DCM_SERVICE_0x27        0x27U   /**< SecurityAccess */
#define DCM_SERVICE_0x28        0x28U   /**< CommunicationControl */
#define DCM_SERVICE_0x31        0x31U   /**< RoutineControl */
#define DCM_SERVICE_0x34        0x34U   /**< RequestDownload */
#define DCM_SERVICE_0x36        0x36U   /**< TransferData */
#define DCM_SERVICE_0x37        0x37U   /**< RequestTransferExit */
#define DCM_SERVICE_0x3E        0x3EU   /**< TesterPresent */
#define DCM_SERVICE_0x85        0x85U   /**< ControlDTCSetting */

/* ============= Negative Response Codes (NRC) ============= */

#define DCM_NRC_GENERAL_REJECT          0x31U
#define DCM_NRC_SERVICE_NOT_SUPPORTED   0x11U
#define DCM_NRC_SUBFUNC_NOT_SUPPORTED   0x12U
#define DCM_NRC_WRONG_MSG_LENGTH        0x13U
#define DCM_NRC_COND_NOT_CORRECT        0x22U
#define DCM_NRC_SECURITY_DENIED         0x33U
#define DCM_NRC_INVALID_SESSION         0x7EU

/* ============= Session IDs ============= */

#define DCM_SESSION_DEFAULT             0x01U
#define DCM_SESSION_PROGRAMMING         0x02U
#define DCM_SESSION_EXTENDED            0x03U

/* ============= Security Levels ============= */

#define DCM_SECURITY_LEVEL_0            0x00U   /**< 无安全保护 */
#define DCM_SECURITY_LEVEL_1            0x01U   /**< L1: APP扩展权限 */
#define DCM_SECURITY_LEVEL_2            0x02U   /**< L2: FBL编程权限 */

/* ============= Function Declarations ============= */

/**
 * @brief DCM初始化
 * @return Std_ReturnType STD_OK或STD_NOT_OK
 */
Std_ReturnType Bsw_Dcm_Init(void);

/**
 * @brief DCM主处理函数（周期调用）
 */
void Bsw_Dcm_MainFunction(void);

/**
 * @brief 处理诊断请求
 * @param[in] Request 请求数据指针
 * @param[in] RequestLength 请求数据长度
 * @param[out] Response 响应数据指针
 * @param[out] ResponseLength 响应数据长度
 * @return Std_ReturnType
 */
Std_ReturnType Bsw_Dcm_ProcessRequest(
    const uint8 *Request,
    uint16 RequestLength,
    uint8 *Response,
    uint16 *ResponseLength
);

/**
 * @brief 0x10 服务处理：会话控制
 */
Std_ReturnType Bsw_Dcm_SessionControl(
    const uint8 *Request,
    uint16 RequestLength,
    uint8 *Response,
    uint16 *ResponseLength
);

/**
 * @brief 0x22 服务处理：读DID
 */
Std_ReturnType Bsw_Dcm_ReadDid(
    const uint8 *Request,
    uint16 RequestLength,
    uint8 *Response,
    uint16 *ResponseLength
);

/**
 * @brief 0x2E 服务处理：写DID
 */
Std_ReturnType Bsw_Dcm_WriteDid(
    const uint8 *Request,
    uint16 RequestLength,
    uint8 *Response,
    uint16 *ResponseLength
);

/**
 * @brief 0x27 服务处理：安全访问
 */
Std_ReturnType Bsw_Dcm_SecurityAccess(
    const uint8 *Request,
    uint16 RequestLength,
    uint8 *Response,
    uint16 *ResponseLength
);

/**
 * @brief 获取当前会话
 * @return 当前会话ID
 */
uint8 Bsw_Dcm_GetCurrentSession(void);

/**
 * @brief 获取安全级别
 * @return 当前安全级别
 */
uint8 Bsw_Dcm_GetSecurityLevel(void);

#ifdef __cplusplus
}
#endif

#endif /* DCM_SERVICE_H */
```

---

## 3. RTE 层代码示例

### 3.1 RTE接口头文件 (inc/rte/rte_interface.h)

```c
/**
 * @file rte_interface.h
 * @brief RTE Rte_Read/Rte_Write接口定义
 * @version 1.0.0
 * @date 2024-01-01
 */

#ifndef RTE_INTERFACE_H
#define RTE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "rte_types.h"

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

#ifdef __cplusplus
}
#endif

#endif /* RTE_INTERFACE_H */
```

---

## 4. 应用主程序代码示例

### 4.1 任务调度器 (src/app/app_task_scheduler.c)

```c
/**
 * @file app_task_scheduler.c
 * @brief 应用任务调度器实现
 * @version 1.0.0
 * @date 2024-01-01
 */

#include "app_task_scheduler.h"
#include "rte.h"
#include "bsw_dcm.h"
#include "bsw_dem.h"
#include "bsw_cannm.h"
#include "bsw_nvm.h"
#include "common.h"

/* ============= Defines ============= */

#define APP_TASK_PERIOD_10MS    10U
#define APP_TASK_PERIOD_100MS   100U
#define APP_TASK_PERIOD_1000MS  1000U

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
    if (s_task10msCounter >= APP_TASK_PERIOD_10MS) {
        s_task10msCounter = 0U;
        s_task10msFlag = 1U;
    }
    
    /* 100ms任务计数 */
    s_task100msCounter++;
    if (s_task100msCounter >= APP_TASK_PERIOD_100MS) {
        s_task100msCounter = 0U;
        s_task100msFlag = 1U;
    }
    
    /* 1s任务计数 */
    s_task1000msCounter++;
    if (s_task1000msCounter >= APP_TASK_PERIOD_1000MS) {
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
    
    if (s_task10msFlag == 0U) {
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
    
    if (s_task100msFlag == 0U) {
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
    if (s_task1000msFlag == 0U) {
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
```

---

## 5. 单元测试代码示例

### 5.1 CAN驱动单元测试 (test/unit/test_can_driver.c)

```c
/**
 * @file test_can_driver.c
 * @brief CAN驱动单元测试
 * @version 1.0.0
 * @date 2024-01-01
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "can_driver.h"
#include "mock_can_driver.h"

/* ============= Test Suite Setup ============= */

/**
 * @brief 测试初始化
 */
int setUp(void)
{
    /* 初始化Mock对象 */
    mock_can_init_setup();
    return 0;
}

/**
 * @brief 测试清理
 */
int tearDown(void)
{
    /* 清理Mock对象 */
    return 0;
}

/* ============= Test Cases ============= */

/**
 * @brief 测试CAN驱动初始化 - 成功case
 */
void test_mcal_can_init_success(void)
{
    Can_ConfigType config;
    config.baudrate = 500;      /* 500kbps */
    config.channel = CAN_CHANNEL_1;
    
    Std_ReturnType result = Mcal_Can_Init(&config);
    
    CU_ASSERT_EQUAL(result, STD_OK);
    CU_ASSERT_EQUAL(mock_can_init_called, TRUE);
    CU_ASSERT_EQUAL(mock_can_baudrate, 500);
}

/**
 * @brief 测试CAN驱动初始化 - 参数为NULL
 */
void test_mcal_can_init_null_config(void)
{
    Std_ReturnType result = Mcal_Can_Init(NULL);
    
    CU_ASSERT_EQUAL(result, STD_NOT_OK);
}

/**
 * @brief 测试CAN驱动初始化 - 无效波特率
 */
void test_mcal_can_init_invalid_baudrate(void)
{
    Can_ConfigType config;
    config.baudrate = 999;   /* 无效波特率 */
    config.channel = CAN_CHANNEL_1;
    
    Std_ReturnType result = Mcal_Can_Init(&config);
    
    CU_ASSERT_EQUAL(result, STD_NOT_OK);
}

/**
 * @brief 测试CAN消息发送 - 成功case
 */
void test_mcal_can_send_success(void)
{
    Can_FrameType frame;
    frame.id = 0x123;
    frame.dlc = 8;
    frame.data[0] = 0x01;
    
    Std_ReturnType result = Mcal_Can_Send(CAN_CHANNEL_1, &frame);
    
    CU_ASSERT_EQUAL(result, STD_OK);
    CU_ASSERT_EQUAL(mock_can_tx_count, 1);
}

/**
 * @brief 测试CAN消息发送 - 缓冲区满
 */
void test_mcal_can_send_buffer_full(void)
{
    Can_FrameType frame;
    
    /* 填满发送缓冲区 */
    for (int i = 0; i < 64; i++) {
        Mcal_Can_Send(CAN_CHANNEL_1, &frame);
    }
    
    /* 再次发送应该失败 */
    Std_ReturnType result = Mcal_Can_Send(CAN_CHANNEL_1, &frame);
    
    CU_ASSERT_EQUAL(result, STD_NOT_OK);
}

/**
 * @brief 测试CAN消息接收 - 成功case
 */
void test_mcal_can_receive_success(void)
{
    Can_FrameType sendFrame, recvFrame;
    sendFrame.id = 0x456;
    sendFrame.dlc = 4;
    sendFrame.data[0] = 0xAA;
    sendFrame.data[1] = 0xBB;
    sendFrame.data[2] = 0xCC;
    sendFrame.data[3] = 0xDD;
    
    /* 模拟接收 (通过Mock) */
    mock_can_receive_data(&sendFrame);
    
    /* 读取接收到的数据 */
    Std_ReturnType result = Mcal_Can_Receive(CAN_CHANNEL_1, &recvFrame);
    
    CU_ASSERT_EQUAL(result, STD_OK);
    CU_ASSERT_EQUAL(recvFrame.id, 0x456);
    CU_ASSERT_EQUAL(recvFrame.dlc, 4);
    CU_ASSERT_EQUAL(recvFrame.data[0], 0xAA);
}

/**
 * @brief 测试CAN消息接收 - 缓冲区空
 */
void test_mcal_can_receive_buffer_empty(void)
{
    Can_FrameType recvFrame;
    
    Std_ReturnType result = Mcal_Can_Receive(CAN_CHANNEL_1, &recvFrame);
    
    CU_ASSERT_EQUAL(result, STD_NOT_OK);
}

/* ============= Test Suite Registration ============= */

int main(void)
{
    CU_pSuite pSuite = NULL;
    
    /* 初始化CUnit框架 */
    if (CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }
    
    /* 创建测试套件 */
    pSuite = CU_add_suite("Can_Driver_Suite", setUp, tearDown);
    if (pSuite == NULL) {
        CU_cleanup_registry();
        return CU_get_error();
    }
    
    /* 添加测试用例 */
    CU_add_test(pSuite, "test_mcal_can_init_success",
                test_mcal_can_init_success);
    CU_add_test(pSuite, "test_mcal_can_init_null_config",
                test_mcal_can_init_null_config);
    CU_add_test(pSuite, "test_mcal_can_init_invalid_baudrate",
                test_mcal_can_init_invalid_baudrate);
    CU_add_test(pSuite, "test_mcal_can_send_success",
                test_mcal_can_send_success);
    CU_add_test(pSuite, "test_mcal_can_send_buffer_full",
                test_mcal_can_send_buffer_full);
    CU_add_test(pSuite, "test_mcal_can_receive_success",
                test_mcal_can_receive_success);
    CU_add_test(pSuite, "test_mcal_can_receive_buffer_empty",
                test_mcal_can_receive_buffer_empty);
    
    /* 运行测试 */
    CU_basic_run_tests();
    
    /* 打印测试报告 */
    CU_basic_show_failures(CU_get_failure_list());
    
    /* 清理 */
    CU_cleanup_registry();
    
    return CU_get_error();
}
```

### 5.2 Mock对象头文件 (test/mock/mock_can_driver.h)

```c
/**
 * @file mock_can_driver.h
 * @brief CAN驱动Mock对象头文件
 * @version 1.0.0
 * @date 2024-01-01
 */

#ifndef MOCK_CAN_DRIVER_H
#define MOCK_CAN_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "can_driver.h"

/* ============= Mock Global Variables ============= */

/** 初始化调用标志 */
extern uint8 mock_can_init_called;

/** 初始化的波特率 */
extern uint32 mock_can_baudrate;

/** 发送计数 */
extern uint32 mock_can_tx_count;

/** 接收计数 */
extern uint32 mock_can_rx_count;

/* ============= Mock Function Prototypes ============= */

/**
 * @brief Mock初始化设置
 */
void mock_can_init_setup(void);

/**
 * @brief Mock接收数据设置
 */
void mock_can_receive_data(const Can_FrameType *Frame);

/**
 * @brief Mock重置
 */
void mock_can_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_CAN_DRIVER_H */
```

---

## 6. 关键宏定义文件

### 6.1 types.h - 标准类型定义

```c
/**
 * @file types.h
 * @brief 标准类型和返回值定义
 * @version 1.0.0
 * @date 2024-01-01
 */

#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============= Basic Types ============= */

typedef unsigned char       uint8;      /**< 无符号8位整数 */
typedef signed char         int8;       /**< 有符号8位整数 */
typedef unsigned short      uint16;     /**< 无符号16位整数 */
typedef signed short        int16;      /**< 有符号16位整数 */
typedef unsigned long       uint32;     /**< 无符号32位整数 */
typedef signed long         int32;      /**< 有符号32位整数 */
typedef unsigned long long  uint64;     /**< 无符号64位整数 */
typedef signed long long    int64;      /**< 有符号64位整数 */

typedef float               float32;    /**< 单精度浮点 */
typedef double              float64;    /**< 双精度浮点 */

/* ============= Boolean Types ============= */

typedef uint8 boolean;                  /**< 布尔类型 */

#ifndef TRUE
#define TRUE    1U
#endif

#ifndef FALSE
#define FALSE   0U
#endif

/* ============= Return Types ============= */

/**
 * @enum Std_ReturnType
 * @brief 标准返回类型
 */
typedef uint8 Std_ReturnType;

#define STD_OK      0U      /**< 操作成功 */
#define STD_NOT_OK  1U      /**< 操作失败 */

/**
 * @enum Std_OkType
 * @brief 标准OK返回类型
 */
typedef uint8 Std_OkType;

#define E_OK  0U

/* ============= Size Types ============= */

typedef uint32 size_t_emu;  /**< Size emulation */

/* ============= NULL Pointer ============= */

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */
```

---

## 7. 编译和构建配置

### 7.1 CMakeLists.txt 完整示例

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(STM32F407_ECU C ASM CXX)

# ========== Toolchain Configuration ==========

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "arm-none-eabi-as")

set(MCU "STM32F407VE")
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS "${CPU_FLAGS} -Wall -Wextra -Wpedantic -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O2 -g0")

set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -Wl,--gc-sections -Wl,-Map=output.map")

# ========== Include Paths ==========

set(COMMON_INCLUDES
    inc
    inc/config
    inc/mcal
    inc/bsw
    inc/rte
    inc/asw
    inc/app
    inc/fbl
)

include_directories(${COMMON_INCLUDES})

# ========== Source Files ==========

# MCAL源文件
set(MCAL_SOURCES
    src/mcal/can/can_driver.c
    src/mcal/can/can_interrupt.c
    src/mcal/adc/adc_driver.c
    src/mcal/timer/timer_driver.c
    src/mcal/gpio/gpio_driver.c
    src/mcal/uart/uart_driver.c
    src/mcal/spi/spi_driver.c
    src/mcal/pwm/pwm_driver.c
    src/mcal/nvm/eeprom_driver.c
)

# BSW源文件
set(BSW_SOURCES
    src/bsw/com/com_manager.c
    src/bsw/canif/canif_driver.c
    src/bsw/dcm/dcm_main.c
    src/bsw/dcm/dcm_service_0x22.c
    src/bsw/dcm/dcm_service_0x2E.c
    src/bsw/dcm/dcm_service_0x27.c
    src/bsw/dem/dem_main.c
    src/bsw/dem/dem_monitor.c
    src/bsw/cannm/cannm_main.c
    src/bsw/nvm/nvm_manager.c
)

# RTE源文件
set(RTE_SOURCES
    src/rte/rte_main.c
    src/rte/rte_interface.c
)

# ASW源文件 (Simulink生成的模型)
set(ASW_SOURCES
    src/asw/app_model_step_10ms.c
    src/asw/app_motor_control.c
)

# 应用主程序源文件
set(APP_SOURCES
    src/app/main.c
    src/app/app_init.c
    src/app/app_task_scheduler.c
    src/app/app_systick_handler.c
    src/app/app_lowpower.c
    src/app/app_watchdog.c
)

# FBL源文件
set(FBL_SOURCES
    src/fbl/fbl_main.c
    src/fbl/fbl_flash_driver.c
    src/fbl/fbl_dcm_service.c
)

# 配置文件
set(CONFIG_SOURCES
    src/config/can_matrix.c
    src/config/system_config.c
)

# 通用源文件
set(COMMON_SOURCES
    src/common.c
)

# ========== Build Targets ==========

# APP固件
add_executable(firmware_app.elf
    ${MCAL_SOURCES}
    ${BSW_SOURCES}
    ${RTE_SOURCES}
    ${ASW_SOURCES}
    ${APP_SOURCES}
    ${CONFIG_SOURCES}
    ${COMMON_SOURCES}
)

# FBL固件
add_executable(firmware_fbl.elf
    ${MCAL_SOURCES}
    ${BSW_SOURCES}
    ${FBL_SOURCES}
    ${CONFIG_SOURCES}
    ${COMMON_SOURCES}
)

# ========== Post-build Commands ==========

add_custom_command(TARGET firmware_app.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex firmware_app.elf firmware_app.hex
    COMMAND arm-none-eabi-objcopy -O binary firmware_app.elf firmware_app.bin
    COMMAND arm-none-eabi-size firmware_app.elf
)

# ========== Unit Tests (Optional) ==========

if(ENABLE_TESTING)
    enable_testing()
    
    add_executable(test_can_driver
        test/unit/test_can_driver.c
        test/mock/mock_can_driver.c
        src/mcal/can/can_driver.c
    )
    
    target_include_directories(test_can_driver PRIVATE
        ${COMMON_INCLUDES}
        test/mock
    )
    
    target_link_libraries(test_can_driver cunit)
    
    add_test(NAME CanDriver COMMAND test_can_driver)
endif()
```

---

**本文档提供了完整的代码模板和示例，可根据实际项目需求进行调整和扩展。**
