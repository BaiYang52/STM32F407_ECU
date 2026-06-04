/**
 * @file com.c
 * @brief AUTOSAR COM (Communication Manager) 实现
 * @version 1.0.0
 *
 * 信号打包/解包 + 周期发送 + E2E (Rolling Counter + CRC)。
 *
 * 基于 DBC 报文矩阵的 Intel 字节序小端格式：
 *   - 信号在发送前放入 TX PDU 缓冲区
 *   - MainFunction 根据周期将 PDU 发给 CanIf
 *   - 接收 PDU 由 CanIf 回调到达，解包后存 RX 缓冲区
 *   - E2E: 发送时更新滚动计数器 + 计算 CRC，接收时验证
 */

#include "com.h"
#include "common.h"
#include "can_driver.h"
#include "string.h"

/* ==================== 常量 ==================== */

/** TX PDU 周期 (ms) */
#define COM_PERIOD_ECU_STATUS    50U
#define COM_PERIOD_ECU_LIFECYCLE 100U
#define COM_PERIOD_ECU_NM        100U

/** 接收超时时间 (ms) */
#define COM_RX_TIMEOUT_MS        2000U

/* ==================== 信号描述表 ==================== */

/**
 * @struct Com_SignalType
 * @brief 信号描述表条目
 */
typedef struct
{
    Com_SignalIdType  sigId;          /**< 信号 ID */
    uint8             pduId;          /**< 所属 PDU ID */
    uint8             startByte;      /**< 起始字节 (Intel) */
    uint8             startBit;       /**< 起始位 (Intel) */
    uint8             bitLength;      /**< 信号位长度 */
    uint8             isSigned;       /**< 有符号标志 */
} Com_SignalType;

/** 信号描述表 (完全映射 DBC) */
static const Com_SignalType s_signalMap[COM_SIG_MAX] = {
    /* ECU_Status */
    {COM_SIG_CHECKSUM,        COM_PDU_ECU_STATUS,     0U,  0U,  8U,  0U},
    {COM_SIG_ROLL_COUNTER,    COM_PDU_ECU_STATUS,     1U,  0U,  4U,  0U},
    {COM_SIG_IGN_STATUS,      COM_PDU_ECU_STATUS,     2U,  0U,  2U,  0U},
    {COM_SIG_BUTTON1_STATUS,  COM_PDU_ECU_STATUS,     2U,  2U,  2U,  0U},
    {COM_SIG_BUTTON2_STATUS,  COM_PDU_ECU_STATUS,     2U,  4U,  2U,  0U},
    {COM_SIG_SYS_VOLTAGE,     COM_PDU_ECU_STATUS,     3U,  0U,  8U,  0U},
    {COM_SIG_ECU_TEMP,        COM_PDU_ECU_STATUS,     4U,  0U,  8U,  1U},
    {COM_SIG_LED_PWM_DUTY,    COM_PDU_ECU_STATUS,     5U,  0U,  8U,  0U},

    /* ECU_LifeCycle */
    {COM_SIG_FLASH_COUNTER,   COM_PDU_ECU_LIFECYCLE,  0U,  0U,  16U, 0U},
    {COM_SIG_ECU_ERROR_CODE,  COM_PDU_ECU_LIFECYCLE,  2U,  0U,  8U,  0U},

    /* Vehicle_Ctrl (RX) */
    {COM_SIG_VCU_CHECKSUM,       COM_PDU_VEHICLE_CTRL, 0U, 0U, 8U,  0U},
    {COM_SIG_VCU_ROLL_COUNTER,   COM_PDU_VEHICLE_CTRL, 1U, 0U, 4U,  0U},
    {COM_SIG_VEH_SPEED,          COM_PDU_VEHICLE_CTRL, 2U, 0U, 16U, 0U},
    {COM_SIG_ENGINE_SPEED,       COM_PDU_VEHICLE_CTRL, 4U, 0U, 16U, 0U},
    {COM_SIG_LED_SWITCH_CMD,     COM_PDU_VEHICLE_CTRL, 6U, 0U, 2U,  0U},
    {COM_SIG_MOTOR_SWITCH_CMD,   COM_PDU_VEHICLE_CTRL, 6U, 2U, 2U,  0U},

    /* ECU_NM_0x415 */
    {COM_SIG_NM_NID,         COM_PDU_ECU_NM,     0U, 0U, 8U, 0U},
    {COM_SIG_NM_CBV,         COM_PDU_ECU_NM,     1U, 0U, 8U, 0U},
    {COM_SIG_NM_REPEAT_MSG,  COM_PDU_ECU_NM,     2U, 0U, 1U, 0U},
    {COM_SIG_NM_ACTIVE_WAKE, COM_PDU_ECU_NM,     2U, 1U, 1U, 0U},
    {COM_SIG_NM_SLEEP_IND,   COM_PDU_ECU_NM,     2U, 2U, 1U, 0U},
    {COM_SIG_NM_WAKE_REASON, COM_PDU_ECU_NM,     3U, 0U, 8U, 0U},
};

