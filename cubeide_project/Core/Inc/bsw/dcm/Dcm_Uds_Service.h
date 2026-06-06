/**
 * @file Dcm_Uds_Services.h
 * @brief UDS Diagnostic Services Header File - AUTOSAR Style
 * @author STM32 ECU Demo
 * @date 2024
 * 
 * This file defines UDS service interfaces following AUTOSAR DCM module standards.
 * Supports: 0x22 (ReadDataByIdentifier), 0x2E (WriteDataByIdentifier), 
 *           0x31 (RoutineControl), 0x27 (SecurityAccess)
 */

#ifndef DCM_UDS_SERVICES_H
#define DCM_UDS_SERVICES_H

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

/* Data Identifiers (DIDs) */
#define DID_BOOT_SOFTWARE_ID                        0xF180U
#define DID_ECU_NAME                                0xF183U
#define DID_ACTIVE_DIAGNOSTIC_SESSION               0xF186U
#define DID_SYSTEM_SUPPLIER_ID                      0xF18AU
#define DID_ECU_MANUFACTURING_DATE                  0xF18BU
#define DID_ECU_SERIAL_NUMBER                       0xF18CU
#define DID_VIN                                     0xF190U
#define DID_SYSTEM_SUPPLIER_HW_VERSION              0xF193U
#define DID_SYSTEM_SUPPLIER_SW_VERSION              0xF195U
#define DID_FINGERPRINT                             0xF198U
#define DID_PROGRAMMING_DATE                        0xF199U
#define DID_TEMPERATURE_THRESHOLD                   0xF200U
#define DID_AUTHOR_NAME                             0xF201U
#define DID_PUBLIC_KEY                              0xF300U
#define DID_RESET_COUNTER                           0xF500U
#define DID_FLASH_COUNTER                           0xF501U

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

/* Return Types */
typedef uint8 Std_ReturnType;
#define E_OK                    0x00U
#define E_NOT_OK                0x01U
#define E_SECURITY_ACCESS_DENIED 0x02U
#define E_SESSION_NOT_ALLOWED   0x03U

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
#define DCM_E_REQUEST_TRANSFER_EXIT_NEGATIVE_RESPONSE 0x71U
#define DCM_E_REQUEST_OUT_OF_RANGE                    0x31U

/*******************************************************************************
 * TYPE DEFINITIONS
 *******************************************************************************/

/**
 * @brief DID Data Structure for 0xF190 (VIN)
 */
typedef struct {
    uint8 Vin[17];              /* VIN: 17 bytes ASCII */
} Dcm_Did_F190_VinType;

/**
 * @brief DID Data Structure for 0xF18C (ECU Serial Number)
 */
typedef struct {
    uint8 SerialNumber[32];     /* Serial Number: 32 bytes ASCII */
} Dcm_Did_F18C_SerialType;

/**
 * @brief DID Data Structure for 0xF183 (ECU Name)
 */
typedef struct {
    uint8 EcuName[16];          /* ECU Name: 16 bytes ASCII */
} Dcm_Did_F183_EcuNameType;

/**
 * @brief DID Data Structure for 0xF195 (SW Version)
 */
typedef struct {
    uint8 SwVersion[8];         /* SW Version: 8 bytes ASCII */
} Dcm_Did_F195_SwVersionType;

/**
 * @brief DID Data Structure for 0xF501 (Flash Counter)
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
 * PUBLIC FUNCTION DECLARATIONS
 *******************************************************************************/

