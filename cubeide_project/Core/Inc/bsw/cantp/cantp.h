/**
 * @file cantp.h
 * @brief AUTOSAR CAN Transport Layer (ISO 15765-2) 接口
 * @version 1.0.0
 *
 * 遵循 AUTOSAR_SWS_CANTransportLayer v4.4 规范。
 * 实现 ISO 15765-2 传输层协议:
 *   - Single Frame (SF)     : N_PCI byte[0] high nibble = 0x0
 *   - First Frame (FF)      : N_PCI byte[0] high nibble = 0x1
 *   - Consecutive Frame (CF): N_PCI byte[0] high nibble = 0x2
 *   - Flow Control (FC)     : N_PCI byte[0] high nibble = 0x3
 *
 * 定时器参数来自项目需求文档:
 *   N_As / N_Ar = 25ms    (发送/接收确认超时)
 *   N_Bs = 75ms           (发送方FlowControl等待超时)
 *   N_Br = <25ms          (接收方FlowControl响应要求)
 *   N_Cs = <50ms          (发送方CF间等待要求)
 *   N_Cr = 150ms          (接收方CF到达超时)
 */

#ifndef CANTP_H
#define CANTP_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "types.h"
#include "compiler.h"
#include "canif.h"

/* ==================== 常量定义 ==================== */

/**
 * @brief N_PCI 类型掩码 (PCI byte 高4位)
 */
#define CANTP_PCI_MASK                0xF0U
#define CANTP_PCI_TYPE_SF             0x00U   /* Single Frame */
#define CANTP_PCI_TYPE_FF             0x10U   /* First Frame */
#define CANTP_PCI_TYPE_CF             0x20U   /* Consecutive Frame */
#define CANTP_PCI_TYPE_FC             0x30U   /* Flow Control */

/**
 * @brief Flow Control 状态
 */
#define CANTP_FC_CTS                  0x00U   /* Continue To Send */
#define CANTP_FC_WAIT                 0x01U   /* Wait */
#define CANTP_FC_OVERFLOW             0x02U   /* Overflow */

/**
 * @brief CAN TP 最大 N-SDU 长度 (12-bit, FF_DL)
 */
#define CANTP_MAX_NSDU_LENGTH         4095U

/**
 * @brief RX/TX 缓冲区大小
 */
#define CANTP_RX_BUFFER_SIZE          512U
#define CANTP_TX_BUFFER_SIZE          512U

/**
 * @brief 默认块大小 (BS) 和最小间隔时间 (STmin)
 *
 * BS=0 表示无流控，接收方一次发送所有CF
 * STmin=10ms (来自项目需求文档)
 */
#define CANTP_DEFAULT_BS              0U
#define CANTP_DEFAULT_STMIN_MS        10U

/**
 * @brief 超时参数 (ms) — 来自项目需求文档
 */
#define CANTP_N_AS_MS                 25U     /* 发送确认超时 */
#define CANTP_N_AR_MS                 25U     /* 接收确认超时 */
#define CANTP_N_BS_MS                 75U     /* 发送方 FC 等待超时 */
#define CANTP_N_CR_MS                 150U    /* 接收方 CF 到达超时 */

/* ==================== 类型定义 ==================== */

/**
 * @enum CanTp_NPciType
 * @brief N_PCI 协议类型
 */
typedef enum
{
    CANTP_IDLE = 0U,            /* 空闲 */
    CANTP_SINGLE_FRAME,         /* 单帧 */
    CANTP_FIRST_FRAME,          /* 首帧 */
    CANTP_CONSECUTIVE_FRAME,    /* 连续帧 */
    CANTP_FLOW_CONTROL          /* 流控帧 */
} CanTp_NPciType;

/**
 * @enum CanTp_ChannelState
 * @brief CAN TP 通道状态机
 */
typedef enum
{
    CANTP_CH_IDLE = 0U,         /* 空闲 */
    CANTP_CH_WAIT_FC,           /* 等待流控 (发送方) */
    CANTP_CH_SENDING_CF,        /* 发送连续帧 */
    CANTP_CH_WAIT_CF,           /* 等待连续帧 (接收方) */
    CANTP_CH_READY              /* 接收完成, 可交付 */
} CanTp_ChannelState;

/**
 * @struct CanTp_ChannelType
 * @brief CAN TP 通道上下文 (每个连接一个实例)
 */
