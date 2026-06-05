/**
 * @file com.c
 * @brief AUTOSAR COM (Communication Manager) 实现 — DBC Motorola 格式
 * @version 2.0.0
 *
 * 信号打包/解包 + 周期发送 + E2E (Rolling Counter + CRC)。
 * 完全遵循 DBC Motorola (@1+) 格式:
 *   - ECU_Status     (0x1A0, TX, 50ms, Motorola)
 *   - ECU_LifeCycle  (0x3A0, TX, 100ms, Motorola)
 *   - Vehicle_Ctrl   (0x210, RX, 50ms, Motorola)
 *   - ECU_NM_0x415   (0x415, TX, 100ms, Intel)
 */

#include "com.h"
#include "common.h"
#include "can_driver.h"
#include "string.h"

/* ==================== 常量 ==================== */

#define COM_PERIOD_ECU_STATUS    50U
#define COM_PERIOD_ECU_LIFECYCLE 100U
#define COM_PERIOD_ECU_NM        100U
#define COM_RX_TIMEOUT_MS        2000U

/* 信号格式 */
#define COM_FORMAT_INTEL    0U   /* Intel @0+ */
#define COM_FORMAT_MOTOROLA 1U   /* Motorola @1+ */

/* ==================== 信号描述表 ==================== */

typedef struct
{
    Com_SignalIdType  sigId;
    uint8             pduId;
    uint8             startByte;      /* Motorola: MSB byte (lowest address) */
    uint8             startBit;       /* Motorola: MSB bit position (7=MSB) */
    uint8             bitLength;
    uint8             isSigned;
    uint8             format;         /* COM_FORMAT_MOTOROLA or COM_FORMAT_INTEL */
} Com_SignalType;

/**
 * 信号描述表 (完全遵循 DBC AutoSarECU.DBC)
 *
 * DBC Motorola (@1+) 格式:
 *   startBit = DBC_startBit
 *   intel_byte = startBit / 8
 *   intel_bit  = 7 - (startBit % 8)
 */
