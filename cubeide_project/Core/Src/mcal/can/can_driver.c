/**
 * @file can_driver.c
 * @brief AUTOSAR CAN Driver 实现
 * @version 2.0.1
 */

#include "can_driver.h"
#include "common.h"
#include "stm32f4xx_hal.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

#define CAN_GET_HANDLE(ch)          (((ch) == CAN_CHANNEL_1) ? (&hcan1) : (&hcan2))
#define CAN_IS_VALID_CH(ch)         ((ch) < CAN_NUM_OF_CHANNELS)

typedef struct
{
    Can_Frame  buffer[CAN_RX_FIFO_DEPTH];
    uint16 writeIdx;
    uint16 readIdx;
    uint16 count;
} Can_RxFifoType;

typedef struct
{
    Can_Frame  buffer[CAN_TX_BUFFER_DEPTH];
    uint16 writeIdx;
    uint16 readIdx;
    uint16 count;
} Can_TxBufferType;

typedef struct
{
    Can_StateType        state;
    uint8                txErrCnt;
    uint8                rxErrCnt;
    boolean              initialized;
    Can_RxFifoType       rxFifo;
    Can_TxBufferType     txBuf;
    uint32               busoffCount;
} Can_ChannelDataType;

static Can_ChannelDataType s_chanData[CAN_NUM_OF_CHANNELS];
static Can_RxNotification  s_rxNotification = NULL_PTR;

static void Can_InitChannel(uint8 Channel);
static void Can_ConfigFilter(uint8 Channel);

/* ==================== 公开 API ==================== */

