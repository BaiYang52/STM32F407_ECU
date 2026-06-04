/**
 * @file cantp.c
 * @brief AUTOSAR CAN Transport Layer (ISO 15765-2) 实现
 * @version 1.0.0
 *
 * 实现 ISO 15765-2 传输层协议，处理 UDS 诊断报文的收发分帧。
 *
 * 协议流程:
 *   SF  (<8 bytes): 直接交付 → N-SDU
 *   FF+CF (>8 bytes): FF → FC → CF[0..N] → 重组交付
 *
 * 定时器参数 (来自需求文档 CANFD Application Software Diagnostic Session):
 *   N_As / N_Ar = 25ms
 *   N_Bs = 75ms
 *   N_Cr = 150ms
 *
 * 通道配置:
 *   Channel 0: UDS Physical  (0x7A0 -> 0x7A8)
 *   Channel 1: UDS Functional (0x7DF, RX only)
 */

#include "cantp.h"
#include "common.h"
#include "timer_driver.h"
#include "stm32f4xx_hal.h"

/* ==================== 常量 ==================== */

/** 最大 CAN TP 通道数 */
#define CANTP_NUM_CHANNELS          2U

/** PDU ID 定义 */
#define CANTP_RX_PDU_PHYSICAL       (CanIf_PduIdType)4U   /* 0x7A0 */
#define CANTP_TX_PDU_PHYSICAL       (CanIf_PduIdType)5U   /* 0x7A8 */
#define CANTP_RX_PDU_FUNCTIONAL     (CanIf_PduIdType)6U   /* 0x7DF (新 PDU) */

/** 填充字节 */
#define CANTP_FILLER_BYTE           0xFFU   /* 来自需求文档 */

/** CF 每帧最大数据字节 (经典 CAN: 8 bytes - 1 byte PCI) */
#define CANTP_CF_DATA_LEN           7U

/** FF 第一帧数据字节 */
#define CANTP_FF_DATA_LEN           6U

/** SF 最大数据长度 (经典 CAN: 8 bytes - 1 byte PCI) */
#define CANTP_SF_MAX_DATA_LEN       7U

/* ==================== 通道配置表 ==================== */

/** 通道上下文数组 */
static CanTp_ChannelType s_channels[CANTP_NUM_CHANNELS];

/* ==================== 回调注册 ==================== */

/** 接收完成回调 (→ PduR) */
static CanTp_RxIndication s_rxCallback = NULL_PTR;

/** 发送确认回调 (→ PduR) */
static CanTp_TxConfirmation s_txCallback = NULL_PTR;

/** 发送缓冲区 (for MF) */
static uint8 s_txDataBuf[CANTP_TX_BUFFER_SIZE];

/* ==================== 私有函数声明 ==================== */

static void CanTp_ResetChannel(CanTp_ChannelType *ch);
static void CanTp_SendFlowControl(CanTp_ChannelType *ch, uint8 fs, uint8 bs, uint8 stmin);
static void CanTp_SendConsecutiveFrame(CanTp_ChannelType *ch);
static void CanTp_SendSingleFrame(CanTp_ChannelType *ch, const uint8 *data, uint16 len);
static void CanTp_SendFirstFrame(CanTp_ChannelType *ch, const uint8 *data, uint16 len);
static void CanTp_ProcessRxFrame(CanTp_ChannelType *ch, const CanIf_Pdu *Pdu);
static void CanTp_ProcessRxSF(CanTp_ChannelType *ch, const uint8 *sdu, uint8 len);
static void CanTp_ProcessRxFF(CanTp_ChannelType *ch, const uint8 *sdu, uint8 len);
static void CanTp_ProcessRxCF(CanTp_ChannelType *ch, const uint8 *sdu, uint8 len);
static void CanTp_ProcessRxFC(CanTp_ChannelType *ch, const uint8 *sdu, uint8 len);
static uint32 CanTp_GetTimestamp(void);

/* ==================== 公开函数实现 ==================== */

