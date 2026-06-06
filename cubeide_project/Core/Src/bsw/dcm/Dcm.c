/**
 * @file Dcm.c
 * @brief AUTOSAR Diagnostic Communication Manager (DCM) Core Module
 * @version 1.0.0
 *
 * 核心诊断通信管理器，负责：
 *   - 接收来自 PduR 的 UDS 请求
 *   - 服务分发 (dispatch) 到各服务处理函数
 *   - 诊断会话管理 (Service 0x10)
 *   - 安全访问级别管理
 *   - TesterPresent 处理 (Service 0x3E)
 *   - 通信控制管理 (Service 0x28)
 *   - 定时参数管理 (P2Server, S3Server)
 *   - 否定响应 (Negative Response) 构建
 *
 * 数据流:
 *   CAN Bus → Can Driver → CanIf → CANtp → PduR → Dcm (本模块)
 *   Dcm → PduR → CANtp → CanIf → Can Driver → CAN Bus
 */

/*******************************************************************************
 * INCLUDES
 *******************************************************************************/
#include "Std_Types.h"
#include "compiler.h"
#include "types.h"
#include "common.h"
#include "pdur.h"
#include <bsw/dcm/Dcm.h>
#include <bsw/dcm/Dcm_Uds_Config.h>
#include <bsw/dcm/dcm_service.h>

/*******************************************************************************
 * DEFINES
 *******************************************************************************/
#define DCM_VERSION_INFO_API                    0x01U
#define DCM_MAJOR_VERSION                       1U
#define DCM_MINOR_VERSION                       0U
#define DCM_PATCH_VERSION                       0U

/* UDS Response SID offset */
#define DCM_RESPONSE_SID_OFFSET                 0x40U
#define DCM_NEGATIVE_RESPONSE_SID               0x7FU

/* Negative Response Code (NRC) 标准 */
#define DCM_NRC_SERVICE_NOT_SUPPORTED           0x11U
#define DCM_NRC_SUBFUNCTION_NOT_SUPPORTED       0x12U
#define DCM_NRC_INCORRECT_MESSAGE_LENGTH        0x13U
#define DCM_NRC_CONDITIONS_NOT_CORRECT          0x22U
#define DCM_NRC_REQUEST_SEQUENCE_ERROR          0x24U
#define DCM_NRC_REQUEST_OUT_OF_RANGE            0x31U
#define DCM_NRC_SECURITY_ACCESS_DENIED          0x33U
#define DCM_NRC_INVALID_KEY                     0x35U
#define DCM_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS     0x36U
#define DCM_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED 0x37U
#define DCM_NRC_UPLOAD_DOWNLOAD_NOT_ACCEPTED    0x70U
#define DCM_NRC_TRANSFER_DATA_SUSPENDED         0x71U
#define DCM_NRC_GENERAL_PROGRAMMING_FAILURE     0x72U
#define DCM_NRC_WRONG_BLOCK_SEQUENCE_COUNTER    0x73U
#define DCM_NRC_RESPONSE_PENDING                0x78U
#define DCM_NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION 0x7EU
#define DCM_NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION     0x7FU

/* 定时参数 (来自需求文档) */
#define DCM_P2_SERVER_MAX_MS                    50U
#define DCM_P2_STAR_SERVER_MAX_MS               5000U
#define DCM_S3_SERVER_TIMEOUT_MS                5000U

/* 服务使能开关 */
#define DCM_SERVICE_0x10_ENABLED                1U
#define DCM_SERVICE_0x11_ENABLED                1U
#define DCM_SERVICE_0x28_ENABLED                1U
#define DCM_SERVICE_0x3E_ENABLED                1U
#define DCM_SERVICE_0x85_ENABLED                1U
#define DCM_SERVICE_0x22_ENABLED                1U
#define DCM_SERVICE_0x2E_ENABLED                1U
#define DCM_SERVICE_0x27_ENABLED                1U
#define DCM_SERVICE_0x31_ENABLED                1U
#define DCM_SERVICE_0x19_ENABLED                0U

/*******************************************************************************
 * TYPE DEFINITIONS
 *******************************************************************************/

