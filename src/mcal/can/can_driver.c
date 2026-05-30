/**
 * @file can_driver.c
 * @brief CAN驱动程序实现
 * @version 1.0.0
 * @date 2024-01-01
 */

#include "can_driver.h"
#include "common.h"

/* ============= Private Defines ============= */

#define CAN_RX_BUFFER_SIZE 256U
#define CAN_TX_BUFFER_SIZE 64U

/* ============= Private Data Types ============= */

/**
 * @struct Can_RxBufferType
 * @brief CAN接收缓冲区结构体
 */
typedef struct
{
    Can_FrameType buffer[CAN_RX_BUFFER_SIZE];
    uint16 writeIndex;
    uint16 readIndex;
    uint16 count;
} Can_RxBufferType;

/**
 * @struct Can_TxBufferType
 * @brief CAN发送缓冲区结构体
 */
typedef struct
{
    Can_FrameType buffer[CAN_TX_BUFFER_SIZE];
    uint16 writeIndex;
    uint16 readIndex;
    uint16 count;
} Can_TxBufferType;

/**
 * @struct Can_DriverStateType
 * @brief CAN驱动状态结构体
 */
typedef struct
{
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
    {0, CAN_MSG_IDLE, CAN_MSG_IDLE, 0, 0, 0}, {0, CAN_MSG_IDLE, CAN_MSG_IDLE, 0, 0, 0}};

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
    if ((Config == NULL) || (Config->channel >= CAN_CHANNEL_MAX))
    {
        return STD_NOT_OK;
    }

    /* 初始化缓冲区 */
    Mcal_Can_BufferInit(Config->channel);

    /* 配置HAL CAN结构体 */
    if (Config->channel == CAN_CHANNEL_1)
    {
        hcan.Instance = CAN1;
    }
    else
    {
        hcan.Instance = CAN2;
    }

    /* 根据波特率计算分频器 */
    /* STM32F407 APB1时钟 = 42MHz */
    switch (Config->baudrate)
    {
    case 125:
        hcan.Init.Prescaler = 42; /* 42 * 16 = 672 => 42MHz / 336 = 125kHz */
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
    if (HAL_CAN_Init(&hcan) != HAL_OK)
    {
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

    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
    {
        return STD_NOT_OK;
    }

    /* 启动CAN */
    if (HAL_CAN_Start(&hcan) != HAL_OK)
    {
        return STD_NOT_OK;
    }

    /* 启用接收中断 */
    if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
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
    if ((Channel >= CAN_CHANNEL_MAX) || (Frame == NULL))
    {
        return STD_NOT_OK;
    }

    /* 检查驱动是否初始化 */
    if (s_canDriverState[Channel].initialized == 0)
    {
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
    if ((Channel >= CAN_CHANNEL_MAX) || (Frame == NULL))
    {
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
    if (pBuffer->count >= CAN_RX_BUFFER_SIZE)
    {
        s_canDriverState[Channel].rxErrors++;
        return STD_NOT_OK;
    }

    /* 复制数据到缓冲区 */
    MEMCPY(&pBuffer->buffer[pBuffer->writeIndex], Frame, sizeof(Can_FrameType));

    /* 更新写指针 */
    pBuffer->writeIndex++;
    if (pBuffer->writeIndex >= CAN_RX_BUFFER_SIZE)
    {
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
    if (pBuffer->count == 0)
    {
        return STD_NOT_OK;
    }

    /* 复制数据 */
    MEMCPY(Frame, &pBuffer->buffer[pBuffer->readIndex], sizeof(Can_FrameType));

    /* 更新读指针 */
    pBuffer->readIndex++;
    if (pBuffer->readIndex >= CAN_RX_BUFFER_SIZE)
    {
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
    if (pBuffer->count >= CAN_TX_BUFFER_SIZE)
    {
        s_canDriverState[Channel].txErrors++;
        return STD_NOT_OK;
    }

    /* 复制数据到缓冲区 */
    MEMCPY(&pBuffer->buffer[pBuffer->writeIndex], Frame, sizeof(Can_FrameType));

    /* 更新写指针 */
    pBuffer->writeIndex++;
    if (pBuffer->writeIndex >= CAN_TX_BUFFER_SIZE)
    {
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
    if (pBuffer->count == 0)
    {
        return STD_NOT_OK;
    }

    /* 复制数据 */
    MEMCPY(Frame, &pBuffer->buffer[pBuffer->readIndex], sizeof(Can_FrameType));

    /* 更新读指针 */
    pBuffer->readIndex++;
    if (pBuffer->readIndex >= CAN_TX_BUFFER_SIZE)
    {
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