FUNC(void, CAN_CODE)
CanTp_Init(void)
{
    uint8 i;
    for (i = 0U; i < CANTP_NUM_CHANNELS; i++) {
        CanTp_ResetChannel(&s_channels[i]);
    }

    /* 配置通道 0: UDS Physical */
    s_channels[0].channelId = 0U;
    s_channels[0].rxPduId   = CANTP_RX_PDU_PHYSICAL;   /* CAN ID: 0x7A0 */
    s_channels[0].txPduId   = CANTP_TX_PDU_PHYSICAL;   /* CAN ID: 0x7A8 */

    /* 配置通道 1: UDS Functional */
    s_channels[1].channelId = 1U;
    s_channels[1].rxPduId   = CANTP_RX_PDU_FUNCTIONAL;  /* CAN ID: 0x7DF */
    s_channels[1].txPduId   = 0xFFU;                     /* 不发送响应 */

    /* 注册 CANtp 的回调到 CanIf (接收 UDS PDU 4/6) */
    CanIf_SetRxIndication2(CanTp_HandleRxPdu);
}

FUNC(Std_ReturnType, CAN_CODE)
CanTp_Transmit(uint8 channelId,
               CONSTP2CONST(uint8, AUTOMATIC, CAN_APPL_DATA) data,
               uint16 length)
{
    CanTp_ChannelType *ch;
    CanIf_Pdu pdu;

    if (channelId >= CANTP_NUM_CHANNELS) {
        return STD_NOT_OK;
    }
    if (data == NULL_PTR) {
        return STD_NOT_OK;
    }
    if (length == 0U || length > CANTP_MAX_NSDU_LENGTH) {
        return STD_NOT_OK;
    }

    ch = &s_channels[channelId];

    /* 检查通道是否空闲 */
    if (ch->state != CANTP_CH_IDLE) {
        return STD_NOT_OK;
    }

    /* 单帧 (SF): len <= 7 bytes */
    if (length <= CANTP_SF_MAX_DATA_LEN) {
        /* 填充 CAN 帧: PCI byte + data */
        uint8 frameData[8];
        uint8 idx;
        frameData[0] = (uint8)(CANTP_PCI_TYPE_SF | (uint8)length);  /* PCI: SF + len */
        MEMCPY(&frameData[1], data, (uint8)length);
        /* 剩余填充 */
        for (idx = (length + 1U); idx < 8U; idx++) {
            frameData[idx] = CANTP_FILLER_BYTE;
        }

        pdu.pduId  = ch->txPduId;
        pdu.sdu    = frameData;
        pdu.length = 8U;

        (void)CanIf_Transmit(ch->txPduId, &pdu);

        /* 通知上层发送完成 */
        if (s_txCallback != NULL_PTR) {
            s_txCallback(channelId, STD_OK);
        }
        return STD_OK;
    }

    /* 多帧 (FF+CF): len > 7 bytes */
    /* 保存待发送数据到缓冲区 */
    MEMCPY(s_txDataBuf, data, length);
    ch->txBuffer     = s_txDataBuf;
    ch->txLength     = length;
    ch->txIndex      = 0U;
    ch->txSN         = 1U;   /* Sequence Number starts at 1 */
    ch->txBlockRemaining = 0U;
    ch->txPending    = TRUE;

    /* 发送首帧 (FF) */
    CanTp_SendFirstFrame(ch, s_txDataBuf, length);

    /* 进入等待 FC 状态 */
    ch->state   = CANTP_CH_WAIT_FC;
    ch->txTimer = CanTp_GetTimestamp();

    return STD_OK;
}

FUNC(void, CAN_CODE)
CanTp_CancelTransmit(uint8 channelId)
{
    if (channelId < CANTP_NUM_CHANNELS) {
        CanTp_ResetChannel(&s_channels[channelId]);
    }
}

FUNC(void, CAN_CODE)
CanTp_MainFunction(void)
{
    uint8 i;
    uint32 now = CanTp_GetTimestamp();

    for (i = 0U; i < CANTP_NUM_CHANNELS; i++) {
        CanTp_ChannelType *ch = &s_channels[i];

        switch (ch->state) {
        case CANTP_CH_WAIT_FC:
            /* 超时检查: N_Bs = 75ms */
            if ((now - ch->txTimer) >= CANTP_N_BS_MS) {
                /* 超时 → 复位通道，通知失败 */
                CanTp_ResetChannel(ch);
                if (s_txCallback != NULL_PTR) {
                    s_txCallback(ch->channelId, STD_NOT_OK);
                }
            }
            break;

        case CANTP_CH_SENDING_CF:
            /* 发送下一帧 CF */
            CanTp_SendConsecutiveFrame(ch);
            break;

        case CANTP_CH_WAIT_CF:
            /* 超时检查: N_Cr = 150ms */
            if ((now - ch->rxTimer) >= CANTP_N_CR_MS) {
                /* 接收超时 → 复位 */
                CanTp_ResetChannel(ch);
            }
            break;

        case CANTP_CH_READY:
            /* 交付 N-SDU 给上层 */
            if (ch->rxComplete && (s_rxCallback != NULL_PTR)) {
                CanTp_PduInfoType pduInfo;
                pduInfo.data           = ch->rxBuffer;
                pduInfo.length         = (uint16)ch->rxLength;
                pduInfo.protocolResult = 0U;
                s_rxCallback(ch->channelId, &pduInfo);
                ch->rxComplete = FALSE;
                CanTp_ResetChannel(ch);
            }
            break;

        default:
            break;
        }
    }
}

