/**
 * @file can_driver.c
 * @brief AUTOSAR CAN Driver 实现
 * @version 2.0.0
 */

#include "mcal/can/can_driver.h"
#include "common.h"
#include "stm32f4xx_hal.h"

/* ==================== 外部引用：CubeMX 全局句柄 ==================== */
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

/* ==================== 私有宏 ==================== */

#define CAN_GET_HANDLE(ch)          (((ch) == CAN_CHANNEL_1) ? (&hcan1) : (&hcan2))
#define CAN_IS_VALID_CH(ch)         ((ch) < CAN_NUM_OF_CHANNELS)
#define CAN_TX_MAILBOX_RETRY       3U

/* ==================== 私有数据结构 ==================== */

typedef struct
{
    Can_Frame  buffer[CAN_RX_FIFO_DEPTH];
    VAR(uint16, CAN_APPL_DATA) writeIdx;
    VAR(uint16, CAN_APPL_DATA) readIdx;
    VAR(uint16, CAN_APPL_DATA) count;
} Can_RxFifoType;

typedef struct
{
    Can_Frame  buffer[CAN_TX_BUFFER_DEPTH];
    VAR(uint16, CAN_APPL_DATA) writeIdx;
    VAR(uint16, CAN_APPL_DATA) readIdx;
    VAR(uint16, CAN_APPL_DATA) count;
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

/* ==================== 私有全局变量 ==================== */

static VAR(Can_ChannelDataType, CAN_APPL_DATA) s_chanData[CAN_NUM_OF_CHANNELS];
static VAR(Can_RxNotification, CAN_APPL_DATA)  s_rxNotification = NULL_PTR;

/* ==================== 私有函数声明 ==================== */

static FUNC(void, CAN_CODE) Can_InitChannel(uint8 Channel);
static FUNC(Std_ReturnType, CAN_CODE) Can_WriteHwMailbox(uint8 Channel, const Can_Frame *Frame);
static FUNC(void, CAN_CODE) Can_ConfigFilter(uint8 Channel);

/* ==================== 公开函数实现 ==================== */

FUNC(Std_ReturnType, CAN_CODE)
Can_Init(
    CONSTP2VAR(Can_Config, AUTOMATIC, CAN_APPL_DATA) Config
)
{
    if (Config == NULL_PTR) {
        return STD_NOT_OK;
    }
    if (!CAN_IS_VALID_CH(Config->channel)) {
        return STD_NOT_OK;
    }

    uint8 ch = Config->channel;

    if (s_chanData[ch].initialized) {
        (void)HAL_CAN_Stop(CAN_GET_HANDLE(ch));
        s_chanData[ch].state = CAN_STATE_STOPPED;
    }

    Can_InitChannel(ch);
    Can_ConfigFilter(ch);

    if (HAL_CAN_Start(CAN_GET_HANDLE(ch)) != HAL_OK) {
        return STD_NOT_OK;
    }
    if (HAL_CAN_ActivateNotification(CAN_GET_HANDLE(ch),
                                     CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        return STD_NOT_OK;
    }
    if (HAL_CAN_ActivateNotification(CAN_GET_HANDLE(ch),
                                     CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) {
        /* 非致命 */
    }

    s_chanData[ch].state       = CAN_STATE_STARTED;
    s_chanData[ch].initialized = TRUE;

    return STD_OK;
}

FUNC(Std_ReturnType, CAN_CODE)
Can_Write(
    uint8                                    Channel,
    CONSTP2CONST(Can_Frame, AUTOMATIC, CAN_APPL_CONST) Frame
)
{
    if (!CAN_IS_VALID_CH(Channel)) { return STD_NOT_OK; }
    if (Frame == NULL_PTR)         { return STD_NOT_OK; }
    if (!s_chanData[Channel].initialized) { return STD_NOT_OK; }

    Can_TxBufferType *pTx = &s_chanData[Channel].txBuf;

    if (pTx->count >= CAN_TX_BUFFER_DEPTH) {
        return STD_NOT_OK;
    }

    MEMCPY(&pTx->buffer[pTx->writeIdx], Frame, sizeof(Can_Frame));

    pTx->writeIdx++;
    if (pTx->writeIdx >= CAN_TX_BUFFER_DEPTH) {
        pTx->writeIdx = 0U;
    }

    __disable_irq();
    pTx->count++;
    __enable_irq();

    return STD_OK;
}

FUNC(Std_ReturnType, CAN_CODE)
Can_Read(
    uint8                    Channel,
    P2VAR(Can_Frame, AUTOMATIC, CAN_APPL_DATA) Frame
)
{
    if (!CAN_IS_VALID_CH(Channel)) { return STD_NOT_OK; }
    if (Frame == NULL_PTR)         { return STD_NOT_OK; }

    Can_RxFifoType *pRx = &s_chanData[Channel].rxFifo;

    if (pRx->count == 0U) { return STD_NOT_OK; }

    __disable_irq();
    MEMCPY(Frame, &pRx->buffer[pRx->readIdx], sizeof(Can_Frame));
    pRx->readIdx++;
    if (pRx->readIdx >= CAN_RX_FIFO_DEPTH) {
        pRx->readIdx = 0U;
    }
    pRx->count--;
    __enable_irq();

    return STD_OK;
}

FUNC(Can_StateType, CAN_CODE)
Can_GetControllerState(uint8 Channel)
{
    if (!CAN_IS_VALID_CH(Channel)) { return CAN_STATE_UNINIT; }
    return s_chanData[Channel].state;
}

FUNC(void, CAN_CODE)
Can_GetErrorCounters(
    uint8           Channel,
    P2VAR(uint8, AUTOMATIC, CAN_APPL_DATA) TxErrCntPtr,
    P2VAR(uint8, AUTOMATIC, CAN_APPL_DATA) RxErrCntPtr
)
{
    if (!CAN_IS_VALID_CH(Channel)) { return; }
    if (TxErrCntPtr != NULL_PTR) { *TxErrCntPtr = s_chanData[Channel].txErrCnt; }
    if (RxErrCntPtr != NULL_PTR) { *RxErrCntPtr = s_chanData[Channel].rxErrCnt; }
}

FUNC(void, CAN_CODE)
Can_SetRxNotification(Can_RxNotification Callback)
{
    s_rxNotification = Callback;
}

/* ==================== 中断处理 ==================== */

FUNC(void, CAN_CODE)
Can_RxISR(P2VAR(void, AUTOMATIC, CAN_APPL_DATA) hcan)
{
    CAN_HandleTypeDef *pHal = (CAN_HandleTypeDef *)hcan;
    uint8 ch;

    if (pHal->Instance == CAN1) {
        ch = CAN_CHANNEL_1;
    } else if (pHal->Instance == CAN2) {
        ch = CAN_CHANNEL_2;
    } else {
        return;
    }

    CAN_RxHeaderTypeDef rxHdr;
    uint8               rawData[8];
    Can_RxFifoType     *pRx = &s_chanData[ch].rxFifo;

    if (HAL_CAN_GetRxMessage(pHal, CAN_RX_FIFO0, &rxHdr, rawData) != HAL_OK) {
        return;
    }

    Can_Frame frame;
    frame.id        = rxHdr.StdId;
    frame.dlc       = rxHdr.DLC;
    frame.idType    = (rxHdr.IDE == CAN_ID_STD) ? CAN_ID_STANDARD : CAN_ID_EXTENDED;
    frame.frameType = (rxHdr.RTR == CAN_RTR_DATA) ? CAN_FRAME_DATA : CAN_FRAME_REMOTE;
    MEMCPY(frame.sdu, rawData, (rxHdr.DLC > CAN_MAX_DLC) ? CAN_MAX_DLC : rxHdr.DLC);

    if (pRx->count < CAN_RX_FIFO_DEPTH) {
        MEMCPY(&pRx->buffer[pRx->writeIdx], &frame, sizeof(Can_Frame));
        pRx->writeIdx++;
        if (pRx->writeIdx >= CAN_RX_FIFO_DEPTH) {
            pRx->writeIdx = 0U;
        }
        pRx->count++;
    }

    if (s_rxNotification != NULL_PTR) {
        s_rxNotification(ch, &frame);
    }
}

FUNC(void, CAN_CODE)
Can_TxISR(P2VAR(void, AUTOMATIC, CAN_APPL_DATA) hcan)
{
    (void)hcan;
}

FUNC(void, CAN_CODE)
Can_ErrorISR(P2VAR(void, AUTOMATIC, CAN_APPL_DATA) hcan)
{
    CAN_HandleTypeDef *pHal = (CAN_HandleTypeDef *)hcan;
    uint8 ch;

    if (pHal->Instance == CAN1) {
        ch = CAN_CHANNEL_1;
    } else if (pHal->Instance == CAN2) {
        ch = CAN_CHANNEL_2;
    } else {
        return;
    }

    /* 通过 HAL 寄存器读取错误计数 */
    CAN_TypeDef *CANx = pHal->Instance;
    uint32 esr = CANx->ESR;
    s_chanData[ch].txErrCnt = (uint8)((esr >> 16U) & 0xFFU);   /* TEC */
    s_chanData[ch].rxErrCnt = (uint8)((esr >> 24U) & 0xFFU);   /* REC */

    if (esr & (1U << 2U)) {  /* BOFF bit in ESR */
        s_chanData[ch].state = CAN_STATE_BUSOFF;
        s_chanData[ch].busoffCount++;
    }
}

/* ==================== 主函数轮询 ==================== */

FUNC(void, CAN_CODE)
Can_MainFunction_Write(void)
{
    uint8 ch;

    for (ch = 0U; ch < CAN_NUM_OF_CHANNELS; ch++) {
        if (!s_chanData[ch].initialized)          { continue; }
        if (s_chanData[ch].state != CAN_STATE_STARTED) { continue; }

        Can_TxBufferType *pTx = &s_chanData[ch].txBuf;

        if (pTx->count == 0U) { continue; }

        Can_Frame frame;
        __disable_irq();
        MEMCPY(&frame, &pTx->buffer[pTx->readIdx], sizeof(Can_Frame));
        pTx->readIdx++;
        if (pTx->readIdx >= CAN_TX_BUFFER_DEPTH) {
            pTx->readIdx = 0U;
        }
        pTx->count--;
        __enable_irq();

        (void)Can_WriteHwMailbox(ch, &frame);
    }
}

FUNC(void, CAN_CODE)
Can_MainFunction_BusOff(void)
{
    uint8 ch;

    for (ch = 0U; ch < CAN_NUM_OF_CHANNELS; ch++) {
        if (s_chanData[ch].state != CAN_STATE_BUSOFF) { continue; }

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

/* ==================== 私有函数实现 ==================== */

static FUNC(void, CAN_CODE)
Can_InitChannel(uint8 Channel)
{
    Can_ChannelDataType *pChan = &s_chanData[Channel];

    pChan->state       = CAN_STATE_STOPPED;
    pChan->txErrCnt    = 0U;
    pChan->rxErrCnt    = 0U;
    pChan->initialized = FALSE;
    pChan->busoffCount = 0U;

    pChan->rxFifo.writeIdx = 0U;
    pChan->rxFifo.readIdx  = 0U;
    pChan->rxFifo.count    = 0U;

    pChan->txBuf.writeIdx = 0U;
    pChan->txBuf.readIdx  = 0U;
    pChan->txBuf.count    = 0U;
}

static FUNC(Std_ReturnType, CAN_CODE)
Can_WriteHwMailbox(uint8 Channel, const Can_Frame *Frame)
{
    CAN_HandleTypeDef *pHal = CAN_GET_HANDLE(Channel);

    CAN_TxHeaderTypeDef txHdr;
    uint8               txData[8];
    uint32              txMailbox;

    if (Frame->idType == CAN_ID_STANDARD) {
        txHdr.StdId = Frame->id;
        txHdr.IDE   = CAN_ID_STD;
    } else {
        txHdr.ExtId = Frame->id;
        txHdr.IDE   = CAN_ID_EXT;
    }

    txHdr.RTR = (Frame->frameType == CAN_FRAME_DATA) ? CAN_RTR_DATA : CAN_RTR_REMOTE;
    txHdr.DLC = (Frame->dlc > CAN_MAX_DLC) ? CAN_MAX_DLC : Frame->dlc;
    txHdr.TransmitGlobalTime = DISABLE;

    MEMCPY(txData, Frame->sdu, txHdr.DLC);

    if (HAL_CAN_AddTxMessage(pHal, &txHdr, txData, &txMailbox) != HAL_OK) {
        s_chanData[Channel].txErrCnt++;
        return STD_NOT_OK;
    }

    return STD_OK;
}

static FUNC(void, CAN_CODE)
Can_ConfigFilter(uint8 Channel)
{
    CAN_HandleTypeDef *pHal = CAN_GET_HANDLE(Channel);

    CAN_FilterTypeDef filter;
    filter.FilterBank           = (Channel == CAN_CHANNEL_1) ? 0U : 14U;
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