Std_ReturnType Can_Init(Can_Config *Config)
{
    if (Config == NULL_PTR) return STD_NOT_OK;
    if (Config->channel >= CAN_NUM_OF_CHANNELS) return STD_NOT_OK;

    uint8 ch = Config->channel;

    if (s_chanData[ch].initialized) {
        (void)HAL_CAN_Stop(CAN_GET_HANDLE(ch));
        s_chanData[ch].state = CAN_STATE_STOPPED;
    }

    s_chanData[ch].state       = CAN_STATE_STOPPED;
    s_chanData[ch].txErrCnt    = 0U;
    s_chanData[ch].rxErrCnt    = 0U;
    s_chanData[ch].initialized = FALSE;
    s_chanData[ch].busoffCount = 0U;
    s_chanData[ch].rxFifo.writeIdx = 0U;
    s_chanData[ch].rxFifo.readIdx  = 0U;
    s_chanData[ch].rxFifo.count    = 0U;
    s_chanData[ch].txBuf.writeIdx  = 0U;
    s_chanData[ch].txBuf.readIdx   = 0U;
    s_chanData[ch].txBuf.count     = 0U;

    Can_ConfigFilter(ch);

    if (HAL_CAN_Start(CAN_GET_HANDLE(ch)) != HAL_OK) return STD_NOT_OK;
    if (HAL_CAN_ActivateNotification(CAN_GET_HANDLE(ch), CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) return STD_NOT_OK;
    (void)HAL_CAN_ActivateNotification(CAN_GET_HANDLE(ch), CAN_IT_TX_MAILBOX_EMPTY);

    s_chanData[ch].state       = CAN_STATE_STARTED;
    s_chanData[ch].initialized = TRUE;
    return STD_OK;
}

Std_ReturnType Can_Write(uint8 Channel, const Can_Frame *Frame)
{
    if (!CAN_IS_VALID_CH(Channel) || Frame == NULL_PTR || !s_chanData[Channel].initialized)
        return STD_NOT_OK;

    Can_TxBufferType *pTx = &s_chanData[Channel].txBuf;
    if (pTx->count >= CAN_TX_BUFFER_DEPTH) return STD_NOT_OK;

    MEMCPY(&pTx->buffer[pTx->writeIdx], Frame, sizeof(Can_Frame));
    pTx->writeIdx = (pTx->writeIdx + 1U) % CAN_TX_BUFFER_DEPTH;
    __disable_irq(); pTx->count++; __enable_irq();
    return STD_OK;
}

Std_ReturnType Can_Read(uint8 Channel, Can_Frame *Frame)
{
    if (!CAN_IS_VALID_CH(Channel) || Frame == NULL_PTR) return STD_NOT_OK;

    Can_RxFifoType *pRx = &s_chanData[Channel].rxFifo;
    if (pRx->count == 0U) return STD_NOT_OK;

    __disable_irq();
    MEMCPY(Frame, &pRx->buffer[pRx->readIdx], sizeof(Can_Frame));
    pRx->readIdx = (pRx->readIdx + 1U) % CAN_RX_FIFO_DEPTH;
    pRx->count--;
    __enable_irq();
    return STD_OK;
}

Can_StateType Can_GetControllerState(uint8 Channel)
{
    if (!CAN_IS_VALID_CH(Channel)) return CAN_STATE_UNINIT;
    return s_chanData[Channel].state;
}

void Can_GetErrorCounters(uint8 Channel, uint8 *TxErrCntPtr, uint8 *RxErrCntPtr)
{
    if (!CAN_IS_VALID_CH(Channel)) return;
    if (TxErrCntPtr) *TxErrCntPtr = s_chanData[Channel].txErrCnt;
    if (RxErrCntPtr) *RxErrCntPtr = s_chanData[Channel].rxErrCnt;
}

void Can_SetRxNotification(Can_RxNotification Callback)
{
    s_rxNotification = Callback;
}

/* ==================== 中断处理 ==================== */

void Can_RxISR(void *hcan)
{
    CAN_HandleTypeDef *pHal = (CAN_HandleTypeDef *)hcan;
    uint8 ch = (pHal->Instance == CAN1) ? 0U : 1U;

    CAN_RxHeaderTypeDef rxHdr;
    uint8 rawData[8];
    if (HAL_CAN_GetRxMessage(pHal, CAN_RX_FIFO0, &rxHdr, rawData) != HAL_OK) return;

    Can_Frame frame;
    frame.id        = rxHdr.StdId;
    frame.dlc       = rxHdr.DLC;
    frame.idType    = (rxHdr.IDE == CAN_ID_STD) ? CAN_ID_STANDARD : CAN_ID_EXTENDED;
    frame.frameType = (rxHdr.RTR == CAN_RTR_DATA) ? CAN_FRAME_DATA : CAN_FRAME_REMOTE;
    MEMCPY(frame.sdu, rawData, (rxHdr.DLC > 8U) ? 8U : rxHdr.DLC);

    Can_RxFifoType *pRx = &s_chanData[ch].rxFifo;
    if (pRx->count < CAN_RX_FIFO_DEPTH) {
        MEMCPY(&pRx->buffer[pRx->writeIdx], &frame, sizeof(Can_Frame));
        pRx->writeIdx = (pRx->writeIdx + 1U) % CAN_RX_FIFO_DEPTH;
        pRx->count++;
    }

    if (s_rxNotification) s_rxNotification(ch, &frame);
}

void Can_TxISR(void *hcan) { (void)hcan; }

void Can_ErrorISR(void *hcan)
{
    CAN_HandleTypeDef *pHal = (CAN_HandleTypeDef *)hcan;
    uint8 ch = (pHal->Instance == CAN1) ? 0U : 1U;

    /* 直接从 ESR 寄存器读取错误计数 */
    uint32 esr = pHal->Instance->ESR;
    s_chanData[ch].txErrCnt = (uint8)((esr >> 16U) & 0xFFU);
    s_chanData[ch].rxErrCnt = (uint8)((esr >> 24U) & 0xFFU);

    if (esr & (1U << 2U)) {  /* BOFF */
        s_chanData[ch].state = CAN_STATE_BUSOFF;
        s_chanData[ch].busoffCount++;
    }
}

/* ==================== 主函数轮询 ==================== */

void Can_MainFunction_Write(void)
{
    for (uint8 ch = 0U; ch < CAN_NUM_OF_CHANNELS; ch++) {
        if (!s_chanData[ch].initialized || s_chanData[ch].state != CAN_STATE_STARTED)
            continue;

        Can_TxBufferType *pTx = &s_chanData[ch].txBuf;
        if (pTx->count == 0U) continue;

        Can_Frame frame;
        __disable_irq();
        MEMCPY(&frame, &pTx->buffer[pTx->readIdx], sizeof(Can_Frame));
        pTx->readIdx = (pTx->readIdx + 1U) % CAN_TX_BUFFER_DEPTH;
        pTx->count--;
        __enable_irq();

        CAN_HandleTypeDef *pHal = CAN_GET_HANDLE(ch);
        CAN_TxHeaderTypeDef txHdr;
        uint8 txData[8];
        uint32 txMailbox;

        if (frame.idType == CAN_ID_STANDARD) { txHdr.StdId = frame.id; txHdr.IDE = CAN_ID_STD; }
        else                                 { txHdr.ExtId = frame.id; txHdr.IDE = CAN_ID_EXT; }
        txHdr.RTR = (frame.frameType == CAN_FRAME_DATA) ? CAN_RTR_DATA : CAN_RTR_REMOTE;
        txHdr.DLC = (frame.dlc > 8U) ? 8U : frame.dlc;
        txHdr.TransmitGlobalTime = DISABLE;
        MEMCPY(txData, frame.sdu, txHdr.DLC);

        if (HAL_CAN_AddTxMessage(pHal, &txHdr, txData, &txMailbox) != HAL_OK)
            s_chanData[ch].txErrCnt++;
    }
}

void Can_MainFunction_BusOff(void)
{
    for (uint8 ch = 0U; ch < CAN_NUM_OF_CHANNELS; ch++) {
        if (s_chanData[ch].state != CAN_STATE_BUSOFF) continue;

        CAN_HandleTypeDef *pHal = CAN_GET_HANDLE(ch);
        HAL_CAN_ResetError(pHal);

        if (HAL_CAN_Start(pHal) == HAL_OK) {
            s_chanData[ch].state = CAN_STATE_STARTED;
            s_chanData[ch].txErrCnt = 0U;
            s_chanData[ch].rxErrCnt = 0U;
            (void)HAL_CAN_ActivateNotification(pHal, CAN_IT_RX_FIFO0_MSG_PENDING);
        }
    }
}

/* ==================== 私有 ==================== */

static void Can_ConfigFilter(uint8 Channel)
{
    CAN_HandleTypeDef *pHal = CAN_GET_HANDLE(Channel);
    CAN_FilterTypeDef filter;
    filter.FilterBank           = (Channel == 0U) ? 0U : 14U;
    filter.FilterMode           = CAN_FILTERMODE_IDMASK;
    filter.FilterScale          = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh         = 0x0000U;
    filter.FilterIdLow          = 0x0000U;
    filter.FilterMaskIdHigh     = 0x0000U;
    filter.FilterMaskIdLow      = 0x0000U;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterActivation     = ENABLE;
    filter.SlaveStartFilterBank = 14U;
    (void)HAL_CAN_ConfigFilter(pHal, &filter);
}