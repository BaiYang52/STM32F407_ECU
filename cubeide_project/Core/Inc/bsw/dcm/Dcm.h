/**
 * @file Dcm.h
 * @brief UDS Diagnostic Services Header File - AUTOSAR Style
 * @author STM32 ECU Demo
 * @date 2024
 *
 * This file defines UDS service interfaces following AUTOSAR DCM module standards.
 * Supports:
 *   - 0x10: DiagnosticSessionControl
 *   - 0x11: ECUReset
 *   - 0x22: ReadDataByIdentifier
 *   - 0x27: SecurityAccess
 *   - 0x28: CommunicationControl
 *   - 0x2E: WriteDataByIdentifier
 *   - 0x31: RoutineControl
 *   - 0x3E: TesterPresent
 *   - 0x85: ControlDTCSetting
 */

#ifndef DCM_H
#define DCM_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * INCLUDES
 *******************************************************************************/
#include "Std_Types.h"
#include "Dcm_Uds_Config.h"

/*******************************************************************************
 * DEFINES & MACROS
 *******************************************************************************/

/* UDS Service IDs */
#define UDS_SID_DIAGNOSTIC_SESSION_CONTROL          0x10U
#define UDS_SID_ECU_RESET                           0x11U
#define UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION        0x14U
#define UDS_SID_READ_DTC_INFORMATION                0x19U
#define UDS_SID_READ_DATA_BY_IDENTIFIER             0x22U
#define UDS_SID_READ_MEMORY_BY_ADDRESS              0x23U
#define UDS_SID_READ_SCALING_DATA_BY_IDENTIFIER     0x24U
#define UDS_SID_SECURITY_ACCESS                     0x27U
#define UDS_SID_COMMUNICATION_CONTROL               0x28U
#define UDS_SID_WRITE_DATA_BY_IDENTIFIER            0x2EU
#define UDS_SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER  0x2FU
#define UDS_SID_ROUTINE_CONTROL                     0x31U
#define UDS_SID_REQUEST_DOWNLOAD                    0x34U
#define UDS_SID_TRANSFER_DATA                       0x36U
#define UDS_SID_REQUEST_TRANSFER_EXIT               0x37U
#define UDS_SID_TESTER_PRESENT                      0x3EU
#define UDS_SID_CONTROL_DTC_SETTING                 0x85U

/* Data Identifiers (DIDs) - Complete 16 DID configuration */
#define DID_BOOT_SOFTWARE_ID                        0xF180U  /* DID1: 16 bytes ASCII, ReadOnly */
#define DID_ECU_NAME                                0xF183U  /* DID2: 16 bytes ASCII, ReadOnly */
#define DID_ACTIVE_DIAGNOSTIC_SESSION               0xF186U  /* DID3: 1 byte hex, ReadOnly */
#define DID_SYSTEM_SUPPLIER_ID                      0xF18AU  /* DID4: 2 bytes ASCII, ReadOnly */
#define DID_ECU_MANUFACTURING_DATE                  0xF18BU  /* DID5: 4 bytes BCD, ReadOnly */
#define DID_ECU_SERIAL_NUMBER                       0xF18CU  /* DID6: 32 bytes ASCII, ReadWrite(App Ext L1) */
#define DID_VIN                                     0xF190U  /* DID7: 17 bytes ASCII, ReadWrite */
#define DID_SYSTEM_SUPPLIER_HW_VERSION              0xF193U  /* DID8: 8 bytes ASCII, ReadOnly */
#define DID_SYSTEM_SUPPLIER_SW_VERSION              0xF195U  /* DID9: 8 bytes ASCII, ReadOnly */
#define DID_FINGERPRINT                             0xF198U  /* DID10: 32 bytes ASCII, ReadOnly(App, L2 in FBL) */
#define DID_PROGRAMMING_DATE                        0xF199U  /* DID11: 4 bytes BCD, ReadOnly(App, L2 in FBL) */
#define DID_TEMPERATURE_THRESHOLD                   0xF200U  /* DID12: 2 bytes unsigned, ReadWrite */
#define DID_AUTHOR_NAME                             0xF201U  /* DID13: 16 bytes ASCII, ReadWrite */
#define DID_PUBLIC_KEY                              0xF300U  /* DID14: 64 bytes hex, ReadOnly(App, L2 in FBL) */
#define DID_RESET_COUNTER                           0xF500U  /* DID15: 1 byte unsigned, ReadOnly */
#define DID_FLASH_COUNTER                           0xF501U  /* DID16: 2 bytes unsigned, ReadOnly */
#define DID_BOOT_FLAG                               0xF502U
#define DID_APP_VALID                               0xF503U
#define DID_SECURITY_FAIL_COUNTER                   0xF504U

