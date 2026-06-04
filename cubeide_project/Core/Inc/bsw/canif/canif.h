/**
 * @file canif.h
 * @brief AUTOSAR CanIf (CAN Interface Layer) 接口头文件
 * @version 1.0.0
 *
 * 遵循 AUTOSAR_SWS_CANInterface v4.4 规范。
 * CanIf 是 MCAL Can 和 BSW 上层模块（CanTp, Dcm, Com）之间的抽象层。
 *
 * 职责：
 *   - 向上层提供统一的 CAN 发送/接收接口（不感知硬件通道）
 *   - PDU 级别路由（将 PDU 路由到正确的 CAN 通道）
 *   - 发送确认/接收指示通知
 */

#ifndef CANIF_H
#define CANIF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "types.h"
#include "compiler.h"
#include "can_types.h"

/* ==================== 常量定义 ==================== */

/**
 * @brief CanIf 支持的最大 PDU 数
 */
#define CANIF_MAX_PDU           32U

/* ==================== 类型定义 ==================== */

/**
 * @enum CanIf_PduIdType
 * @brief PDU ID 类型
 */
typedef uint8 CanIf_PduIdType;

/**
 * @enum CanIf_NotifResultType
 * @brief 发送/接收通知结果类型
 */
typedef enum
{
    CANIF_NOTIF_OK      = 0U,   /**< 发送/接收成功 */
    CANIF_NOTIF_ERROR   = 1U,   /**< 发送/接收失败 */
    CANIF_NOTIF_TX_OVERFLOW = 2U, /**< TX 缓冲溢出 */
    CANIF_NOTIF_BUFFER_FULL  = 3U /**< RX 缓冲满 */
} CanIf_NotifResultType;

/**
 * @struct CanIf_TxPduType
 * @brief CanIf 发送 PDU 配置
 */
typedef struct
{
    CanIf_PduIdType    pduId;          /**< PDU ID */
    uint32             canId;          /**< CAN 报文 ID */
    uint8              dlc;            /**< 数据长度 */
    uint8              channel;        /**< CAN 通道号 */
    Can_IdType         idType;         /**< ID 类型 (标准/扩展) */
} CanIf_TxPduType;

/**
 * @struct CanIf_RxPduType
 * @brief CanIf 接收 PDU 配置
 */
typedef struct
{
    CanIf_PduIdType    pduId;          /**< PDU ID */
    uint32             canId;          /**< CAN 报文 ID (用于过滤) */
    uint32             canIdMask;      /**< CAN ID 掩码 */
    uint8              dlc;            /**< 期望数据长度 */
    uint8              channel;        /**< CAN 通道号 */
    Can_IdType         idType;         /**< ID 类型 */
} CanIf_RxPduType;

/**
 * @struct CanIf_Pdu
 * @brief CanIf PDU 数据 (用于发送/接收)
 */
typedef struct
{
    CanIf_PduIdType    pduId;          /**< PDU ID */
    uint8             *sdu;            /**< 数据指针 */
    uint8              length;         /**< 数据长度 */
} CanIf_Pdu;

/* ==================== 回调函数类型 ==================== */

/**
 * @brief TX 完成通知回调 (上层模块注册)
 */
typedef void (*CanIf_TxConfirmation)(CanIf_PduIdType PduId,
                                     CanIf_NotifResultType Result);

/**
 * @brief RX 指示回调 (上层模块注册)
 */
typedef void (*CanIf_RxIndication)(const CanIf_Pdu *Pdu);

/* ==================== 公开函数声明 ==================== */

/**
 * @brief CanIf 初始化
 *
 * 安装默认 PDU 路由配置。
 */
FUNC(void, CAN_CODE)
CanIf_Init(void);

/**
 * @brief CanIf 发送 PDU
 *
 * 根据 PDU ID 查找对应的 CAN 通道和 ID，调用 Can_Write。
 *
 * @param[in] PduId  PDU ID
 * @param[in] PduPtr PDU 数据指针
 * @return Std_ReturnType
 */
FUNC(Std_ReturnType, CAN_CODE)
CanIf_Transmit(
    CanIf_PduIdType                PduId,
    CONSTP2VAR(CanIf_Pdu, AUTOMATIC, CAN_APPL_DATA) PduPtr
);

/**
 * @brief CanIf 取消发送
 *
 * @param[in] PduId  PDU ID
 */
FUNC(void, CAN_CODE)
CanIf_CancelTransmit(
    CanIf_PduIdType PduId
);

/**
 * @brief 注册 TX 完成通知回调
 *
 * @param[in] Callback 回调函数指针
 */
FUNC(void, CAN_CODE)
CanIf_SetTxConfirmation(
    CanIf_TxConfirmation Callback
);

/**
 * @brief 注册 RX 指示回调 (Com)
 *
 * @param[in] Callback 回调函数指针
 */
FUNC(void, CAN_CODE)
CanIf_SetRxIndication(
    CanIf_RxIndication Callback
);

/**
 * @brief 注册 RX 指示回调 2 (CANtp/PduR)
 *
 * @param[in] Callback 回调函数指针
 */
FUNC(void, CAN_CODE)
CanIf_SetRxIndication2(
    CanIf_RxIndication Callback
);

/**
 * @brief CanIf 主函数 (10ms 轮询)
 *
 * 将 CanIf 缓冲区的待发 PDU 传给 Can_Write，
 * 同时检查发送完成确认。
 */
FUNC(void, CAN_CODE)
CanIf_MainFunction(void);

/**
 * @brief CanIf 接收处理 (由 MCAL Can_RxISR 回调调用)
 *
 * @param[in] Channel  CAN 通道号
 * @param[in] Frame    接收到的 CAN 帧指针
 */
FUNC(void, CAN_CODE)
CanIf_RxIndicationHandler(
    uint8                    Channel,
    CONSTP2CONST(Can_Frame, AUTOMATIC, CAN_APPL_CONST) Frame
);

#ifdef __cplusplus
}
#endif

#endif /* CANIF_H */