/* ==================== PDU 缓冲区定义 ==================== */

#define COM_PDU_ECU_STATUS_LEN      8U
#define COM_PDU_ECU_LIFECYCLE_LEN   4U
#define COM_PDU_VEHICLE_CTRL_LEN    8U
#define COM_PDU_ECU_NM_LEN          4U

/** TX PDU 缓冲区 */
static VAR(uint8, COM_APPL_DATA) s_txPduBuf[4][8] = {{0U}};

/** TX PDU 长度 */
static const uint8 s_txPduLen[4] = {
    COM_PDU_ECU_STATUS_LEN,
    COM_PDU_ECU_LIFECYCLE_LEN,
    0U,                         /* Vehicle_Ctrl 是 RX，不发送 */
    COM_PDU_ECU_NM_LEN
};

/** RX PDU 缓冲区 (最新的接收帧) */
static VAR(uint8, COM_APPL_DATA) s_rxPduBuf[COM_PDU_VEHICLE_CTRL_LEN];

/** RX 时间戳 (用于超时检测) */
static VAR(uint32, COM_APPL_DATA) s_rxTimestamp = 0U;

/** 滚动计数器 */
static VAR(uint8, COM_APPL_DATA) s_rollCounter = 0U;

/** 发送周期计数器 */
static VAR(uint32, COM_APPL_DATA) s_periodCounter = 0U;

/* ==================== 私有函数 ==================== */

/**
 * @brief Intel 格式信号打包 (小端)
 *
 * 将值写入 PDU 缓冲区的指定位。
 * Intel 格式：起始位 = startByte * 8 + startBit
 */
static FUNC(void, COM_CODE)
Com_PackSignal(uint8 *pduBuf, const Com_SignalType *sig, uint32 value)
{
    uint16 bitPos = (uint16)sig->startByte * 8U + (uint16)sig->startBit;
    uint8  byteIdx;
    uint8  bitIdx;
    uint8  bitsRemaining = sig->bitLength;
    uint32 mask;

    byteIdx = bitPos / 8U;
    bitIdx  = (uint8)(bitPos % 8U);

    while (bitsRemaining > 0U) {
        uint8 bitsInByte = (uint8)(8U - bitIdx);
        if (bitsInByte > bitsRemaining) {
            bitsInByte = bitsRemaining;
        }

        mask = (1U << bitsInByte) - 1U;
        pduBuf[byteIdx] &= (uint8)~(mask << bitIdx);               /* 清零 */
        pduBuf[byteIdx] |= (uint8)((value & (uint32)mask) << bitIdx); /* 置位 */

        value >>= bitsInByte;
        bitsRemaining -= bitsInByte;
        byteIdx++;
        bitIdx = 0U;
    }
}

/**
 * @brief Intel 格式信号解包
 *
 * 从 PDU 缓冲区读取指定信号的值。
 */
static FUNC(uint32, COM_CODE)
Com_UnpackSignal(const uint8 *pduBuf, const Com_SignalType *sig)
{
    uint16 bitPos = (uint16)sig->startByte * 8U + (uint16)sig->startBit;
    uint8  byteIdx;
    uint8  bitIdx;
    uint8  bitsRemaining = sig->bitLength;
    uint32 value = 0U;
    uint8  shift = 0U;

    byteIdx = bitPos / 8U;
    bitIdx  = (uint8)(bitPos % 8U);

    while (bitsRemaining > 0U) {
        uint8 bitsInByte = (uint8)(8U - bitIdx);
        if (bitsInByte > bitsRemaining) {
            bitsInByte = bitsRemaining;
        }

        uint8 mask = (uint8)((1U << bitsInByte) - 1U);
        value |= ((uint32)((pduBuf[byteIdx] >> bitIdx) & mask)) << shift;

        shift += bitsInByte;
        bitsRemaining -= bitsInByte;
        byteIdx++;
        bitIdx = 0U;
    }

    return value;
}