/* Routine IDs (RIDs) */
#define RID_ERASE_MEMORY                            0x0201U
#define RID_CHECK_PROGRAMMING_PRECONDITION          0x0202U
#define RID_CHECK_APP_INTEGRITY                     0x0203U
#define RID_CHECK_PROGRAMMING_DEPENDENCY            0x0204U
#define RID_ACTIVATE_APPLICATION                    0x0205U
#define RID_BACKUP_APPLICATION                      0x0206U
#define RID_RESTORE_APPLICATION                     0x0207U
#define RID_RESET_DTC_MEMORY                        0x0301U
#define RID_SAVE_RUNTIME_DATA                       0x0302U
#define RID_CAN_BUS_TEST                            0x0303U
#define RID_LED_TEST                                0x0304U
#define RID_MOTOR_TEST                              0x0305U
#define RID_EEPROM_TEST                             0x0306U
#define RID_RAM_TEST                                0x0307U
#define RID_ADC_INPUT_TEST                          0x0308U
#define RID_FORCE_BUS_OFF                           0x0309U
#define RID_WATCHDOG_RESET_TEST                     0x030AU

/* DiagnosticSession Types */
#define DCM_SESSION_DEFAULT                         0x01U
#define DCM_SESSION_PROGRAMMING                     0x02U
#define DCM_SESSION_EXTENDED                        0x03U

/* Data Types */
typedef uint8 Dcm_NegativeResponseCodeType;

/* Negative Response Codes (NRC) */
#define DCM_E_GENERAL_REJECT                        0x31U
#define DCM_E_SERVICE_NOT_SUPPORTED                 0x11U
#define DCM_E_SUBFUNCTION_NOT_SUPPORTED             0x12U
#define DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT        0x13U
#define DCM_E_RESPONSE_TOO_LONG                     0x14U
#define DCM_E_BUSY_REPEAT_REQUEST                   0x21U
#define DCM_E_CONDITIONS_NOT_CORRECT                0x22U
#define DCM_E_REQUEST_SEQUENCE_ERROR                0x24U
#define DCM_E_NO_ACCESS_TO_REQUESTED_DID            0x31U
#define DCM_E_SECURITY_ACCESS_DENIED                0x33U
#define DCM_E_INVALID_KEY                           0x35U
#define DCM_E_EXCEEDED_NUMBER_OF_ATTEMPTS           0x36U
#define DCM_E_REQUIRED_TIME_DELAY_NOT_EXPIRED       0x37U
#define DCM_E_REQUEST_DOWNLOAD_NOT_ACCEPTED         0x70U
#define DCM_E_REQUEST_UPLOAD_NOT_ACCEPTED           0x71U
#define DCM_E_DATA_TRANSFER_ABORTED                 0x72U
#define DCM_E_GENERAL_PROGRAMMING_FAILURE           0x72U
#define DCM_E_TRANSFER_DATA_CRCERROR                0x73U
#define DCM_E_REQUEST_TRANSFER_EXIT_NEGATIVE_RESPONSE    0x71U
#define DCM_E_SUBFUNCTION_NOT_SUPPORT_IN_CURRENT_SESSION 0x7EU
#define DCM_E_SERVICE_NOT_SUPPORT_IN_CURRENT_SESSION     0x7FU
#define DCM_E_REQUEST_OUT_OF_RANGE                    0x31U

/*******************************************************************************
 * TYPE DEFINITIONS
 *******************************************************************************/
typedef enum
{
    DCM_ADDRESS_PHYSICAL    = 0U,   /**< UDS 物理寻址 : CAN ID 0x7A0->0x7A8 */
    DCM_ADDRESS_FUNCTIONAL  = 1U,   /**< UDS 功能寻址 : CAN ID 0x7DF */
    DCM_MAX_ADDRESS_NUMBER          /**< 最大 ID */
} DCM_AddressType;

/**
 * @brief DID Data Structure for 0xF180 (Boot Software ID) - DID1
 */
typedef struct {
    uint8 BootSwId[16];         /* Boot SW ID: 16 bytes ASCII */
} Dcm_Did_F180_BootSwIdType;