typedef enum
{
    DCM_STATE_UNINIT = 0U,
    DCM_STATE_IDLE,
    DCM_STATE_PROCESSING,
    DCM_STATE_PENDING
} Dcm_StateType;

// typedef enum
// {
//     DCM_SESSION_DEFAULT     = 0x01U,
//     DCM_SESSION_PROGRAMMING = 0x02U,
//     DCM_SESSION_EXTENDED    = 0x03U
// } Dcm_SessionType;

typedef enum
{
    DCM_COMM_RX_TX_ENABLED      = 0x00U,
    DCM_COMM_RX_TX_DISABLED     = 0x01U,
    DCM_COMM_RX_ENABLED_TX_DIS  = 0x02U
} Dcm_CommStateType;

/*******************************************************************************
 * LOCAL VARIABLES
 *******************************************************************************/

static boolean Dcm_Initialized = FALSE;
static Dcm_StateType Dcm_State = DCM_STATE_UNINIT;
static uint8 Dcm_CurrentSession = DCM_SESSION_DEFAULT;
static uint8 Dcm_LastSession = DCM_SESSION_DEFAULT;
static boolean Dcm_SecurityLevel1Unlocked = FALSE;
static boolean Dcm_SecurityLevel2Unlocked = FALSE;
static boolean Dcm_DTCSettingEnabled = TRUE;
static Dcm_CommStateType Dcm_CommState = DCM_COMM_RX_TX_ENABLED;
static uint32 Dcm_S3Timer = 0U;
static uint32 Dcm_P2StarTimer = 0U;
static uint8 Dcm_RequestBuffer[DCM_REQUEST_BUFFER_SIZE];
static uint16 Dcm_RequestLength = 0U;
static uint8 Dcm_ResponseBuffer[DCM_RESPONSE_BUFFER_SIZE];
static uint16 Dcm_ResponseLength = 0U;
static boolean Dcm_NeedNegativeResponse = FALSE;
static boolean Dcm_NeedResponse = FALSE;
static boolean Dcm_SuppressPositiveResponse = FALSE;
static uint8 Dcm_CurrentRequestSID = 0U;
static uint8 Dcm_CurrentRequestSubFunction = 0U;

/*******************************************************************************
 * GLOBAL VARIABLES
 *******************************************************************************/

/**
 * @brief 全局否定响应码
 * 
 * Dcm_Uds_ECUReset.c, Dcm_Uds_CommunicationControl.c 等外部服务文件
 * 在检测到错误时设置此变量，由 dispatcher 在返回后读取。
 * 值为 0 表示无错误，非 0 表示需要发送否定响应。
 */
uint8 Dcm_Global_NegativeResponseCode = 0U;

/*******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *******************************************************************************/

static void Dcm_PduRRxCallback(PduR_PduIdType PduId,
                               CONSTP2CONST(uint8, AUTOMATIC, CAN_APPL_DATA) Data,
                               uint16 Length);

static void Dcm_PduRTxCallback(PduR_PduIdType PduId, Std_ReturnType Result);

static void Dcm_BuildNegativeResponse(uint8 RequestSID, uint8 NRC);

static uint8 Dcm_GetPositiveResponseSID(uint8 RequestSID);

static boolean Dcm_IsServiceSupportedInSession(uint8 SID);

static boolean Dcm_IsSubFunctionSupportedInSession(uint8 SID, uint8 SubFunction);

static void Dcm_ResetS3Timer(void);

static Std_ReturnType Dcm_SwitchSession(uint8 NewSession);

static boolean Dcm_ValidateRequestLength(uint8 SID, uint16 RequestLength);

/* 内嵌服务处理函数前置声明 */
static Std_ReturnType Dcm_Service_DiagnosticSessionControl_0x10(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) RequestData,
    uint16 RequestLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) ResponseData,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) ResponseLength);

static Std_ReturnType Dcm_Service_TesterPresent_0x3E(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) RequestData,
    uint16 RequestLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) ResponseData,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) ResponseLength);

/*******************************************************************************
 * PUBLIC FUNCTIONS
 *******************************************************************************/