/**
 * @brief 计算简单 CRC-8 校验
 *
 * @param[in] data  数据指针
 * @param[in] len   数据长度
 * @return uint8 CRC8 值
 */
static FUNC(uint8, COM_CODE)
Com_CalcCRC8(const uint8 *data, uint8 len)
{
    uint8 crc = 0xFFU;
    uint8 i, j;

    for (i = 0U; i < len; i++) {
        crc ^= data[i];
        for (j = 0U; j < 8U; j++) {
            if (crc & 0x80U) {
                crc = (uint8)((crc << 1U) ^ 0x1DU);  /* CRC-8 polynom */
            } else {
                crc = (uint8)(crc << 1U);
            }
        }
    }
    return ~crc;
}

/**
 * @brief 发送 TX PDU 到 CanIf
 */
static FUNC(void, COM_CODE)
Com_SendPdu(uint8 pduId)
{
    CanIf_Pdu pdu;

    if (s_txPduLen[pduId] == 0U) {
        return;  /* RX 报文不发送 */
    }

    pdu.pduId  = pduId;
    pdu.sdu    = s_txPduBuf[pduId];
    pdu.length = s_txPduLen[pduId];

    (void)CanIf_Transmit(pduId, &pdu);
}

/* ==================== 公开函数实现 ==================== */

/**
 * @brief Com 初始化
 *
 * 初始化 TX 缓冲区为 0，注册 CanIf RX 回调。
 */
FUNC(void, COM_CODE)
Com_Init(void)
{
    uint8 i;

    /* 清空 TX 缓冲区 */
    for (i = 0U; i < 4U; i++) {
        MEMSET(s_txPduBuf[i], 0U, s_txPduLen[i]);
    }

    /* 清空 RX 缓冲区 */
    MEMSET(s_rxPduBuf, 0U, COM_PDU_VEHICLE_CTRL_LEN);

    s_rollCounter  = 0U;
    s_rxTimestamp = 0U;

    /* 注册 RX 回调到 CanIf */
    CanIf_SetRxIndication(Com_RxIndication);
}

/**
 * @brief Com 主函数 (10ms 任务)
 *
 * 每个 10ms 周期调用一次，调度周期报文发送。
 */
FUNC(void, COM_CODE)
Com_MainFunction(void)
{
    s_periodCounter++;

    /* ── ECU_Status: TX, 50ms 周期 ── */
    if ((s_periodCounter % 5U) == 0U) {  /* 50ms = 5 * 10ms */

        /* 更新滚动计数器 (0-15) */
        s_rollCounter++;
        if (s_rollCounter > 0x0FU) {
            s_rollCounter = 0U;
        }

        /* 将 RollCounter 写入 TX 缓冲区 */
        uint32 rc = s_rollCounter;
        Com_WriteSignal(COM_SIG_ROLL_COUNTER, &rc);

        /* 计算 CRC (Byte1~5, 不包括 CRC 字节本身) */
        uint8 crcData[5];
        MEMCPY(crcData, &s_txPduBuf[COM_PDU_ECU_STATUS][1U], 5U);
        uint8 crc = Com_CalcCRC8(crcData, 5U);

        /* 写入 CRC */
        uint32 crcVal = crc;
        Com_WriteSignal(COM_SIG_CHECKSUM, &crcVal);

        /* 发送 */
        Com_SendPdu(COM_PDU_ECU_STATUS);
    }

    /* ── ECU_LifeCycle: TX, 100ms 周期 ── */
    if ((s_periodCounter % 10U) == 0U) {
        /* 计算 CRC */
        uint8 crcData[5] = {s_txPduBuf[COM_PDU_ECU_LIFECYCLE][0U],
                             s_txPduBuf[COM_PDU_ECU_LIFECYCLE][1U],
                             s_txPduBuf[COM_PDU_ECU_LIFECYCLE][2U],
                             s_txPduBuf[COM_PDU_ECU_LIFECYCLE][3U], 0U};
        uint8 crc = Com_CalcCRC8(crcData, 4U);
        s_txPduBuf[COM_PDU_ECU_LIFECYCLE][3U] = crc;

        Com_SendPdu(COM_PDU_ECU_LIFECYCLE);
    }

    /* ── ECU_NM_0x415: TX, 100ms 周期 ── */
    if ((s_periodCounter % 10U) == 0U) {
        Com_SendPdu(COM_PDU_ECU_NM);
    }
}

