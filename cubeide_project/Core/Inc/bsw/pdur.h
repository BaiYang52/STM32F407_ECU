/**
 * @file pdur.h
 * @brief AUTOSAR PDU Router 接口
 * @version 1.0.0
 *
 * 遵循 AUTOSAR_SWS_PDURouter v4.4 规范。
 * PduR 是 CANtp 和 DCM 之间的路由层:
 *   - 将 CANtp 接收的 N-SDU 路由到 DCM
 *   - 将 DCM 的发送请求路由到 CANtp
 *   - 支持 PDU ID 到模块的映射
 *
 * 路由表:
 *   PDU 0: UDS Physical Request/Response (0x7A0/0x7A8)
 *   PDU 1: UDS Functional Request       (0x7DF, 仅接收)
 */

#ifndef PDUR_H
#define PDUR_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "types.h"
#include "compiler.h"
#include "cantp.h"

/* ==================== 常量定义 ==================== */

/**
 * @brief PduR 支持的最大路由路径数
 */
#define PDUR_MAX_ROUTES             4U

/**
 * @brief PduR PDU ID 枚举
 */
typedef enum
{
    PDUR_ID_UDS_PHYSICAL    = 0U,   /**< UDS 物理寻址 : CAN ID 0x7A0->0x7A8 */
    PDUR_ID_UDS_FUNCTIONAL  = 1U,   /**< UDS 功能寻址 : CAN ID 0x7DF */
    PDUR_MAX_ID                    /**< 最大 ID */
} PduR_PduIdType;

/* ==================== 回调类型 ==================== */

/**
 * @brief PduR 接收指示回调 (通知 DCM)
 * @param[in] PduId  PDU ID
 * @param[in] Data   接收数据指针
 * @param[in] Length 数据长度
 */
typedef void (*PduR_RxIndication)(PduR_PduIdType PduId,
                                  CONSTP2CONST(uint8, AUTOMATIC, CAN_APPL_DATA) Data,
                                  uint16 Length);

/**
 * @brief PduR 发送确认回调 (通知 DCM)
 * @param[in] PduId  PDU ID
 * @param[in] Result 发送结果
 */
typedef void (*PduR_TxConfirmation)(PduR_PduIdType PduId, Std_ReturnType Result);

/* ==================== 公开函数声明 ==================== */

/**
 * @brief PduR 初始化
 *
 * 注册回调链: PduR ← CANtp ← CanIf ← Can_MCAL
 */
FUNC(void, CAN_CODE)
PduR_Init(void);

/**
 * @brief PduR 主函数
 *
 * 轮询 CANtp 状态机，处理超时。
 */
FUNC(void, CAN_CODE)
PduR_MainFunction(void);

/**
 * @brief PduR 请求发送数据
 *
 * 将 DCM 的 UDS 响应通过 CANtp 发送到 CAN 总线。
 *
 * @param[in] PduId  PDU ID
 * @param[in] Data   待发送数据指针
 * @param[in] Length 数据长度
 * @return Std_ReturnType
 */
FUNC(Std_ReturnType, CAN_CODE)
PduR_Transmit(PduR_PduIdType PduId,
              CONSTP2CONST(uint8, AUTOMATIC, CAN_APPL_DATA) Data,
              uint16 Length);

/**
 * @brief PduR 取消发送
 * @param[in] PduId  PDU ID
 */
FUNC(void, CAN_CODE)
PduR_CancelTransmit(PduR_PduIdType PduId);

/**
 * @brief PduR 注册接收回调 (DCM 调用)
 * @param[in] Callback  接收回调
 */
FUNC(void, CAN_CODE)
PduR_SetRxIndication(PduR_RxIndication Callback);

/**
 * @brief PduR 注册发送确认回调 (DCM 调用)
 * @param[in] Callback  发送确认回调
 */
FUNC(void, CAN_CODE)
PduR_SetTxConfirmation(PduR_TxConfirmation Callback);

#ifdef __cplusplus
}
#endif

#endif /* PDUR_H */