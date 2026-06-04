/**
 * @file canif.c
 * @brief AUTOSAR CanIf (CAN Interface Layer) 实现
 * @version 1.0.0
 *
 * 对接 MCAL Can 驱动，为上层的 CanTp/Dcm/Com 提供 PDU 级路由。
 *
 * 路由表基于 DBC 报文矩阵配置（参见 docs/AutoSarECU.DBC）:
 *   PDU 0: ECU_Status         (0x1A0, 50ms周期, ch1)
 *   PDU 1: ECU_LifeCycle      (0x3A0, 100ms周期, ch1)
 *   PDU 2: Vehicle_Ctrl       (0x210, 50ms周期, ch1, RX)
 *   PDU 3: ECU_NM_0x415       (0x415, 100ms周期, ch1)
 *   PDU 4: Diag_Req_ECU       (0x7A0, event, ch1, RX)
 *   PDU 5: Diag_Resp_ECU      (0x7A8, event, ch1, TX)
 *   PDU 6: Diag_Functional_Req (0x7DF, event, ch1, RX)
 */

#include "canif.h"
#include "can_driver.h"
#include "common.h"
#include "string.h"

/* ==================== 常量：DBC 报文路由表 ==================== */

/** TX PDU 路由表 */
static const CanIf_TxPduType s_txPduMap[] = {
    /* PDU 0: ECU_Status (0x1A0, 8 bytes) */
    {0U, 0x1A0U, 8U, CAN_CHANNEL_1, CAN_ID_STANDARD},
    /* PDU 1: ECU_LifeCycle (0x3A0, 4 bytes) */
    {1U, 0x3A0U, 4U, CAN_CHANNEL_1, CAN_ID_STANDARD},
    /* PDU 3: ECU_NM_0x415 (0x415, 4 bytes) */
    {3U, 0x415U, 4U, CAN_CHANNEL_1, CAN_ID_STANDARD},
    /* PDU 5: Diag_Resp_ECU (0x7A8, 8 bytes) */
    {5U, 0x7A8U, 8U, CAN_CHANNEL_1, CAN_ID_STANDARD},
};

/** TX PDU 数量 */
#define CANIF_TX_PDU_COUNT  (sizeof(s_txPduMap) / sizeof(s_txPduMap[0]))

/** RX PDU 路由表 */
static const CanIf_RxPduType s_rxPduMap[] = {
    /* PDU 2: Vehicle_Ctrl (0x210, RX from VCU) */
    {2U, 0x210U, 0x7FFU, 8U, CAN_CHANNEL_1, CAN_ID_STANDARD},
    /* PDU 4: Diag_Req_ECU (0x7A0, UDS request) */
    {4U, 0x7A0U, 0x7FFU, 8U, CAN_CHANNEL_1, CAN_ID_STANDARD},
    /* PDU 6: Diag_Functional_Req (0x7DF, UDS functional) */
    {6U, 0x7DFU, 0x7FFU, 8U, CAN_CHANNEL_1, CAN_ID_STANDARD},
};

/** RX PDU 数量 */
#define CANIF_RX_PDU_COUNT  (sizeof(s_rxPduMap) / sizeof(s_rxPduMap[0]))

/* ==================== 私有全局变量 ==================== */

/** TX 完成通知回调 (由 Dcm 或 Com 注册) */
static CanIf_TxConfirmation s_txCallback = NULL_PTR;

/** RX 指示回调 1 (由 Com 注册, 处理普通信号 PDU) */
static CanIf_RxIndication  s_rxCallback1 = NULL_PTR;

/** RX 指示回调 2 (由 CANtp/PduR 注册, 处理 UDS PDU) */
static CanIf_RxIndication  s_rxCallback2 = NULL_PTR;

/* ==================== 公开函数实现 ==================== */

/**
 * @brief CanIf 初始化
 *
 * 将接收通知回调注册到 MCAL Can 驱动。
 */
FUNC(void, CAN_CODE)
CanIf_Init(void)
{
    /* 将 CanIf 注册为 Can 驱动的接收通知接收者 */
    Can_SetRxNotification(CanIf_RxIndicationHandler);
}

/**
 * @brief CanIf 发送 PDU
 *
 * 根据 PDU ID 查找 TX PDU 映射表，填充 CAN 帧后调用 Can_Write。
 */