FUNC(void, CAN_CODE)
CanTp_HandleRxPdu(CONSTP2CONST(CanIf_Pdu, AUTOMATIC, CAN_APPL_DATA) Pdu)
{
    uint8 i;

    if (Pdu == NULL_PTR) {
        return;
    }
    if (Pdu->sdu == NULL_PTR) {
        return;
    }

    /* 查找匹配的通道 (rxPduId 匹配) */
    for (i = 0U; i < CANTP_NUM_CHANNELS; i++) {
        if (s_channels[i].rxPduId == Pdu->pduId) {
            CanTp_ProcessRxFrame(&s_channels[i], Pdu);
            return;
        }
    }
}

FUNC(void, CAN_CODE)
CanTp_SetRxIndication(CanTp_RxIndication Callback)
{
    s_rxCallback = Callback;
}

FUNC(void, CAN_CODE)
CanTp_SetTxConfirmation(CanTp_TxConfirmation Callback)
{
    s_txCallback = Callback;
}

FUNC(uint8, CAN_CODE)
CanTp_GetChannelCount(void)
{
    return CANTP_NUM_CHANNELS;
}

/* ==================== 私有函数实现 ==================== */

/**
 * @brief 复位通道上下文
 */
static void CanTp_ResetChannel(CanTp_ChannelType *ch)
{
    ch->state         = CANTP_CH_IDLE;
    ch->rxLength      = 0U;
    ch->rxIndex       = 0U;
    ch->rxExpectedSN  = 1U;
    ch->rxTimer       = 0U;
    ch->rxComplete    = FALSE;
    ch->txBuffer      = NULL_PTR;
    ch->txLength      = 0U;
    ch->txIndex       = 0U;
    ch->txSN          = 1U;
    ch->txBlockRemaining = 0U;
    ch->txTimer       = 0U;
    ch->txPending     = FALSE;
}

/**
 * @brief 发送流控帧 (FC)
 */
static void CanTp_SendFlowControl(CanTp_ChannelType *ch, uint8 fs, uint8 bs, uint8 stmin)
{
    CanIf_Pdu pdu;
    uint8 fcData[8];
    uint8 i;

    fcData[0] = CANTP_PCI_TYPE_FC | (fs & 0x0FU);  /* PCI: FC + FS */
    fcData[1] = bs;                                  /* Block Size */
    fcData[2] = stmin;                               /* STmin */

    /* 填充剩余字节 */
    for (i = 3U; i < 8U; i++) {
        fcData[i] = CANTP_FILLER_BYTE;
    }

    pdu.pduId  = ch->txPduId;
    pdu.sdu    = fcData;
    pdu.length = 8U;

    (void)CanIf_Transmit(ch->txPduId, &pdu);
}

/**
 * @brief 发送连续帧 (CF)
 */
static void CanTp_SendConsecutiveFrame(CanTp_ChannelType *ch)
{
    CanIf_Pdu pdu;
    uint8 cfData[8];
    uint16 remaining;
    uint8  copyLen;
    uint8  idx;

    if (ch->txIndex >= ch->txLength) {
        /* 全部发送完成 */
        CanTp_ResetChannel(ch);
        if (s_txCallback != NULL_PTR) {
            s_txCallback(ch->channelId, STD_OK);
        }
        return;
    }

    /* 构建 CF PCI */
    cfData[0] = CANTP_PCI_TYPE_CF | (ch->txSN & 0x0FU);

    /* 计算本帧数据长度 */
    remaining = ch->txLength - ch->txIndex;
    copyLen = (remaining > CANTP_CF_DATA_LEN) ? CANTP_CF_DATA_LEN : (uint8)remaining;

    /* 复制数据 */
    MEMCPY(&cfData[1], &ch->txBuffer[ch->txIndex], copyLen);

    /* 填充剩余字节 */
    for (idx = (copyLen + 1U); idx < 8U; idx++) {
        cfData[idx] = CANTP_FILLER_BYTE;
    }

    pdu.pduId  = ch->txPduId;
    pdu.sdu    = cfData;
    pdu.length = 8U;

    (void)CanIf_Transmit(ch->txPduId, &pdu);

    /* 更新发送状态 */
    ch->txIndex += copyLen;
    ch->txSN++;
    if (ch->txSN >= 16U) {
        ch->txSN = 0U;  /* 回绕 */
    }

    /* 更新块计数器 */
    ch->txBlockRemaining--;

    /* 回到空闲状态 (BS=0 时不需流控) */
    if (ch->txIndex >= ch->txLength) {
        CanTp_ResetChannel(ch);
        if (s_txCallback != NULL_PTR) {
            s_txCallback(ch->channelId, STD_OK);
        }
    }
}