FUNC(void, DCM_CODE) Dcm_Init(void)
{
    if (Dcm_Initialized) {
        return;
    }

    Dcm_State              = DCM_STATE_IDLE;
    Dcm_CurrentSession     = DCM_SESSION_DEFAULT;
    Dcm_LastSession        = DCM_SESSION_DEFAULT;
    Dcm_SecurityLevel1Unlocked = FALSE;
    Dcm_SecurityLevel2Unlocked = FALSE;
    Dcm_DTCSettingEnabled  = TRUE;
    Dcm_CommState          = DCM_COMM_RX_TX_ENABLED;
    Dcm_S3Timer            = 0U;
    Dcm_P2StarTimer        = 0U;
    Dcm_NeedResponse       = FALSE;
    Dcm_NeedNegativeResponse = FALSE;
    Dcm_SuppressPositiveResponse = FALSE;
    Dcm_Global_NegativeResponseCode = 0U;

    PduR_SetRxIndication(Dcm_PduRRxCallback);
    PduR_SetTxConfirmation(Dcm_PduRTxCallback);

    Dcm_Initialized = TRUE;
}

FUNC(void, DCM_CODE) Dcm_MainFunction(void)
{
    if (!Dcm_Initialized) {
        return;
    }

    if (Dcm_CurrentSession != DCM_SESSION_DEFAULT) {
        if (Dcm_S3Timer < DCM_S3_SERVER_TIMEOUT_MS) {
            Dcm_S3Timer += 10U;
        } else {
            Dcm_SwitchSession(DCM_SESSION_DEFAULT);
        }
    }
}

FUNC(uint8, DCM_CODE) Dcm_GetCurrentSession(void)
{
    return Dcm_CurrentSession;
}

FUNC(uint8, DCM_CODE) Dcm_GetSecurityLevel(void)
{
    if (Dcm_SecurityLevel2Unlocked) {
        return 2U;
    } else if (Dcm_SecurityLevel1Unlocked) {
        return 1U;
    } else {
        return 0U;
    }
}

FUNC(boolean, DCM_CODE) Dcm_IsSecurityLevelUnlocked(uint8 SecurityLevel)
{
    switch (SecurityLevel) {
    case 0U: return TRUE;
    case 1U: return Dcm_SecurityLevel1Unlocked;
    case 2U: return Dcm_SecurityLevel2Unlocked;
    default: return FALSE;
    }
}

FUNC(void, DCM_CODE) Dcm_SetSecurityLevel(uint8 SecurityLevel, boolean Unlocked)
{
    switch (SecurityLevel) {
    case 1U: Dcm_SecurityLevel1Unlocked = Unlocked; break;
    case 2U: Dcm_SecurityLevel2Unlocked = Unlocked; break;
    default: break;
    }
}

FUNC(boolean, DCM_CODE) Dcm_GetDTCSettingEnabled(void)
{
    return Dcm_DTCSettingEnabled;
}

FUNC(void, DCM_CODE) Dcm_SetDTCSettingEnabled(boolean Enabled)
{
    Dcm_DTCSettingEnabled = Enabled;
}

FUNC(uint8, DCM_CODE) Dcm_GetCommState(void)
{
    return (uint8)Dcm_CommState;
}

FUNC(void, DCM_CODE) Dcm_SetCommState(uint8 CommState)
{
    switch (CommState) {
    case 0x00U: Dcm_CommState = DCM_COMM_RX_TX_ENABLED;     break;
    case 0x01U: Dcm_CommState = DCM_COMM_RX_TX_DISABLED;    break;
    case 0x03U: Dcm_CommState = DCM_COMM_RX_ENABLED_TX_DIS; break;
    default: break;
    }
}

FUNC(void, DCM_CODE) Dcm_TesterPresentReset(void)
{
    Dcm_ResetS3Timer();
}

/*******************************************************************************
 * PRIVATE FUNCTIONS - PduR CALLBACKS
 *******************************************************************************/

