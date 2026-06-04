/**
 * @file com.h
 * @brief AUTOSAR COM (Communication Manager) 接口头文件
 * @version 1.0.0
 *
 * 遵循 AUTOSAR_SWS_COM 规范。
 * 职责：
 *   - 信号打包/解包 (Signal Pack/Unpack)
 *   - 周期发送调度 (根据 DBC 定义的周期)
 *   - E2E 保护 (滚动计数器 + Checksum)
 *   - 信号超时监测
 *
 * 对应 DBC 报文：
 *   - ECU_Status     (0x1A0, TX, 50ms)
 *   - ECU_LifeCycle  (0x3A0, TX, 100ms)
 *   - Vehicle_Ctrl   (0x210, RX, 50ms)
 *   - ECU_NM_0x415   (0x415, TX, 100ms)
 */

#ifndef COM_H
#define COM_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "types.h"
#include "compiler.h"
#include "canif.h"

/* ==================== 信号 ID 枚举 ==================== */

/**
 * @enum Com_SignalIdType
 * @brief 信号 ID 枚举 (对应 DBC 信号)
 */
typedef enum
{
    /* ── ECU_Status (0x1A0) ── */
    COM_SIG_CHECKSUM       = 0U,    /**< Byte0, Intel 0-7, CRC */
    COM_SIG_ROLL_COUNTER   = 1U,    /**< Byte1, Intel 0-3, Roll Counter */
    COM_SIG_IGN_STATUS     = 2U,    /**< Byte2, Intel 0-1 */
    COM_SIG_BUTTON1_STATUS = 3U,    /**< Byte2, Intel 2-3 */
    COM_SIG_BUTTON2_STATUS = 4U,    /**< Byte2, Intel 4-5 */
    COM_SIG_SYS_VOLTAGE    = 5U,    /**< Byte3, Intel 0-7 */
    COM_SIG_ECU_TEMP       = 6U,    /**< Byte4, Intel 0-7 */
    COM_SIG_LED_PWM_DUTY   = 7U,    /**< Byte5, Intel 0-7 */

    /* ── ECU_LifeCycle (0x3A0) ── */
    COM_SIG_FLASH_COUNTER  = 8U,    /**< Byte0-1, Intel 0-15 */
    COM_SIG_ECU_ERROR_CODE = 9U,    /**< Byte2, Intel 0-7 */

    /* ── Vehicle_Ctrl (0x210, RX) ── */
    COM_SIG_VCU_CHECKSUM       = 10U, /**< Byte0, Intel 0-7 */
    COM_SIG_VCU_ROLL_COUNTER   = 11U, /**< Byte1, Intel 0-3 */
    COM_SIG_VEH_SPEED          = 12U, /**< Byte2-3, Intel 0-15 */
    COM_SIG_ENGINE_SPEED       = 13U, /**< Byte4-5, Intel 0-15 */
    COM_SIG_LED_SWITCH_CMD     = 14U, /**< Byte6, Intel 0-1 */
    COM_SIG_MOTOR_SWITCH_CMD   = 15U, /**< Byte6, Intel 2-3 */

    /* ── ECU_NM_0x415 (TX) ── */
    COM_SIG_NM_NID        = 16U,     /**< Byte0, Intel 0-7 */
    COM_SIG_NM_CBV        = 17U,     /**< Byte1, Intel 0-7 */
    COM_SIG_NM_REPEAT_MSG = 18U,     /**< Byte2, Intel 0 */
    COM_SIG_NM_ACTIVE_WAKE = 19U,    /**< Byte2, Intel 1 */
    COM_SIG_NM_SLEEP_IND  = 20U,     /**< Byte2, Intel 2 */
    COM_SIG_NM_WAKE_REASON = 21U,    /**< Byte3, Intel 0-7 */

    COM_SIG_MAX                    /**< 信号总数 */
} Com_SignalIdType;

/* ==================== PDU ID 枚举 ==================== */

#define COM_PDU_ECU_STATUS     0U
#define COM_PDU_ECU_LIFECYCLE  1U
#define COM_PDU_VEHICLE_CTRL   2U
#define COM_PDU_ECU_NM         3U

/* ==================== 公开函数声明 ==================== */

/**
 * @brief Com 初始化
 *
 * 注册 CAN 发送/接收回调到 CanIf。
 */
FUNC(void, COM_CODE)
Com_Init(void);

/**
 * @brief Com 主函数 (10ms 轮询)
 *
 * 执行以下操作：
 *   1. 更新 Rolling Counter 和 CRC 校验
 *   2. 根据周期 (50ms/100ms) 发送周期报文
 */
FUNC(void, COM_CODE)
Com_MainFunction(void);

/**
 * @brief 写信号值到发送缓冲区
 *
 * @param[in] SignalId   信号 ID
 * @param[in] SignalData 信号数据指针 (uint8* 或 uint16* 或 uint32*)
 */
FUNC(void, COM_CODE)
Com_WriteSignal(
    Com_SignalIdType  SignalId,
    CONSTP2VAR(void, AUTOMATIC, COM_APPL_DATA) SignalData
);

/**
 * @brief 从接收缓冲区读信号值
 *
 * @param[in]  SignalId   信号 ID
 * @param[out] SignalData 接收信号数据指针
 * @return Std_ReturnType
 *   @retval STD_OK    信号有效
 *   @retval STD_NOT_OK 信号超时或无效
 */
FUNC(Std_ReturnType, COM_CODE)
Com_ReadSignal(
    Com_SignalIdType       SignalId,
    P2VAR(void, AUTOMATIC, COM_APPL_DATA) SignalData
);

/**
 * @brief 获取信号超时状态
 *
 * @param[in] SignalId 信号 ID
 * @return boolean TRUE = 超时
 */
FUNC(boolean, COM_CODE)
Com_GetSignalTimeout(
    Com_SignalIdType SignalId
);

/**
 * @brief RX 指示回调 (由 CanIf 调用)
 *
 * @param[in] Pdu 接收到的 PDU
 */
void Com_RxIndication(
    CONSTP2CONST(CanIf_Pdu, AUTOMATIC, COM_APPL_DATA) Pdu
);

#ifdef __cplusplus
}
#endif

#endif /* COM_H */