/**
 * @brief 发送单帧 (SF)
 */
static void CanTp_SendSingleFrame(CanTp_ChannelType *ch, const uint8 *data, uint16 len)
{
    (void)ch;
    (void)data;
    (void)len;
    /* CanTp_Transmit 中直接构建 SF */
}

/**
 * @brief 发送首帧 (FF)
 */
static void CanTp_SendFirstFrame(CanTp_ChannelType *ch, const uint8 *data, uint16 len)
{
    CanIf_Pdu pdu;
    uint8 ffData[8];

    /* PCI byte 0: FF(0x1) + high nibble of FF_DL */
    ffData[0] = CANTP_PCI_TYPE_FF | ((uint8)((len >> 8U) & 0x0FU));
    /* PCI byte 1: low byte of FF_DL */
    ffData[1] = (uint8)(len & 0xFFU);

    /* 复制前 6 bytes */
    MEMCPY(&ffData[2], data, CANTP_FF_DATA_LEN);

    pdu.pduId  = ch->txPduId;
    pdu.sdu    = ffData;
    pdu.length = 8U;

    (void)CanIf_Transmit(ch->txPduId, &pdu);

    /* 更新发送状态 */
    ch->txIndex = CANTP_FF_DATA_LEN;
}

/**
 * @brief 处理接收 CAN 帧 (分发到 SF/FF/CF/FC)
 */
static void CanTp_ProcessRxFrame(CanTp_ChannelType *ch, const CanIf_Pdu *Pdu)
{
    uint8 pci = Pdu->sdu[0];
    uint8 pciType = pci & CANTP_PCI_MASK;

    switch (pciType) {
    case CANTP_PCI_TYPE_SF:
        CanTp_ProcessRxSF(ch, Pdu->sdu, Pdu->length);
        break;

    case CANTP_PCI_TYPE_FF:
        CanTp_ProcessRxFF(ch, Pdu->sdu, Pdu->length);
        break;

    case CANTP_PCI_TYPE_CF:
        CanTp_ProcessRxCF(ch, Pdu->sdu, Pdu->length);
        break;

    case CANTP_PCI_TYPE_FC:
        CanTp_ProcessRxFC(ch, Pdu->sdu, Pdu->length);
        break;

    default:
        break;
    }
}

/**
 * @brief 处理单帧 (SF): 直接交付 N-SDU
 */
static void CanTp_ProcessRxSF(CanTp_ChannelType *ch, const uint8 *sdu, uint8 len)
{
    uint8 sfLen;

    /* SF_DL = PCI byte[0] & 0x0F */
    sfLen = sdu[0] & 0x0FU;

    if (sfLen > CANTP_SF_MAX_DATA_LEN) {
        return;  /* 无效长度 */
    }

    /* 检查 CAN DLC >= SF_DL + 1 */
    if (len < (uint8)(sfLen + 1U)) {
        return;
    }

    /* 复制数据到接收缓冲区 */
    MEMCPY(ch->rxBuffer, &sdu[1], sfLen);
    ch->rxLength   = sfLen;
    ch->rxComplete = TRUE;
    ch->state      = CANTP_CH_READY;
}

/**
 * @brief 处理首帧 (FF): 准备接收多帧数据
 */
