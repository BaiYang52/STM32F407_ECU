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
    /* ── ECU_Status (0x1A0, TX, Motorola @1+) ── */
    COM_SIG_ROLL_COUNTER   = 0U,    /**< Byte0, bit0-3, DBC: 0|4 */
    COM_SIG_CHECKSUM       = 1U,    /**< Byte1, bit0-7, DBC: 8|8 */
    COM_SIG_BUTTON1_STATUS = 2U,    /**< Byte2, bit0-1, DBC: 16|2 */
    COM_SIG_BUTTON2_STATUS = 3U,    /**< Byte2, bit2-3, DBC: 18|2 */
    COM_SIG_SYS_VOLTAGE    = 4U,    /**< Byte3, bit0-7, DBC: 24|8 */
    COM_SIG_ECU_TEMP       = 5U,    /**< Byte4, bit0-7, DBC: 32|8 */
    COM_SIG_LED_PWM_DUTY   = 6U,    /**< Byte5, bit0-7, DBC: 40|8 */

    /* ── ECU_LifeCycle (0x3A0, TX, Motorola @1+) ── */
    COM_SIG_FLASH_COUNTER  = 7U,    /**< Byte0-1, DBC: 0|16 (Motorola MSB first) */
    COM_SIG_ECU_ERROR_CODE = 8U,    /**< Byte2, DBC: 16|8 */

    /* ── Vehicle_Ctrl (0x210, RX, Motorola @1+) ── */
    COM_SIG_VCU_ROLL_COUNTER       = 9U,  /**< Byte0, bit0-3, DBC: 0|4 */
    COM_SIG_VCU_CHECKSUM           = 10U, /**< Byte1, bit0-7, DBC: 8|8 */
    COM_SIG_LED_BRIGHTNESS_LEVEL   = 11U, /**< Byte2, bit0-3, DBC: 16|4 */
    COM_SIG_IGN_STATUS             = 12U, /**< Byte2, bit4-5, DBC: 20|2 */
    COM_SIG_VEH_SPEED              = 13U, /**< Byte3-4, DBC: 24|16 (Motorola: Byte3=MSB) */
    COM_SIG_ENGINE_SPEED           = 14U, /**< Byte5-6, DBC: 40|16 (Motorola: Byte5=MSB) */
    COM_SIG_MOTOR_SWITCH_CMD       = 15U, /**< Byte7, bit0-1, DBC: 56|2 */
    COM_SIG_LED_SWITCH_CMD         = 16U, /**< Byte7, bit2-3, DBC: 58|2 */

    /* ── ECU_NM_0x415 (TX, Intel @0+) ── */
    COM_SIG_NM_NID        = 17U,     /**< Byte0, Intel 0-7 */
    COM_SIG_NM_CBV        = 18U,     /**< Byte1, Intel 0-7 */
    COM_SIG_NM_REPEAT_MSG = 19U,     /**< Byte2, Intel 0 */
    COM_SIG_NM_ACTIVE_WAKE = 20U,    /**< Byte2, Intel 1 */
    COM_SIG_NM_SLEEP_IND  = 21U,     /**< Byte2, Intel 2 */
    COM_SIG_NM_WAKE_REASON = 22U,    /**< Byte3, Intel 0-7 */

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