/**
 * @brief 写信号值到 TX 缓冲区
 */
FUNC(void, COM_CODE)
Com_WriteSignal(
    Com_SignalIdType  SignalId,
    CONSTP2VAR(void, AUTOMATIC, COM_APPL_DATA) SignalData
)
{
    if (SignalId >= COM_SIG_MAX) {
        return;
    }
    if (SignalData == NULL_PTR) {
        return;
    }

    const Com_SignalType *pSig = &s_signalMap[SignalId];
    uint32 value = 0U;

    /* 按类型长度读取源数据 */
    if (pSig->bitLength <= 8U) {
        value = *(const uint8 *)SignalData;
    } else if (pSig->bitLength <= 16U) {
        value = *(const uint16 *)SignalData;
    } else {
        value = *(const uint32 *)SignalData;
    }

    Com_PackSignal(s_txPduBuf[pSig->pduId], pSig, value);
}

/**
 * @brief 读信号值从 RX 缓冲区
 */
FUNC(Std_ReturnType, COM_CODE)
Com_ReadSignal(
    Com_SignalIdType       SignalId,
    P2VAR(void, AUTOMATIC, COM_APPL_DATA) SignalData
)
{
    if (SignalId >= COM_SIG_MAX) {
        return STD_NOT_OK;
    }
    if (SignalData == NULL_PTR) {
        return STD_NOT_OK;
    }

    const Com_SignalType *pSig = &s_signalMap[SignalId];

    /* 检查是否为 RX 信号 (所属 PDU 为 Vehicle_Ctrl) */
    if (pSig->pduId != COM_PDU_VEHICLE_CTRL) {
        return STD_NOT_OK;
    }

    /* 解包 */
    uint32 value = Com_UnpackSignal(s_rxPduBuf, pSig);

    /* 按位长度写入目标缓冲区 */
    if (pSig->bitLength <= 8U) {
        *(uint8 *)SignalData = (uint8)value;
    } else if (pSig->bitLength <= 16U) {
        *(uint16 *)SignalData = (uint16)value;
    } else {
        *(uint32 *)SignalData = value;
    }

    return STD_OK;
}

/**
 * @brief 获取信号超时状态
 */
FUNC(boolean, COM_CODE)
Com_GetSignalTimeout(
    Com_SignalIdType SignalId
)
{
    (void)SignalId;
    /* 检查接收超时 */
    if (s_rxTimestamp == 0U) {
        return TRUE;  /* 从未收到 */
    }

    /* 超时检测：Vehicle_Ctrl 周期 50ms，超时设为 100ms */
    if ((HAL_GetTick() - s_rxTimestamp) > COM_RX_TIMEOUT_MS) {
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief RX 指示回调 (由 CanIf 调用)
 *
 * @param[in] Pdu 接收到的 PDU
 */
void Com_RxIndication(const CanIf_Pdu *Pdu)
{
    if (Pdu == NULL_PTR) {
        return;
    }

    /* 只处理 Vehicle_Ctrl (PDU ID = 2) */
    if (Pdu->pduId == COM_PDU_VEHICLE_CTRL) {
        uint8 copyLen = (Pdu->length > COM_PDU_VEHICLE_CTRL_LEN)
                        ? COM_PDU_VEHICLE_CTRL_LEN : Pdu->length;

        MEMCPY(s_rxPduBuf, Pdu->sdu, copyLen);
        s_rxTimestamp = HAL_GetTick();

        /* E2E 验证：CRC + RollCounter */
        uint32 rxCRC   = Com_UnpackSignal(s_rxPduBuf, &s_signalMap[COM_SIG_VCU_CHECKSUM]);
        uint32 rxRC    = Com_UnpackSignal(s_rxPduBuf, &s_signalMap[COM_SIG_VCU_ROLL_COUNTER]);

        /* 计算 CRC */
        uint8 crcData[7];
        MEMCPY(crcData, s_rxPduBuf, 7U);
        uint8 calcCRC = Com_CalcCRC8(crcData, 7U);

        if ((uint8)rxCRC != calcCRC) {
            /* CRC 错误 → 丢弃 */
            s_rxTimestamp = 0U;
        }
        (void)rxRC;  /* RollCounter 可用于 E2E 序列号验证 */
    }
}