static void Dcm_PduRRxCallback(PduR_PduIdType PduId,
                               CONSTP2CONST(uint8, AUTOMATIC, CAN_APPL_DATA) Data,
                               uint16 Length)
{
    uint8  sid;
    uint8  subFunction;
    Std_ReturnType retVal = E_OK;

    if (!Dcm_Initialized || (Data == NULL_PTR) || (Length == 0U)) {
        return;
    }

    if (PduId == PDUR_ID_UDS_FUNCTIONAL) {
        Dcm_SuppressPositiveResponse = TRUE;
    } else {
        Dcm_SuppressPositiveResponse = FALSE;
    }

    if (Length < 1U) {
        return;
    }

    sid = Data[0];
    Dcm_CurrentRequestSID = sid;

    Dcm_NeedResponse       = FALSE;
    Dcm_NeedNegativeResponse = FALSE;
    Dcm_ResponseLength     = 0U;
    Dcm_Global_NegativeResponseCode = 0U;  /* 复位全局 NRC */

    if ((sid & 0x80U) != 0U) {
        Dcm_SuppressPositiveResponse = TRUE;
        sid &= 0x7FU;
        Dcm_CurrentRequestSID = sid;
    }

    if (Length >= 2U) {
        subFunction = Data[1];
        Dcm_CurrentRequestSubFunction = subFunction;
    } else {
        subFunction = 0U;
        Dcm_CurrentRequestSubFunction = subFunction;
    }

    Dcm_ResetS3Timer();

    /* ========================================================================
     * Service Dispatcher
     * ======================================================================== */

    switch (sid) {

    /* 0x10: DiagnosticSessionControl */
    case 0x10U:
#if DCM_SERVICE_0x10_ENABLED
        Dcm_State = DCM_STATE_PROCESSING;
        retVal = Dcm_Service_DiagnosticSessionControl_0x10(
            Data, Length, Dcm_ResponseBuffer, &Dcm_ResponseLength);
        Dcm_State = DCM_STATE_IDLE;
        if (retVal != E_OK) {
            Dcm_NeedNegativeResponse = TRUE;
        } else {
            Dcm_NeedResponse = TRUE;
        }
#else
        Dcm_Global_NegativeResponseCode = DCM_NRC_SERVICE_NOT_SUPPORTED;
        Dcm_NeedNegativeResponse = TRUE;
#endif
        break;

    /* 0x11: ECUReset */
    case 0x11U:
#if DCM_SERVICE_0x11_ENABLED
        Dcm_State = DCM_STATE_PROCESSING;
        retVal = Dcm_Service_ECUReset_0x11(
            Data, Length, Dcm_ResponseBuffer, &Dcm_ResponseLength);
        Dcm_State = DCM_STATE_IDLE;
        if (retVal != E_OK) {
            Dcm_NeedNegativeResponse = TRUE;
        } else {
            Dcm_NeedResponse = TRUE;
        }
#else
        Dcm_Global_NegativeResponseCode = DCM_NRC_SERVICE_NOT_SUPPORTED;
        Dcm_NeedNegativeResponse = TRUE;
#endif
        break;

    /* 0x22: ReadDataByIdentifier */
    case 0x22U:
#if DCM_SERVICE_0x22_ENABLED
    {
        Dcm_MsgType requestWrapper;
        Dcm_MsgType responseWrapper;
        requestWrapper.SduLength = Length;
        for (uint16 i = 0U; i < Length && i < 256U; i++) {
            requestWrapper.Sdu[i] = Data[i];
        }
        responseWrapper.SduLength = 0U;
        Dcm_State = DCM_STATE_PROCESSING;
        retVal = Dcm_Service_ReadDataByIdentifier_0x22(
            &requestWrapper, &responseWrapper, &Dcm_ResponseLength, &Dcm_Global_NegativeResponseCode);
        Dcm_State = DCM_STATE_IDLE;
        if (retVal != E_OK) {
            Dcm_NeedNegativeResponse = TRUE;
        } else {
            for (uint16 i = 0U; i < Dcm_ResponseLength; i++) {
                Dcm_ResponseBuffer[i] = responseWrapper.Sdu[i];
            }
            Dcm_NeedResponse = TRUE;
        }
    }
#else
        Dcm_Global_NegativeResponseCode = DCM_NRC_SERVICE_NOT_SUPPORTED;
        Dcm_NeedNegativeResponse = TRUE;
#endif
        break;

    /* 0x27: SecurityAccess */
    case 0x27U:
#if DCM_SERVICE_0x27_ENABLED
    {
        Dcm_MsgType requestWrapper;
        Dcm_MsgType responseWrapper;
        requestWrapper.SduLength = Length;
        for (uint16 i = 0U; i < Length && i < 256U; i++) {
            requestWrapper.Sdu[i] = Data[i];
        }
        responseWrapper.SduLength = 0U;
        Dcm_State = DCM_STATE_PROCESSING;
        retVal = Dcm_Service_SecurityAccess_0x27(
            &requestWrapper, &responseWrapper, &Dcm_ResponseLength, &Dcm_Global_NegativeResponseCode);
        Dcm_State = DCM_STATE_IDLE;
        if (retVal != E_OK) {
            Dcm_NeedNegativeResponse = TRUE;
        } else {
            for (uint16 i = 0U; i < Dcm_ResponseLength; i++) {
                Dcm_ResponseBuffer[i] = responseWrapper.Sdu[i];
            }
            Dcm_NeedResponse = TRUE;
        }
    }
#else
        Dcm_Global_NegativeResponseCode = DCM_NRC_SERVICE_NOT_SUPPORTED;
        Dcm_NeedNegativeResponse = TRUE;
#endif
        break;

    /* 0x28: CommunicationControl */
    case 0x28U:
#if DCM_SERVICE_0x28_ENABLED
        Dcm_State = DCM_STATE_PROCESSING;
        retVal = Dcm_Service_CommunicationControl_0x28(
            Data, Length, Dcm_ResponseBuffer, &Dcm_ResponseLength);
        Dcm_State = DCM_STATE_IDLE;
        if (retVal != E_OK) {
            Dcm_NeedNegativeResponse = TRUE;
        } else {
            Dcm_NeedResponse = TRUE;
        }
#else
        Dcm_Global_NegativeResponseCode = DCM_NRC_SERVICE_NOT_SUPPORTED;
        Dcm_NeedNegativeResponse = TRUE;
#endif
        break;

    /* 0x2E: WriteDataByIdentifier */
    case 0x2EU:
#if DCM_SERVICE_0x2E_ENABLED
    {
        Dcm_MsgType requestWrapper;
        Dcm_MsgType responseWrapper;
        requestWrapper.SduLength = Length;
        for (uint16 i = 0U; i < Length && i < 256U; i++) {
            requestWrapper.Sdu[i] = Data[i];
        }
        responseWrapper.SduLength = 0U;
        Dcm_State = DCM_STATE_PROCESSING;
        retVal = Dcm_Service_WriteDataByIdentifier_0x2E(
            &requestWrapper, &responseWrapper, &Dcm_ResponseLength, &Dcm_Global_NegativeResponseCode);
        Dcm_State = DCM_STATE_IDLE;
        if (retVal != E_OK) {
            Dcm_NeedNegativeResponse = TRUE;
        } else {
            for (uint16 i = 0U; i < Dcm_ResponseLength; i++) {
                Dcm_ResponseBuffer[i] = responseWrapper.Sdu[i];
            }
            Dcm_NeedResponse = TRUE;
        }
    }
#else
        Dcm_Global_NegativeResponseCode = DCM_NRC_SERVICE_NOT_SUPPORTED;
        Dcm_NeedNegativeResponse = TRUE;
#endif
        break;

    /* 0x31: RoutineControl */
    case 0x31U:
#if DCM_SERVICE_0x31_ENABLED
    {
        Dcm_MsgType requestWrapper;
        Dcm_MsgType responseWrapper;
        requestWrapper.SduLength = Length;
        for (uint16 i = 0U; i < Length && i < 256U; i++) {
            requestWrapper.Sdu[i] = Data[i];
        }
        responseWrapper.SduLength = 0U;
        Dcm_State = DCM_STATE_PROCESSING;
        retVal = Dcm_Service_RoutineControl_0x31(
            &requestWrapper, &responseWrapper, &Dcm_ResponseLength, &Dcm_Global_NegativeResponseCode);
        Dcm_State = DCM_STATE_IDLE;
        if (retVal != E_OK) {
            Dcm_NeedNegativeResponse = TRUE;
        } else {
            for (uint16 i = 0U; i < Dcm_ResponseLength; i++) {
                Dcm_ResponseBuffer[i] = responseWrapper.Sdu[i];
            }
            Dcm_NeedResponse = TRUE;
        }
    }
#else
        Dcm_Global_NegativeResponseCode = DCM_NRC_SERVICE_NOT_SUPPORTED;
        Dcm_NeedNegativeResponse = TRUE;
#endif
        break;

    /* 0x3E: TesterPresent */
    case 0x3EU:
#if DCM_SERVICE_0x3E_ENABLED
        Dcm_State = DCM_STATE_PROCESSING;
        retVal = Dcm_Service_TesterPresent_0x3E(
            Data, Length, Dcm_ResponseBuffer, &Dcm_ResponseLength);
        Dcm_State = DCM_STATE_IDLE;
        if (retVal != E_OK) {
            Dcm_NeedNegativeResponse = TRUE;
        } else {
            Dcm_NeedResponse = TRUE;
        }
#else
        Dcm_Global_NegativeResponseCode = DCM_NRC_SERVICE_NOT_SUPPORTED;
        Dcm_NeedNegativeResponse = TRUE;
#endif
        break;

    /* 0x85: ControlDTCSetting */
    case 0x85U:
#if DCM_SERVICE_0x85_ENABLED
        Dcm_State = DCM_STATE_PROCESSING;
        retVal = Dcm_Service_ControlDTCSetting_0x85(
            Data, Length, Dcm_ResponseBuffer, &Dcm_ResponseLength);
        Dcm_State = DCM_STATE_IDLE;
        if (retVal != E_OK) {
            Dcm_NeedNegativeResponse = TRUE;
        } else {
            Dcm_NeedResponse = TRUE;
        }
#else
        Dcm_Global_NegativeResponseCode = DCM_NRC_SERVICE_NOT_SUPPORTED;
        Dcm_NeedNegativeResponse = TRUE;
#endif
        break;

    default:
        Dcm_Global_NegativeResponseCode = DCM_NRC_SERVICE_NOT_SUPPORTED;
        Dcm_NeedNegativeResponse = TRUE;
        break;
    }

    /* ========================================================================
     * 发送响应
     * ======================================================================== */

    if (Dcm_SuppressPositiveResponse) {
        return;
    }

    if (Dcm_NeedNegativeResponse) {
        /* 使用全局 NRC：可能由外部服务文件设置 */
        Dcm_BuildNegativeResponse(Dcm_CurrentRequestSID, Dcm_Global_NegativeResponseCode);
    } else if (Dcm_NeedResponse) {
        /* 肯定响应已在 Dcm_ResponseBuffer 中 */
    } else {
        return;
    }

    PduR_Transmit(PDUR_ID_UDS_PHYSICAL, Dcm_ResponseBuffer, Dcm_ResponseLength);
}