/**
 * @brief DID Data Structure for 0xF183 (ECU Name) - DID2
 */
typedef struct {
    uint8 EcuName[16];          /* ECU Name: 16 bytes ASCII */
} Dcm_Did_F183_EcuNameType;

/**
 * @brief DID Data Structure for 0xF186 (Active Diagnostic Session) - DID3
 */
typedef struct {
    uint8 Session;              /* Current session: 1 byte hex */
} Dcm_Did_F186_SessionType;

/**
 * @brief DID Data Structure for 0xF18A (System Supplier ID) - DID4 (2 bytes)
 */
typedef struct {
    uint8 SupplierId[2];        /* Supplier ID: 2 bytes ASCII ("ST") */
} Dcm_Did_F18A_SupplierIdType;

/**
 * @brief DID Data Structure for 0xF18B (ECU Manufacturing Date) - DID5
 */
typedef struct {
    uint8 MfgDate[4];           /* Manufacturing Date: 4 bytes BCD */
} Dcm_Did_F18B_MfgDateType;

/**
 * @brief DID Data Structure for 0xF18C (ECU Serial Number) - DID6
 */
typedef struct {
    uint8 SerialNumber[32];     /* Serial Number: 32 bytes ASCII */
} Dcm_Did_F18C_SerialType;

/**
 * @brief DID Data Structure for 0xF190 (VIN) - DID7
 */
typedef struct {
    uint8 Vin[17];              /* VIN: 17 bytes ASCII */
} Dcm_Did_F190_VinType;

/**
 * @brief DID Data Structure for 0xF193 (System Supplier HW Version) - DID8
 */
typedef struct {
    uint8 HwVersion[8];         /* HW Version: 8 bytes ASCII */
} Dcm_Did_F193_HwVersionType;

/**
 * @brief DID Data Structure for 0xF195 (System Supplier SW Version) - DID9
 */
typedef struct {
    uint8 SwVersion[8];         /* SW Version: 8 bytes ASCII */
} Dcm_Did_F195_SwVersionType;

/**
 * @brief DID Data Structure for 0xF198 (Fingerprint) - DID10
 */
typedef struct {
    uint8 Fingerprint[32];      /* Fingerprint: 32 bytes ASCII */
} Dcm_Did_F198_FingerprintType;

/**
 * @brief DID Data Structure for 0xF199 (Programming Date) - DID11
 */
typedef struct {
    uint8 ProgDate[4];          /* Programming Date: 4 bytes BCD */
} Dcm_Did_F199_ProgDateType;

/**
 * @brief DID Data Structure for 0xF200 (Temperature Threshold) - DID12
 */
typedef struct {
    uint16 TempThreshold;       /* Temperature Threshold: 2 bytes unsigned */
} Dcm_Did_F200_TempThresholdType;

/**
 * @brief DID Data Structure for 0xF201 (Author Name) - DID13
 */
typedef struct {
    uint8 AuthorName[16];       /* Author Name: 16 bytes ASCII */
} Dcm_Did_F201_AuthorNameType;

/**
 * @brief DID Data Structure for 0xF300 (Public Key) - DID14
 */
typedef struct {
    uint8 PublicKey[64];        /* Public Key: 64 bytes hex */
} Dcm_Did_F300_PublicKeyType;

/**
 * @brief DID Data Structure for 0xF500 (Reset Counter) - DID15
 */
typedef struct {
    uint8 ResetCounter;         /* Reset Counter: 1 byte unsigned */
} Dcm_Did_F500_ResetCounterType;

/**
 * @brief DID Data Structure for 0xF501 (Flash Counter) - DID16
 */
typedef struct {
    uint16 FlashCounter;        /* Flash Counter: 2 bytes unsigned */
} Dcm_Did_F501_FlashCounterType;

/**
 * @brief Routine Control Input/Output Structure
 */
typedef struct {
    uint16 RoutineId;
    uint8 SubFunction;          /* 0x01: Start, 0x02: Stop, 0x03: Request Results */
    uint8 *InputBuffer;
    uint16 InputLength;
    uint8 *OutputBuffer;
    uint16 OutputLength;
} Dcm_RoutineControlType;

/*******************************************************************************
 * GLOBAL VARIABLES (跨模块共享)
 *******************************************************************************/

