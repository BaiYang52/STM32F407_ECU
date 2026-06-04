/**
 * @file pdur.c
 * @brief AUTOSAR PDU Router 实现
 * @version 1.0.0
 *
 * PduR 在 CANtp 和 DCM 之间建立路由:
 *   PDU 0 (UDS Physical)  : 0x7A0→0x7A8, CanTp Channel 0
 *   PDU 1 (UDS Functional): 0x7DF, CanTp Channel 1 (RX only)
 *
 * 回调链:
 *   DCM ← PduR ← CANtp ← CanIf ← CAN Driver
 */

#include "pdur.h"
#include "common.h"

/* ==================== 回调注册 ==================== */

/** 接收回调 (→ DCM) */
static PduR_RxIndication  s_rxCallback = NULL_PTR;

/** 发送确认回调 (→ DCM) */
static PduR_TxConfirmation s_txCallback = NULL_PTR;

/* ==================== 私有函数声明 ==================== */

/**
 * @brief CANtp 接收回调 (由 CANtp 调用)
 */
static void PduR_CanTpRxIndication(uint8 channelId,
                                   CONSTP2CONST(CanTp_PduInfoType, AUTOMATIC, CAN_APPL_DATA) pduInfo);

/**
 * @brief CANtp 发送确认回调 (由 CANtp 调用)
 */
static void PduR_CanTpTxConfirmation(uint8 channelId, Std_ReturnType result);

/* ==================== 公开函数实现 ==================== */

FUNC(void, CAN_CODE)
PduR_Init(void)
{
    /* 初始化 CANtp (CANtp 初始化通道配置) */
    CanTp_Init();

    /* 注册 PduR 的回调到 CANtp */
    CanTp_SetRxIndication(PduR_CanTpRxIndication);
    CanTp_SetTxConfirmation(PduR_CanTpTxConfirmation);
}

FUNC(void, CAN_CODE)
PduR_MainFunction(void)
{
    /* 驱动 CANtp 的状态机 (超时检测 + CF 调度 + 交付) */
    CanTp_MainFunction();
}

FUNC(Std_ReturnType, CAN_CODE)
PduR_Transmit(PduR_PduIdType PduId,
              CONSTP2CONST(uint8, AUTOMATIC, CAN_APPL_DATA) Data,
              uint16 Length)
{
    uint8 canTpChannel;

    /* PDU ID → CANtp Channel 映射 */
    switch (PduId) {
    case PDUR_ID_UDS_PHYSICAL:
        canTpChannel = 0U;
        break;

    case PDUR_ID_UDS_FUNCTIONAL:
        /* 功能寻址不发送响应 */
        return STD_OK;

    default:
        return STD_NOT_OK;
    }

    return CanTp_Transmit(canTpChannel, Data, Length);
}

FUNC(void, CAN_CODE)
PduR_CancelTransmit(PduR_PduIdType PduId)
{
    switch (PduId) {
    case PDUR_ID_UDS_PHYSICAL:
        CanTp_CancelTransmit(0U);
        break;

    default:
        break;
    }
}

FUNC(void, CAN_CODE)
PduR_SetRxIndication(PduR_RxIndication Callback)
{
    s_rxCallback = Callback;
}

FUNC(void, CAN_CODE)
PduR_SetTxConfirmation(PduR_TxConfirmation Callback)
{
    s_txCallback = Callback;
}

/* ==================== 私有函数实现 ==================== */

/**
 * @brief CANtp 接收回调 → 路由到 DCM
 */
static void PduR_CanTpRxIndication(uint8 channelId,
                                   CONSTP2CONST(CanTp_PduInfoType, AUTOMATIC, CAN_APPL_DATA) pduInfo)
{
    PduR_PduIdType pduId;

    if (pduInfo == NULL_PTR) {
        return;
    }
    if (pduInfo->data == NULL_PTR) {
        return;
    }

    /* 通道 → PDU ID 映射 */
    switch (channelId) {
    case 0U:
        pduId = PDUR_ID_UDS_PHYSICAL;
        break;

    case 1U:
        pduId = PDUR_ID_UDS_FUNCTIONAL;
        break;

    default:
        return;
    }

    /* 通知 DCM */
    if (s_rxCallback != NULL_PTR) {
        s_rxCallback(pduId, pduInfo->data, pduInfo->length);
    }
}

/**
 * @brief CANtp 发送确认回调 → 路由到 DCM
 */
static void PduR_CanTpTxConfirmation(uint8 channelId, Std_ReturnType result)
{
    PduR_PduIdType pduId;

    switch (channelId) {
    case 0U:
        pduId = PDUR_ID_UDS_PHYSICAL;
        break;

    case 1U:
        pduId = PDUR_ID_UDS_FUNCTIONAL;
        break;

    default:
        return;
    }

    if (s_txCallback != NULL_PTR) {
        s_txCallback(pduId, result);
    }
}