/**
 * @brief Read Data By Identifier Service (0x22)
 * 
 * UDS Service 0x22 implementation for reading diagnostic data identifiers.
 * Supported DIDs: 0xF180, 0xF183, 0xF18A, 0xF18C, 0xF190, 0xF193, 0xF195, etc.
 *
 * @param[in]  Dcm_Ptr              Pointer to the DCM service request
 * @param[out] RespData_Ptr         Pointer to response data buffer
 * @param[in]  RespData_Len_Ptr     Pointer to response data length
 * @param[out] ErrorCode_Ptr        Pointer to negative response code
 * 
 * @return     E_OK on success, E_NOT_OK on failure
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_ReadDataByIdentifier_0x22(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Read DID F190 (VIN)
 * 
 * FUNC macro style: Returns Std_ReturnType
 * Implementation of reading Vehicle Identification Number
 *
 * @param[out] VinData_Ptr    Pointer to VIN data structure
 * @param[out] ErrorCode_Ptr  Pointer to error code
 * 
 * @return     E_OK if read successful
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF190_Vin(
    CONSTP2VAR(Dcm_Did_F190_VinType, AUTOMATIC, DCM_APPL_DATA) VinData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Read DID F18C (ECU Serial Number)
 * 
 * Implementation of reading ECU Serial Number
 *
 * @param[out] SerialData_Ptr Pointer to serial number data structure
 * @param[out] ErrorCode_Ptr  Pointer to error code
 * 
 * @return     E_OK if read successful
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF18C_Serial(
    CONSTP2VAR(Dcm_Did_F18C_SerialType, AUTOMATIC, DCM_APPL_DATA) SerialData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Read DID F183 (ECU Name)
 * 
 * Implementation of reading ECU Name
 *
 * @param[out] EcuNameData_Ptr  Pointer to ECU name data structure
 * @param[out] ErrorCode_Ptr    Pointer to error code
 * 
 * @return     E_OK if read successful
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF183_EcuName(
    CONSTP2VAR(Dcm_Did_F183_EcuNameType, AUTOMATIC, DCM_APPL_DATA) EcuNameData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Read DID F195 (Software Version)
 * 
 * Implementation of reading Software Version Number
 *
 * @param[out] SwVersionData_Ptr  Pointer to SW version data structure
 * @param[out] ErrorCode_Ptr      Pointer to error code
 * 
 * @return     E_OK if read successful
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF195_SwVersion(
    CONSTP2VAR(Dcm_Did_F195_SwVersionType, AUTOMATIC, DCM_APPL_DATA) SwVersionData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Read DID F501 (Flash Counter)
 * 
 * Implementation of reading Flash Write Counter
 *
 * @param[out] FlashCounterData_Ptr  Pointer to flash counter data structure
 * @param[out] ErrorCode_Ptr         Pointer to error code
 * 
 * @return     E_OK if read successful
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF501_FlashCounter(
    CONSTP2VAR(Dcm_Did_F501_FlashCounterType, AUTOMATIC, DCM_APPL_DATA) FlashCounterData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Write Data By Identifier Service (0x2E)
 * 
 * UDS Service 0x2E implementation for writing diagnostic data identifiers.
 * Supported DIDs: 0xF190, 0xF198, 0xF199, 0xF200, 0xF201, 0xF501
 *
 * @param[in]  Dcm_Ptr              Pointer to the DCM service request
 * @param[out] RespData_Ptr         Pointer to response data buffer
 * @param[in]  RespData_Len_Ptr     Pointer to response data length
 * @param[out] ErrorCode_Ptr        Pointer to negative response code
 * 
 * @return     E_OK on success, E_NOT_OK on failure
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_WriteDataByIdentifier_0x2E(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Write DID F190 (VIN)
 * 
 * Implementation of writing Vehicle Identification Number
 *
 * @param[in]  VinData_Ptr     Pointer to VIN data to write
 * @param[out] ErrorCode_Ptr   Pointer to error code
 * 
 * @return     E_OK if write successful
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestWriteDidF190_Vin(
    CONSTP2CONST(Dcm_Did_F190_VinType, AUTOMATIC, DCM_APPL_DATA) VinData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Routine Control Service (0x31)
 * 
 * UDS Service 0x31 implementation for routine control.
 * Supports: StartRoutine (0x01), StopRoutine (0x02), RequestRoutineResults (0x03)
 *
 * @param[in]  Dcm_Ptr              Pointer to the DCM service request
 * @param[out] RespData_Ptr         Pointer to response data buffer
 * @param[in]  RespData_Len_Ptr     Pointer to response data length
 * @param[out] ErrorCode_Ptr        Pointer to negative response code
 * 
 * @return     E_OK on success, E_NOT_OK on failure
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_RoutineControl_0x31(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Start Routine 0x0201 (Erase Memory)
 * 
 * Erases application memory area
 *
 * @param[in]  InputData_Ptr      Pointer to input data (memory address + size)
 * @param[in]  InputLength        Length of input data
 * @param[out] OutputData_Ptr     Pointer to output data (erase result)
 * @param[out] OutputLength_Ptr   Pointer to output data length
 * @param[out] ErrorCode_Ptr      Pointer to error code
 * 
 * @return     E_OK if routine started successfully
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_EraseMemory_0x0201(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Start Routine 0x0202 (Check Programming Precondition)
 * 
 * Checks programming preconditions: voltage, session, temperature
 *
 * @param[out] OutputData_Ptr     Pointer to output data (condition result)
 * @param[out] OutputLength_Ptr   Pointer to output data length
 * @param[out] ErrorCode_Ptr      Pointer to error code
 * 
 * @return     E_OK if preconditions satisfied
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_CheckPrecondition_0x0202(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Start Routine 0x0203 (Check Application Integrity)
 * 
 * Verifies application CRC32 integrity
 *
 * @param[in]  InputData_Ptr      Pointer to input data (CRC32 value)
 * @param[in]  InputLength        Length of input data
 * @param[out] OutputData_Ptr     Pointer to output data (verify result)
 * @param[out] OutputLength_Ptr   Pointer to output data length
 * @param[out] ErrorCode_Ptr      Pointer to error code
 * 
 * @return     E_OK if CRC verified successfully
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_CheckIntegrity_0x0203(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Start Routine 0x0303 (CAN Bus Test)
 * 
 * Tests CAN bus communication
 *
 * @param[in]  InputData_Ptr      Pointer to input data (CAN channel)
 * @param[in]  InputLength        Length of input data
 * @param[out] OutputData_Ptr     Pointer to output data (test result)
 * @param[out] OutputLength_Ptr   Pointer to output data length
 * @param[out] ErrorCode_Ptr      Pointer to error code
 * 
 * @return     E_OK if test passed
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_CANBusTest_0x0303(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Security Access Service (0x27)
 * 
 * UDS Service 0x27 implementation for security access levels.
 * Supports: requestSeed (odd subfunctions), sendKey (even subfunctions)
 * Levels: Level 1, Level 2, Level 3, Level 4
 *
 * @param[in]  Dcm_Ptr              Pointer to the DCM service request
 * @param[out] RespData_Ptr         Pointer to response data buffer
 * @param[in]  RespData_Len_Ptr     Pointer to response data length
 * @param[out] ErrorCode_Ptr        Pointer to negative response code
 * 
 * @return     E_OK on success, E_NOT_OK on failure
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_SecurityAccess_0x27(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Request Seed - Security Level 1
 * 
 * Generates and returns a random seed for Level 1 security access
 *
 * @param[out] SeedData_Ptr       Pointer to seed data buffer
 * @param[out] SeedLength_Ptr     Pointer to seed length
 * @param[out] ErrorCode_Ptr      Pointer to error code
 * 
 * @return     E_OK if seed generated successfully
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestSeed_Level1(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) SeedData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) SeedLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Send Key - Security Level 1
 * 
 * Validates security key for Level 1 access
 *
 * @param[in]  KeyData_Ptr         Pointer to key data buffer
 * @param[in]  KeyLength           Length of key data
 * @param[out] ErrorCode_Ptr       Pointer to error code
 * 
 * @return     E_OK if key validated successfully
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_SendKey_Level1(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) KeyData_Ptr,
    uint16 KeyLength,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Request Seed - Security Level 2
 * 
 * Generates and returns a random seed for Level 2 security access
 *
 * @param[out] SeedData_Ptr       Pointer to seed data buffer
 * @param[out] SeedLength_Ptr     Pointer to seed length
 * @param[out] ErrorCode_Ptr      Pointer to error code
 * 
 * @return     E_OK if seed generated successfully
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestSeed_Level2(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) SeedData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) SeedLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

/**
 * @brief Send Key - Security Level 2
 * 
 * Validates security key for Level 2 access (Programming)
 *
 * @param[in]  KeyData_Ptr         Pointer to key data buffer
 * @param[in]  KeyLength           Length of key data
 * @param[out] ErrorCode_Ptr       Pointer to error code
 * 
 * @return     E_OK if key validated successfully
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_SendKey_Level2(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) KeyData_Ptr,
    uint16 KeyLength,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
);

#ifdef __cplusplus
}
#endif

#endif /* DCM_UDS_SERVICES_H */