static void CanTp_ProcessRxFF(CanTp_ChannelType *ch, const uint8 *sdu, uint8 len)
{
    uint16 ffDl;

    if (len < 8U) {
        return;  /* FF 必须 8 bytes */
    }

    /* FF_DL = ((PCI[0] & 0x0F) << 8) | PCI[1] */
    ffDl = (uint16)(((uint16)(sdu[0] & 0x0FU) << 8U) | (uint16)sdu[1]);

    if (ffDl > CANTP_MAX_NSDU_LENGTH) {
        /* 长度溢出 → 发送 FC Overflow */
        CanTp_SendFlowControl(ch, CANTP_FC_OVERFLOW, CANTP_DEFAULT_BS, CANTP_DEFAULT_STMIN_MS);
        CanTp_ResetChannel(ch);
        return;
    }

    /* 初始化接收缓冲区 */
    ch->rxLength     = ffDl;
    ch->rxIndex      = 0U;
    ch->rxExpectedSN = 1U;

    /* 复制 FF 的 6 bytes 数据 */
    MEMCPY(ch->rxBuffer, &sdu[2], CANTP_FF_DATA_LEN);
    ch->rxIndex = CANTP_FF_DATA_LEN;

    /* 发送 Flow Control: CTS, BS=0, STmin=10ms */
    CanTp_SendFlowControl(ch, CANTP_FC_CTS, CANTP_DEFAULT_BS, CANTP_DEFAULT_STMIN_MS);

    /* 进入等待 CF 状态 */
    ch->state   = CANTP_CH_WAIT_CF;
    ch->rxTimer = CanTp_GetTimestamp();
}

/**
 * @brief 处理连续帧 (CF): 追加数据
 */
static void CanTp_ProcessRxCF(CanTp_ChannelType *ch, const uint8 *sdu, uint8 len)
{
    uint8  sn;
    uint8  cfLen;
    uint16 remaining;

    if (ch->state != CANTP_CH_WAIT_CF) {
        return;
    }

    /* CF 序列号 = PCI[0] & 0x0F */
    sn = sdu[0] & 0x0FU;

    if (sn != ch->rxExpectedSN) {
        /* 序列号不匹配 → 丢弃 */
        return;
    }

    remaining = ch->rxLength - ch->rxIndex;
    cfLen = (remaining > CANTP_CF_DATA_LEN) ? CANTP_CF_DATA_LEN : (uint8)remaining;

    if (len < (uint8)(cfLen + 1U)) {
        return;  /* 帧长度不足 */
    }

    /* 追加数据到接收缓冲区 */
    MEMCPY(&ch->rxBuffer[ch->rxIndex], &sdu[1], cfLen);
    ch->rxIndex += cfLen;

    /* 更新期望序列号 */
    ch->rxExpectedSN++;
    if (ch->rxExpectedSN >= 16U) {
        ch->rxExpectedSN = 0U;
    }

    /* 复位 N_Cr 定时器 */
    ch->rxTimer = CanTp_GetTimestamp();

    /* 检查是否接收完毕 */
    if (ch->rxIndex >= ch->rxLength) {
        ch->rxComplete = TRUE;
        ch->state      = CANTP_CH_READY;
    }
}

/**
 * @brief 处理流控帧 (FC): 发送方接收 FC 响应
 */
static void CanTp_ProcessRxFC(CanTp_ChannelType *ch, const uint8 *sdu, uint8 len)
{
    uint8 fs;
    uint8 bs;

    if (ch->state != CANTP_CH_WAIT_FC) {
        return;
    }
    if (len < 3U) {
        return;
    }

    fs = sdu[0] & 0x0FU;
    bs = sdu[1];
    /* STmin = sdu[2] (暂未使用) */

    switch (fs) {
    case CANTP_FC_CTS:
        /* 继续发送 */
        if (bs == 0U) {
            /* BS=0: 不限制块大小，直接发送所有 CF */
            ch->txBlockRemaining = 0xFFU;  /* 足够大 */
        } else {
            ch->txBlockRemaining = bs;
        }
        ch->state   = CANTP_CH_SENDING_CF;
        /* 立即发送第一帧 CF (在 MainFunction 中处理) */
        break;

    case CANTP_FC_WAIT:
        /* 等待: 保持 WAIT_FC 状态，重新计时 */
        ch->txTimer = CanTp_GetTimestamp();
        break;

    case CANTP_FC_OVERFLOW:
        /* 溢出: 取消发送 */
        CanTp_ResetChannel(ch);
        if (s_txCallback != NULL_PTR) {
            s_txCallback(ch->channelId, STD_NOT_OK);
        }
        break;

    default:
        break;
    }
}

/**
 * @brief 获取系统时间戳 (ms)
 *
 * 使用 HAL_GetTick() 获取自启动以来的毫秒数。
 */
static uint32 CanTp_GetTimestamp(void)
{
    /* HAL_GetTick() 由 SysTick 中断管理 */
    return HAL_GetTick();
}