typedef struct
{
    uint8                channelId;              /* 通道 ID (0-based) */
    CanTp_ChannelState   state;                  /* 状态机 */
    CanIf_PduIdType      rxPduId;                /* 接收 PDU ID (CAN ID) */
    CanIf_PduIdType      txPduId;                /* 发送 PDU ID (CAN ID) */
    uint8                txAddr;                 /* 目标 N_TA (=N_SA) */
    uint8                rxAddr;                 /* 源 N_SA (=N_TA) */

    /* ── 接收状态 ── */
    uint8                rxBuffer[CANTP_RX_BUFFER_SIZE];
    uint16               rxLength;               /* 预期总长度 (FF_DL) */
    uint16               rxIndex;                /* 已接收字节数 */
    uint8                rxExpectedSN;           /* 期望的 Sequence Number */
    uint32               rxTimer;                /* N_Cr 定时器 (ms) */
    boolean              rxComplete;             /* 接收完成标志 */

    /* ── 发送状态 ── */
    const uint8         *txBuffer;               /* 待发送数据指针 */
    uint16               txLength;               /* 待发送总长度 */
    uint16               txIndex;                /* 已发送字节数 */
    uint8                txSN;                   /* 当前 Sequence Number */
    uint8                txBlockRemaining;       /* 当前块剩余帧数 */
    uint32               txTimer;                /* 发送定时器 (N_As/N_Bs) */
    boolean              txPending;              /* 有待发送数据 */
} CanTp_ChannelType;

/**
 * @struct CanTp_PduInfoType
 * @brief CAN TP 交付给上层的 PDU 信息
 */
typedef struct
{
    uint8               *data;                   /* 数据指针 */
    uint16               length;                 /* 数据长度 */
    uint8                protocolResult;          /* 0=OK */
} CanTp_PduInfoType;

/* ==================== 回调类型 ==================== */

/**
 * @brief CANtp 接收完成回调 (通知 PduR)
 * @param[in] channelId  CAN TP 通道 ID
 * @param[in] pduInfo    接收到的 N-SDU
 */
typedef void (*CanTp_RxIndication)(uint8 channelId,
                                   CONSTP2CONST(CanTp_PduInfoType, AUTOMATIC, CAN_APPL_DATA) pduInfo);

/**
 * @brief CANtp 发送完成回调 (通知 PduR)
 * @param[in] channelId  CAN TP 通道 ID
 * @param[in] result     发送结果 (STD_OK / STD_NOT_OK)
 */
typedef void (*CanTp_TxConfirmation)(uint8 channelId, Std_ReturnType result);

/* ==================== 公开函数声明 ==================== */

/**
 * @brief CANtp 初始化
 *
 * 分配各通道资源，初始化状态机。
 */
FUNC(void, CAN_CODE)
CanTp_Init(void);

/**
 * @brief CANtp 请求发送 N-SDU
 *
 * 数据可能跨多帧 (SF / FF+CF)，通过协议栈分片发送。
 *
 * @param[in] channelId  CAN TP 通道 ID
 * @param[in] data       待发送数据指针
 * @param[in] length     数据长度
 * @return Std_ReturnType
 */
FUNC(Std_ReturnType, CAN_CODE)
CanTp_Transmit(uint8 channelId,
               CONSTP2CONST(uint8, AUTOMATIC, CAN_APPL_DATA) data,
               uint16 length);

/**
 * @brief CANtp 取消发送
 *
 * @param[in] channelId  CAN TP 通道 ID
 */
FUNC(void, CAN_CODE)
CanTp_CancelTransmit(uint8 channelId);

/**
 * @brief CANtp 主函数 (建议每 1-10ms 调用一次)
 *
 * 驱动发送/接收状态机，处理超时。
 */
FUNC(void, CAN_CODE)
CanTp_MainFunction(void);

/**
 * @brief CANtp 接收来自 CanIf 的 CAN 帧
 *
 * 由 CanIf_RxIndication 调用，根据 PDU ID 路由到对应通道。
 *
 * @param[in] Pdu  CanIf 接收的 PDU
 */
FUNC(void, CAN_CODE)
CanTp_HandleRxPdu(CONSTP2CONST(CanIf_Pdu, AUTOMATIC, CAN_APPL_DATA) Pdu);

/**
 * @brief 注册接收回调
 * @param[in] Callback  接收回调函数
 */
FUNC(void, CAN_CODE)
CanTp_SetRxIndication(CanTp_RxIndication Callback);

/**
 * @brief 注册发送确认回调
 * @param[in] Callback  发送确认回调函数
 */
FUNC(void, CAN_CODE)
CanTp_SetTxConfirmation(CanTp_TxConfirmation Callback);

/**
 * @brief 获取通道数量
 * @return 配置的最大通道数
 */
FUNC(uint8, CAN_CODE)
CanTp_GetChannelCount(void);

#ifdef __cplusplus
}
#endif

#endif /* CANTP_H */