/**
 * @brief 全局否定响应码
 *
 * 当外部服务文件 (如 Dcm_Uds_ECUReset.c) 需要返回特定的 NRC 时,
 * 设置此变量后由 Dcm.c 的 dispatcher 读取并构建否定响应。
 */
extern uint8 Dcm_Global_NegativeResponseCode;

extern DCM_AddressType Dcm_RequestAddressType;

extern uint8 Dcm_CurrentSession;

extern boolean Dcm_SecurityLevel1Unlocked;

extern boolean Dcm_DTCSettingEnabled;

extern boolean Dcm_SuppressPositiveResponse; 

extern boolean Dcm_SuppressNegativeResponse; 

extern boolean Dcm_SecurityAccessSequence;

extern boolean Dcm_ECUResetPending;
/*******************************************************************************
 * DCM CORE FUNCTION DECLARATIONS
 *******************************************************************************/

/**
 * @brief DCM 初始化
 *
 * 注册回调到 PduR, 初始化内部状态变量。
 */
FUNC(void, DCM_CODE) Dcm_Init(void);

/**
 * @brief DCM 主函数 (每 10ms 周期调用)
 *
 * 处理 S3 超时 (自动回退到 Default Session)
 */
FUNC(void, DCM_CODE) Dcm_MainFunction(void);

/**
 * @brief 获取当前诊断会话
 * @return 当前会话 ID (0x01/0x02/0x03)
 */
FUNC(uint8, DCM_CODE) Dcm_GetCurrentSession(void);

/**
 * @brief 获取安全级别
 * @return 当前安全级别 (0/1)
 */
FUNC(uint8, DCM_CODE) Dcm_GetSecurityLevel(void);

/**
 * @brief 检查安全级别是否解锁
 * @param[in] SecurityLevel  安全级别 (1)
 * @return TRUE 已解锁, FALSE 未解锁
 */
FUNC(boolean, DCM_CODE) Dcm_IsSecurityLevelUnlocked(uint8 SecurityLevel);

/**
 * @brief 设置安全级别状态
 * @param[in] SecurityLevel  安全级别 (1)
 * @param[in] Unlocked       TRUE=解锁, FALSE=锁定
 */
FUNC(void, DCM_CODE) Dcm_SetSecurityLevel(uint8 SecurityLevel, boolean Unlocked);

/**
 * @brief 获取 DTC 设置状态
 * @return TRUE=允许 DTC, FALSE=禁止 DTC
 */
FUNC(boolean, DCM_CODE) Dcm_GetDTCSettingEnabled(void);

/**
 * @brief 设置 DTC 设置状态
 * @param[in] Enabled  TRUE=允许, FALSE=禁止
 */
FUNC(void, DCM_CODE) Dcm_SetDTCSettingEnabled(boolean Enabled);

/**
 * @brief 获取通信控制状态
 * @return 通信状态 (0x00/0x01/0x03)
 */
FUNC(uint8, DCM_CODE) Dcm_GetCommState(void);

/**
 * @brief 设置通信控制状态
 * @param[in] CommState  通信状态 (0x00=Enable/0x01=RxOnly/0x03=Disable)
 */
FUNC(void, DCM_CODE) Dcm_SetCommState(uint8 CommState);

/**
 * @brief 复位 TesterPresent 定时器
 */
FUNC(void, DCM_CODE) Dcm_TesterPresentReset(void);

/**
 * @brief Check subfunction to decide if Suppress Positive Response
* @param[in] subfunction  Subfunction
 */
void Dcm_CheckIfSuppressPositiveResponse(uint8 subfunction);
/*******************************************************************************
 * UDS SERVICE FUNCTION DECLARATIONS
 *******************************************************************************/