static void Dcm_PduRTxCallback(PduR_PduIdType PduId, Std_ReturnType Result)
{
    (void)PduId;
    (void)Result;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS - NEGATIVE RESPONSE
 *******************************************************************************/

static void Dcm_BuildNegativeResponse(uint8 RequestSID, uint8 NRC)
{
    Dcm_ResponseBuffer[0] = DCM_NEGATIVE_RESPONSE_SID;
    Dcm_ResponseBuffer[1] = RequestSID;
    Dcm_ResponseBuffer[2] = NRC;
    Dcm_ResponseLength    = 3U;
    Dcm_NeedResponse      = TRUE;
    Dcm_NeedNegativeResponse = FALSE;
}

static uint8 Dcm_GetPositiveResponseSID(uint8 RequestSID)
{
    return (RequestSID + DCM_RESPONSE_SID_OFFSET);
}

/*******************************************************************************
 * PRIVATE FUNCTIONS - SESSION MANAGEMENT
 *******************************************************************************/

static Std_ReturnType Dcm_SwitchSession(uint8 NewSession)
{
    switch (NewSession) {
    case DCM_SESSION_DEFAULT:
    case DCM_SESSION_PROGRAMMING:
    case DCM_SESSION_EXTENDED:
        break;
    default:
        return E_NOT_OK;
    }

    Dcm_LastSession = Dcm_CurrentSession;
    Dcm_CurrentSession = NewSession;
    Dcm_SecurityLevel1Unlocked = FALSE;
    Dcm_SecurityLevel2Unlocked = FALSE;
    Dcm_CommState = DCM_COMM_RX_TX_ENABLED;
    Dcm_ResetS3Timer();
    return E_OK;
}

static void Dcm_ResetS3Timer(void)
{
    Dcm_S3Timer = 0U;
}

/*******************************************************************************
 * SERVICE 0x10: DiagnosticSessionControl
 *******************************************************************************/

static Std_ReturnType Dcm_Service_DiagnosticSessionControl_0x10(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) RequestData,
    uint16 RequestLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) ResponseData,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) ResponseLength)
{
    uint8 subFunction;
    uint8 newSession;
    Std_ReturnType retVal;

    if ((RequestData == NULL_PTR) || (ResponseData == NULL_PTR) ||
        (ResponseLength == NULL_PTR)) {
        return E_NOT_OK;
    }

    if (RequestLength < 2U) {
        Dcm_Global_NegativeResponseCode = DCM_NRC_INCORRECT_MESSAGE_LENGTH;
        return E_NOT_OK;
    }

    subFunction = RequestData[1];

    switch (subFunction) {
    case 0x01U: newSession = DCM_SESSION_DEFAULT;     break;
    case 0x02U: newSession = DCM_SESSION_PROGRAMMING; break;
    case 0x03U: newSession = DCM_SESSION_EXTENDED;    break;
    default:
        Dcm_Global_NegativeResponseCode = DCM_NRC_SUBFUNCTION_NOT_SUPPORTED;
        return E_NOT_OK;
    }

    retVal = Dcm_SwitchSession(newSession);
    if (retVal != E_OK) {
        Dcm_Global_NegativeResponseCode = DCM_NRC_CONDITIONS_NOT_CORRECT;
        return E_NOT_OK;
    }

    ResponseData[0] = 0x50U;        /* 0x10 + 0x40 */
    ResponseData[1] = subFunction;
    ResponseData[2] = (uint8)(DCM_P2_SERVER_MAX_MS >> 8U);
    ResponseData[3] = (uint8)(DCM_P2_SERVER_MAX_MS & 0xFFU);
    ResponseData[4] = (uint8)(DCM_P2_STAR_SERVER_MAX_MS >> 8U);
    ResponseData[5] = (uint8)(DCM_P2_STAR_SERVER_MAX_MS & 0xFFU);
    *ResponseLength = 6U;
    return E_OK;
}

/*******************************************************************************
 * SERVICE 0x3E: TesterPresent
 *******************************************************************************/

static Std_ReturnType Dcm_Service_TesterPresent_0x3E(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) RequestData,
    uint16 RequestLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) ResponseData,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) ResponseLength)
{
    uint8 subFunction;

    if ((RequestData == NULL_PTR) || (ResponseData == NULL_PTR) ||
        (ResponseLength == NULL_PTR)) {
        return E_NOT_OK;
    }

    if (RequestLength < 2U) {
        Dcm_Global_NegativeResponseCode = DCM_NRC_INCORRECT_MESSAGE_LENGTH;
        return E_NOT_OK;
    }

    subFunction = RequestData[1];

    if (subFunction != 0x00U) {
        Dcm_Global_NegativeResponseCode = DCM_NRC_SUBFUNCTION_NOT_SUPPORTED;
        return E_NOT_OK;
    }

    Dcm_ResetS3Timer();

    ResponseData[0] = 0x7EU;        /* 0x3E + 0x40 */
    ResponseData[1] = subFunction;
    *ResponseLength = 2U;

    return E_OK;
}

/* End of file */