static const Com_SignalType s_signalMap[COM_SIG_MAX] = {
    /* ========== ECU_Status (0x1A0, TX, Motorola @1+) ========== */
    {COM_SIG_ROLL_COUNTER,   COM_PDU_ECU_STATUS,     0U, 3U,  4U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:0|4 → Byte0 bits3-0 */
    {COM_SIG_CHECKSUM,       COM_PDU_ECU_STATUS,     1U, 7U,  8U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:8|8 → Byte1 */
    {COM_SIG_BUTTON1_STATUS, COM_PDU_ECU_STATUS,     2U, 7U,  2U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:16|2 → Byte2 bits7-6 */
    {COM_SIG_BUTTON2_STATUS, COM_PDU_ECU_STATUS,     2U, 5U,  2U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:18|2 → Byte2 bits5-4 */
    {COM_SIG_SYS_VOLTAGE,    COM_PDU_ECU_STATUS,     3U, 7U,  8U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:24|8 → Byte3 */
    {COM_SIG_ECU_TEMP,       COM_PDU_ECU_STATUS,     4U, 7U,  8U,  1U, COM_FORMAT_MOTOROLA}, /* DBC:32|8 → Byte4 */
    {COM_SIG_LED_PWM_DUTY,   COM_PDU_ECU_STATUS,     5U, 7U,  8U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:40|8 → Byte5 */

    /* ========== ECU_LifeCycle (0x3A0, TX, Motorola @1+) ========== */
    {COM_SIG_FLASH_COUNTER,  COM_PDU_ECU_LIFECYCLE,  0U, 7U,  16U, 0U, COM_FORMAT_MOTOROLA}, /* DBC:0|16 → Byte0-1 MSB */
    {COM_SIG_ECU_ERROR_CODE, COM_PDU_ECU_LIFECYCLE,  2U, 7U,  8U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:16|8 → Byte2 */

    /* ========== Vehicle_Ctrl (0x210, RX, Motorola @1+) ========== */
    {COM_SIG_VCU_ROLL_COUNTER,       COM_PDU_VEHICLE_CTRL, 0U, 3U,  4U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:0|4 → Byte0 bits3-0 */
    {COM_SIG_VCU_CHECKSUM,           COM_PDU_VEHICLE_CTRL, 1U, 7U,  8U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:8|8 → Byte1 */
    {COM_SIG_LED_BRIGHTNESS_LEVEL,   COM_PDU_VEHICLE_CTRL, 2U, 3U,  4U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:16|4 → Byte2 bits3-0 */
    {COM_SIG_IGN_STATUS,             COM_PDU_VEHICLE_CTRL, 2U, 5U,  2U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:20|2 → Byte2 bits5-4 */
    {COM_SIG_VEH_SPEED,              COM_PDU_VEHICLE_CTRL, 3U, 7U,  16U, 0U, COM_FORMAT_MOTOROLA}, /* DBC:24|16 → Byte3-4 */
    {COM_SIG_ENGINE_SPEED,           COM_PDU_VEHICLE_CTRL, 5U, 7U,  16U, 0U, COM_FORMAT_MOTOROLA}, /* DBC:40|16 → Byte5-6 */
    {COM_SIG_MOTOR_SWITCH_CMD,       COM_PDU_VEHICLE_CTRL, 7U, 1U,  2U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:56|2 → Byte7 bits1-0 */
    {COM_SIG_LED_SWITCH_CMD,         COM_PDU_VEHICLE_CTRL, 7U, 3U,  2U,  0U, COM_FORMAT_MOTOROLA}, /* DBC:58|2 → Byte7 bits3-2 */

    /* ========== ECU_NM_0x415 (TX, Intel @0+) ========== */
    {COM_SIG_NM_NID,         COM_PDU_ECU_NM,     0U, 0U, 8U, 0U, COM_FORMAT_INTEL},
    {COM_SIG_NM_CBV,         COM_PDU_ECU_NM,     1U, 0U, 8U, 0U, COM_FORMAT_INTEL},
    {COM_SIG_NM_REPEAT_MSG,  COM_PDU_ECU_NM,     2U, 0U, 1U, 0U, COM_FORMAT_INTEL},
    {COM_SIG_NM_ACTIVE_WAKE, COM_PDU_ECU_NM,     2U, 1U, 1U, 0U, COM_FORMAT_INTEL},
    {COM_SIG_NM_SLEEP_IND,   COM_PDU_ECU_NM,     2U, 2U, 1U, 0U, COM_FORMAT_INTEL},
    {COM_SIG_NM_WAKE_REASON, COM_PDU_ECU_NM,     3U, 0U, 8U, 0U, COM_FORMAT_INTEL},
};

/* ==================== PDU 缓冲区定义 ==================== */

#define COM_PDU_ECU_STATUS_LEN      8U
#define COM_PDU_ECU_LIFECYCLE_LEN   4U
#define COM_PDU_VEHICLE_CTRL_LEN    8U
#define COM_PDU_ECU_NM_LEN          4U

static VAR(uint8, COM_APPL_DATA) s_txPduBuf[4][8] = {{0U}};

static const uint8 s_txPduLen[4] = {
    COM_PDU_ECU_STATUS_LEN,
    COM_PDU_ECU_LIFECYCLE_LEN,
    0U,
    COM_PDU_ECU_NM_LEN
};

static VAR(uint8, COM_APPL_DATA) s_rxPduBuf[COM_PDU_VEHICLE_CTRL_LEN];
static VAR(uint32, COM_APPL_DATA) s_rxTimestamp = 0U;
static VAR(uint8, COM_APPL_DATA) s_rollCounter = 0U;
static VAR(uint32, COM_APPL_DATA) s_periodCounter = 0U;

/* ==================== 私有函数: DBC Motorola/Intel 打包/解包 ==================== */

/**
 * @brief DBC Motorola 格式信号打包 (@1+)
 *
 * Motorola 规则:
 *   - MSB 位在 startBit 指示的位置
 *   - 位从 MSB 向 LSB 递减排列
 *   - 跨字节时, 高地址字节 = LSB 部分
 */
static FUNC(void, COM_CODE)
Com_PackSignalMotorola(uint8 *pduBuf, const Com_SignalType *sig, uint32 value)
{
    uint16 bitPos = (uint16)sig->startByte * 8U + (uint16)sig->startBit;
    uint8  bitsRemaining = sig->bitLength;

    while (bitsRemaining > 0U) {
        uint8 byteIdx = bitPos / 8U;
        uint8 bitIdx  = (uint8)(bitPos % 8U);
        uint8 bitsInByte = (bitIdx >= bitsRemaining) ? bitsRemaining : (uint8)(bitIdx + 1U);

        uint8 mask = (uint8)((1U << bitsInByte) - 1U);
        uint8 val  = (uint8)(value & ((uint32)(1U << bitsInByte) - 1U));

        pduBuf[byteIdx] &= (uint8)~(mask << (bitIdx - bitsInByte + 1U));
        pduBuf[byteIdx] |= (uint8)(val << (bitIdx - bitsInByte + 1U));

        value >>= bitsInByte;
        bitsRemaining -= bitsInByte;
        bitPos = (uint16)((byteIdx + 1U) * 8U + 7U);  /* 下一字节的 MSB */
    }
}

/**
 * @brief DBC Intel 格式信号打包 (@0+)
 */
static FUNC(void, COM_CODE)
Com_PackSignalIntel(uint8 *pduBuf, const Com_SignalType *sig, uint32 value)
{
    uint16 bitPos = (uint16)sig->startByte * 8U + (uint16)sig->startBit;
    uint8  bitIdx  = (uint8)(bitPos % 8U);
    uint8  byteIdx = bitPos / 8U;
    uint8  bitsRemaining = sig->bitLength;

    while (bitsRemaining > 0U) {
        uint8 bitsInByte = (uint8)(8U - bitIdx);
        if (bitsInByte > bitsRemaining) { bitsInByte = bitsRemaining; }

        uint32 mask = (1U << bitsInByte) - 1U;
        pduBuf[byteIdx] &= (uint8)~(mask << bitIdx);
        pduBuf[byteIdx] |= (uint8)((value & mask) << bitIdx);

        value >>= bitsInByte;
        bitsRemaining -= bitsInByte;
        byteIdx++;
        bitIdx = 0U;
    }
}

/**
 * @brief DBC Motorola 格式信号解包 (@1+)
 */
static FUNC(uint32, COM_CODE)
Com_UnpackSignalMotorola(const uint8 *pduBuf, const Com_SignalType *sig)
{
    uint16 bitPos = (uint16)sig->startByte * 8U + (uint16)sig->startBit;
    uint8  bitsRemaining = sig->bitLength;
    uint32 value = 0U;
    uint8  shift = 0U;

    while (bitsRemaining > 0U) {
        uint8 byteIdx = bitPos / 8U;
        uint8 bitIdx  = (uint8)(bitPos % 8U);
        uint8 bitsInByte = (bitIdx >= bitsRemaining) ? bitsRemaining : (uint8)(bitIdx + 1U);

        uint8 mask = (uint8)((1U << bitsInByte) - 1U);
        uint8 val  = (uint8)((pduBuf[byteIdx] >> (bitIdx - bitsInByte + 1U)) & mask);

        value |= ((uint32)val) << shift;
        shift += bitsInByte;
        bitsRemaining -= bitsInByte;
        bitPos = (uint16)((byteIdx + 1U) * 8U + 7U);
    }

    return value;
}

/**
 * @brief DBC Intel 格式信号解包 (@0+)
 */
static FUNC(uint32, COM_CODE)
Com_UnpackSignalIntel(const uint8 *pduBuf, const Com_SignalType *sig)
{
    uint16 bitPos = (uint16)sig->startByte * 8U + (uint16)sig->startBit;
    uint8  byteIdx = bitPos / 8U;
    uint8  bitIdx  = (uint8)(bitPos % 8U);
    uint8  bitsRemaining = sig->bitLength;
    uint32 value = 0U;
    uint8  shift = 0U;

    while (bitsRemaining > 0U) {
        uint8 bitsInByte = (uint8)(8U - bitIdx);
        if (bitsInByte > bitsRemaining) { bitsInByte = bitsRemaining; }

        uint8 mask = (uint8)((1U << bitsInByte) - 1U);
        value |= ((uint32)((pduBuf[byteIdx] >> bitIdx) & mask)) << shift;

        shift += bitsInByte;
        bitsRemaining -= bitsInByte;
        byteIdx++;
        bitIdx = 0U;
    }

    return value;
}

/* 统一封装 */
static FUNC(void, COM_CODE)
Com_PackSignal(uint8 *pduBuf, const Com_SignalType *sig, uint32 value)
{
    if (sig->format == COM_FORMAT_MOTOROLA) {
        Com_PackSignalMotorola(pduBuf, sig, value);
    } else {
        Com_PackSignalIntel(pduBuf, sig, value);
    }
}

static FUNC(uint32, COM_CODE)
Com_UnpackSignal(const uint8 *pduBuf, const Com_SignalType *sig)
{
    if (sig->format == COM_FORMAT_MOTOROLA) {
        return Com_UnpackSignalMotorola(pduBuf, sig);
    }
    return Com_UnpackSignalIntel(pduBuf, sig);
}

/**
 * @brief E2E CRC-8 计算
 */
static FUNC(uint8, COM_CODE)
Com_CalcCRC8(const uint8 *data, uint8 len)
{
    uint8 crc = 0xFFU;
    uint8 i, j;
    for (i = 0U; i < len; i++) {
        crc ^= data[i];
        for (j = 0U; j < 8U; j++) {
            if (crc & 0x80U) { crc = (uint8)((crc << 1U) ^ 0x1DU); }
            else              { crc = (uint8)(crc << 1U); }
        }
    }
    return ~crc;
}

static FUNC(void, COM_CODE)
Com_SendPdu(uint8 pduId)
{
    CanIf_Pdu pdu;
    if (s_txPduLen[pduId] == 0U) return;

    pdu.pduId  = pduId;
    pdu.sdu    = s_txPduBuf[pduId];
    pdu.length = s_txPduLen[pduId];
    (void)CanIf_Transmit(pduId, &pdu);
}

/* ==================== 公开函数 ==================== */

FUNC(void, COM_CODE)
Com_Init(void)
{
    uint8 i;
    for (i = 0U; i < 4U; i++) { MEMSET(s_txPduBuf[i], 0U, s_txPduLen[i]); }
    MEMSET(s_rxPduBuf, 0U, COM_PDU_VEHICLE_CTRL_LEN);
    s_rollCounter   = 0U;
    s_rxTimestamp   = 0U;
    CanIf_SetRxIndication(Com_RxIndication);
}

FUNC(void, COM_CODE)
Com_MainFunction(void)
{
    s_periodCounter++;

    /* ── ECU_Status: TX, 50ms ── */
    if ((s_periodCounter % 5U) == 0U) {
        s_rollCounter = (s_rollCounter >= 0x0FU) ? 0U : (uint8)(s_rollCounter + 1U);
        uint32 rc = s_rollCounter;
        Com_WriteSignal(COM_SIG_ROLL_COUNTER, &rc);

        uint8 crcData[5];
        MEMCPY(crcData, &s_txPduBuf[COM_PDU_ECU_STATUS][1U], 5U);
        uint8 crc = Com_CalcCRC8(crcData, 5U);
        uint32 crcVal = crc;
        Com_WriteSignal(COM_SIG_CHECKSUM, &crcVal);

        Com_SendPdu(COM_PDU_ECU_STATUS);
    }

    /* ── ECU_LifeCycle: TX, 100ms ── */
    if ((s_periodCounter % 10U) == 0U) {
        Com_SendPdu(COM_PDU_ECU_LIFECYCLE);
    }

    /* ── ECU_NM_0x415: TX, 100ms ── */
    if ((s_periodCounter % 10U) == 0U) {
        Com_SendPdu(COM_PDU_ECU_NM);
    }
}

FUNC(void, COM_CODE)
Com_WriteSignal(Com_SignalIdType SignalId, CONSTP2VAR(void, AUTOMATIC, COM_APPL_DATA) SignalData)
{
    if (SignalId >= COM_SIG_MAX || SignalData == NULL_PTR) return;

    const Com_SignalType *pSig = &s_signalMap[SignalId];
    uint32 value = 0U;

    if (pSig->bitLength <= 8U)      { value = *(const uint8 *)SignalData; }
    else if (pSig->bitLength <= 16U) { value = *(const uint16 *)SignalData; }
    else                             { value = *(const uint32 *)SignalData; }

    Com_PackSignal(s_txPduBuf[pSig->pduId], pSig, value);
}

FUNC(Std_ReturnType, COM_CODE)
Com_ReadSignal(Com_SignalIdType SignalId, P2VAR(void, AUTOMATIC, COM_APPL_DATA) SignalData)
{
    if (SignalId >= COM_SIG_MAX || SignalData == NULL_PTR) return STD_NOT_OK;

    const Com_SignalType *pSig = &s_signalMap[SignalId];
    if (pSig->pduId != COM_PDU_VEHICLE_CTRL) return STD_NOT_OK;

    uint32 value = Com_UnpackSignal(s_rxPduBuf, pSig);

    if (pSig->bitLength <= 8U)      { *(uint8 *)SignalData = (uint8)value; }
    else if (pSig->bitLength <= 16U) { *(uint16 *)SignalData = (uint16)value; }
    else                             { *(uint32 *)SignalData = value; }

    return STD_OK;
}

FUNC(boolean, COM_CODE)
Com_GetSignalState(Com_SignalIdType SignalId,Com_SignalStateType *SignalState)
{
    (void)SignalId;
    if (s_rxTimestamp == 0U) {
        *SignalState = msg_never_received;
    }
    if ((HAL_GetTick() - s_rxTimestamp) > COM_RX_TIMEOUT_MS) {
        *SignalState = msg_timeout;
    }
    
    *SignalState = msg_normal;
    return TRUE;
}

void Com_RxIndication(const CanIf_Pdu *Pdu)
{
    if (Pdu == NULL_PTR) return;

    if (Pdu->pduId == COM_PDU_VEHICLE_CTRL) {
        uint8 copyLen = (Pdu->length > COM_PDU_VEHICLE_CTRL_LEN)
                        ? COM_PDU_VEHICLE_CTRL_LEN : Pdu->length;

        MEMCPY(s_rxPduBuf, Pdu->sdu, copyLen);
        s_rxTimestamp = HAL_GetTick();

        uint32 rxCRC = Com_UnpackSignal(s_rxPduBuf, &s_signalMap[COM_SIG_VCU_CHECKSUM]);
        uint32 rxRC  = Com_UnpackSignal(s_rxPduBuf, &s_signalMap[COM_SIG_VCU_ROLL_COUNTER]);

        (void)rxRC;

        uint8 crcData[7];
        MEMCPY(crcData, s_rxPduBuf, 7U);
        uint8 calcCRC = Com_CalcCRC8(crcData, 7U);

        if ((uint8)rxCRC != calcCRC) {
            s_rxTimestamp = 0U;
        }
    }
}

//For test
 FUNC(void, COM_CODE)
 Com_TestFunction(void)
 {
     uint8 ledlvl=0U;
     uint8 ignstatus=0U;
     uint16 vehspeed=0U;
     uint16 enginespeed=0U;
     uint8 motorcmd=0U;
     uint8 ledcmd=0U;
     Com_ReadSignal(COM_SIG_LED_BRIGHTNESS_LEVEL,&ledlvl);
     printf("ledlvl%d\n",ledlvl);
     Com_ReadSignal(COM_SIG_IGN_STATUS,&ignstatus);
     printf("ignstatus%d\n",ignstatus);
     Com_ReadSignal(COM_SIG_VEH_SPEED,&vehspeed);
     printf("vehspeed%d\n",vehspeed);
     Com_ReadSignal(COM_SIG_ENGINE_SPEED,&enginespeed);
     printf("enginespeed %d\n",enginespeed);
     Com_ReadSignal(COM_SIG_MOTOR_SWITCH_CMD,&motorcmd);
     printf("motorcmd %d\n",motorcmd);
     Com_ReadSignal(COM_SIG_LED_SWITCH_CMD,&ledcmd);
     printf("ledcmd %d\n",ledcmd);
 }