/* ==================== 0x22: ReadDataByIdentifier ==================== */

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_ReadDataByIdentifier_0x22(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF180_BootSwId(
    CONSTP2VAR(Dcm_Did_F180_BootSwIdType, AUTOMATIC, DCM_APPL_DATA) BootSwIdData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF183_EcuName(
    CONSTP2VAR(Dcm_Did_F183_EcuNameType, AUTOMATIC, DCM_APPL_DATA) EcuNameData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF186_Session(
    CONSTP2VAR(Dcm_Did_F186_SessionType, AUTOMATIC, DCM_APPL_DATA) SessionData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF18A_SupplierId(
    CONSTP2VAR(Dcm_Did_F18A_SupplierIdType, AUTOMATIC, DCM_APPL_DATA) SupplierIdData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF18B_MfgDate(
    CONSTP2VAR(Dcm_Did_F18B_MfgDateType, AUTOMATIC, DCM_APPL_DATA) MfgDateData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF18C_Serial(
    CONSTP2VAR(Dcm_Did_F18C_SerialType, AUTOMATIC, DCM_APPL_DATA) SerialData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF190_Vin(
    CONSTP2VAR(Dcm_Did_F190_VinType, AUTOMATIC, DCM_APPL_DATA) VinData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF193_HwVersion(
    CONSTP2VAR(Dcm_Did_F193_HwVersionType, AUTOMATIC, DCM_APPL_DATA) HwVersionData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF195_SwVersion(
    CONSTP2VAR(Dcm_Did_F195_SwVersionType, AUTOMATIC, DCM_APPL_DATA) SwVersionData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF198_Fingerprint(
    CONSTP2VAR(Dcm_Did_F198_FingerprintType, AUTOMATIC, DCM_APPL_DATA) FingerprintData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF199_ProgDate(
    CONSTP2VAR(Dcm_Did_F199_ProgDateType, AUTOMATIC, DCM_APPL_DATA) ProgDateData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF200_TempThreshold(
    CONSTP2VAR(Dcm_Did_F200_TempThresholdType, AUTOMATIC, DCM_APPL_DATA) TempThresholdData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF201_AuthorName(
    CONSTP2VAR(Dcm_Did_F201_AuthorNameType, AUTOMATIC, DCM_APPL_DATA) AuthorNameData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF300_PublicKey(
    CONSTP2VAR(Dcm_Did_F300_PublicKeyType, AUTOMATIC, DCM_APPL_DATA) PublicKeyData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF500_ResetCounter(
    CONSTP2VAR(Dcm_Did_F500_ResetCounterType, AUTOMATIC, DCM_APPL_DATA) ResetCounterData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF501_FlashCounter(
    CONSTP2VAR(Dcm_Did_F501_FlashCounterType, AUTOMATIC, DCM_APPL_DATA) FlashCounterData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/* ==================== 0x2E: WriteDataByIdentifier ==================== */

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_WriteDataByIdentifier_0x2E(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestWriteDidF190_Vin(
    CONSTP2CONST(Dcm_Did_F190_VinType, AUTOMATIC, DCM_APPL_DATA) VinData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestWriteDidF198_Fingerprint(
    CONSTP2CONST(Dcm_Did_F198_FingerprintType, AUTOMATIC, DCM_APPL_DATA) FingerprintData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestWriteDidF199_ProgDate(
    CONSTP2CONST(Dcm_Did_F199_ProgDateType, AUTOMATIC, DCM_APPL_DATA) ProgDateData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestWriteDidF200_TempThreshold(
    CONSTP2CONST(Dcm_Did_F200_TempThresholdType, AUTOMATIC, DCM_APPL_DATA) TempThresholdData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestWriteDidF201_AuthorName(
    CONSTP2CONST(Dcm_Did_F201_AuthorNameType, AUTOMATIC, DCM_APPL_DATA) AuthorNameData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestWriteDidF300_PublicKey(
    CONSTP2CONST(Dcm_Did_F300_PublicKeyType, AUTOMATIC, DCM_APPL_DATA) PublicKeyData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/* ==================== 0x27: SecurityAccess ==================== */

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_SecurityAccess_0x27(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestSeed_Level1(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) SeedData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) SeedLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_SendKey_Level1(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) KeyData_Ptr,
    uint16 KeyLength,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/* ==================== 0x31: RoutineControl ==================== */

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_RoutineControl_0x31(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_EraseMemory_0x0201(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_CheckPrecondition_0x0202(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_CheckIntegrity_0x0203(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_CANBusTest_0x0303(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/* ==================== 0x11: ECUReset ==================== */

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_ECUReset_0x11(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) RequestData,
    uint16 RequestLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) ResponseData,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) ResponseLength);

/* ==================== 0x28: CommunicationControl ==================== */

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_CommunicationControl_0x28(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) RequestData,
    uint16 RequestLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) ResponseData,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) ResponseLength);

/* ==================== 0x85: ControlDTCSetting ==================== */

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_ControlDTCSetting_0x85(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) RequestData,
    uint16 RequestLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) ResponseData,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) ResponseLength);

#ifdef __cplusplus
}
#endif

#endif /* DCM_H */