FUNC(Std_ReturnType, CAN_CODE)
CanIf_Transmit(
    CanIf_PduIdType                PduId,
    CONSTP2VAR(CanIf_Pdu, AUTOMATIC, CAN_APPL_DATA) PduPtr
)
{
    Can_Frame frame;
    uint8     i;

    /* 参数检查 */
    if (PduPtr == NULL_PTR) {
        return STD_NOT_OK;
    }

    /* 查找 PDU ID */
    for (i = 0U; i < CANIF_TX_PDU_COUNT; i++) {
        if (s_txPduMap[i].pduId == PduId) {
            /* 填充 CAN 帧 */
            frame.id        = s_txPduMap[i].canId;
            frame.dlc       = (PduPtr->length > s_txPduMap[i].dlc)
                              ? s_txPduMap[i].dlc : PduPtr->length;
            frame.idType    = s_txPduMap[i].idType;
            frame.frameType = CAN_FRAME_DATA;

            MEMCPY(frame.sdu, PduPtr->sdu, frame.dlc);

            /* 通过 MCAL Can 发送 */
            return Can_Write(s_txPduMap[i].channel, &frame);
        }
    }

    /* 未找到 PDU ID */
    return STD_NOT_OK;
}

/**
 * @brief CanIf 取消发送 (暂未实现)
 */
FUNC(void, CAN_CODE)
CanIf_CancelTransmit(
    CanIf_PduIdType PduId
)
{
    (void)PduId;
}

/**
 * @brief 注册 TX 完成通知回调
 */
FUNC(void, CAN_CODE)
CanIf_SetTxConfirmation(
    CanIf_TxConfirmation Callback
)
{
    s_txCallback = Callback;
}

/**
 * @brief 注册 RX 指示回调 1 (Com)
 */
FUNC(void, CAN_CODE)
CanIf_SetRxIndication(
    CanIf_RxIndication Callback
)
{
    s_rxCallback1 = Callback;
}

/**
 * @brief 注册 RX 指示回调 2 (CANtp/PduR)
 */
FUNC(void, CAN_CODE)
CanIf_SetRxIndication2(
    CanIf_RxIndication Callback
)
{
    s_rxCallback2 = Callback;
}

/**
 * @brief CanIf 主函数 (10ms 任务)
 *
 * 检查 MCAL Can 驱动中是否有待发的缓冲帧。
 * 实际发送由 Can_MainFunction_Write() 完成，CanIf 不做重复工作，
 * 此处仅作为扩展预留。
 */
FUNC(void, CAN_CODE)
CanIf_MainFunction(void)
{
    /* Can_MainFunction_Write() 在 10ms 任务中由 app_task_scheduler 调用 */
}

/**
 * @brief CanIf 接收处理
 *
 * 由 MCAL Can 驱动的 Can_SetRxNotification 注册的回调触发。
 * 从 Can_Frame 提取 PDU ID，调用上层 RX 指示回调。
 */
FUNC(void, CAN_CODE)
CanIf_RxIndicationHandler(
    uint8                    Channel,
    CONSTP2CONST(Can_Frame, AUTOMATIC, CAN_APPL_CONST) Frame
)
{
    CanIf_Pdu pdu;
    uint8     i;

    if (Frame == NULL_PTR) {
        return;
    }

    /* 查找 RX PDU 映射表 */
    for (i = 0U; i < CANIF_RX_PDU_COUNT; i++) {
        if ((s_rxPduMap[i].channel == Channel) &&
            ((Frame->id & s_rxPduMap[i].canIdMask) == s_rxPduMap[i].canId)) {
            /* 匹配：填充 PDU 并通知上层 */
            pdu.pduId  = s_rxPduMap[i].pduId;
            pdu.sdu    = (uint8 *)(Frame->sdu);
            pdu.length = (Frame->dlc > s_rxPduMap[i].dlc)
                         ? s_rxPduMap[i].dlc : Frame->dlc;

            /* 路由：UDS PDU (4/6) → CANtp, 其他 PDU (2) → Com */
            if (s_rxPduMap[i].pduId == 4U || s_rxPduMap[i].pduId == 6U) {
                if (s_rxCallback2 != NULL_PTR) {
                    s_rxCallback2(&pdu);
                }
            } else {
                if (s_rxCallback1 != NULL_PTR) {
                    s_rxCallback1(&pdu);
                }
            }
            return;
        }
    }

    /* 未匹配任何 PDU → 丢弃 */
}