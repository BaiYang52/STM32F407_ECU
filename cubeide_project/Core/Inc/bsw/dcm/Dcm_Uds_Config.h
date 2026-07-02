/**
 * @file Dcm_Uds_Config.h
 * @brief AUTOSAR UDS Services Configuration - Integration Guide
 * @author STM32 ECU Demo
 * @date 2024
 * 
 * Configuration file for UDS service integration in ECU application
 */

#ifndef DCM_UDS_CONFIG_H
#define DCM_UDS_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * PROJECT STRUCTURE
 * 
 * Standard AUTOSAR ECU project structure for UDS implementation:
 *
 * project-root/
 * ├── src/
 * │   ├── bsw/
 * │   │   ├── mcal/
 * │   │   │   └── dcm/
 * │   │   │       ├── Dcm.h                    (Core API + service declarations)
 * │   │   │       ├── Dcm.c                    (Core + 0x10, 0x3E)
 * │   │   │       ├── Dcm_Uds_Services.c       (0x22, 0x2E, 0x27)
 * │   │   │       ├── Dcm_Uds_RoutineControl.c (0x31)
 * │   │   │       ├── Dcm_Uds_ECUReset.c       (0x11)
 * │   │   │       ├── Dcm_Uds_CommunicationControl.c (0x28)
 * │   │   │       ├── Dcm_Uds_ControlDTCSetting.c    (0x85)
 * │   │   │       └── Dcm_Uds_Config.h         (This file)
 *
 *******************************************************************************/

/*******************************************************************************
 * CONFIGURATION PARAMETERS
 *******************************************************************************/

/* ============================================================================
 * 1. SERVICE SUPPORT CONFIGURATION
 * ============================================================================
 */

/* Enable/Disable UDS Services */
#define DCM_SERVICE_0x10_ENABLED                1U      /* DiagnosticSessionControl (Dcm.c) */
#define DCM_SERVICE_0x11_ENABLED                1U      /* EcuReset (Dcm_Uds_ECUReset.c) */
#define DCM_SERVICE_0x14_ENABLED                0U      /* ClearDiagnosticInformation (未实现) */
#define DCM_SERVICE_0x19_ENABLED                0U      /* ReadDTCInformation (未实现) */
#define DCM_SERVICE_0x22_ENABLED                1U      /* ReadDataByIdentifier (Dcm_Uds_Services.c) */
#define DCM_SERVICE_0x27_ENABLED                1U      /* SecurityAccess (Dcm_Uds_Services.c) */
#define DCM_SERVICE_0x28_ENABLED                1U      /* CommunicationControl (Dcm_Uds_CommunicationControl.c) */
#define DCM_SERVICE_0x2E_ENABLED                1U      /* WriteDataByIdentifier (Dcm_Uds_Services.c) */
#define DCM_SERVICE_0x31_ENABLED                1U      /* RoutineControl (Dcm_Uds_RoutineControl.c) */
#define DCM_SERVICE_0x3E_ENABLED                1U      /* TesterPresent (Dcm.c) */
#define DCM_SERVICE_0x85_ENABLED                1U      /* ControlDTCSetting (Dcm_Uds_ControlDTCSetting.c) */

/* ============================================================================
 * 2. DID CONFIGURATION
 * ============================================================================
 */

/* Number of supported DIDs */
#define DCM_NUM_SUPPORTED_DIDS                  16U

/* DID Buffer Sizes */
#define DCM_DID_MAX_READ_LENGTH                 256U
#define DCM_DID_MAX_WRITE_LENGTH                256U
#define DCM_DID_CACHE_ENABLED                   1U      /* Enable DID caching */

/* DID Access Rights & Security Configuration */
/* F180: Boot SW ID - ReadOnly all sessions, writable only by supplier */
#define DCM_DID_F180_READABLE                   1U
#define DCM_DID_F180_WRITABLE                   0U
#define DCM_DID_F180_SECURITY_LEVEL             0U
#define DCM_DID_F180_SESSION_APP_DEF            1U
#define DCM_DID_F180_SESSION_APP_EXT            1U
#define DCM_DID_F180_SESSION_BOOT_DEF           1U
#define DCM_DID_F180_SESSION_BOOT_PROG          1U
#define DCM_DID_F180_SESSION_BOOT_EXT           1U

/* F183: ECU Name - ReadOnly, App only */
#define DCM_DID_F183_READABLE                   1U
#define DCM_DID_F183_WRITABLE                   0U
#define DCM_DID_F183_SECURITY_LEVEL             0U
#define DCM_DID_F183_SESSION_APP_DEF            1U
#define DCM_DID_F183_SESSION_APP_EXT            1U
#define DCM_DID_F183_SESSION_BOOT_DEF           0U
#define DCM_DID_F183_SESSION_BOOT_PROG          0U
#define DCM_DID_F183_SESSION_BOOT_EXT           0U

/* F186: Active Diag Session - ReadOnly, App only */
#define DCM_DID_F186_READABLE                   1U
#define DCM_DID_F186_WRITABLE                   0U
#define DCM_DID_F186_SECURITY_LEVEL             0U
#define DCM_DID_F186_SESSION_APP_DEF            1U
#define DCM_DID_F186_SESSION_APP_EXT            1U
#define DCM_DID_F186_SESSION_BOOT_DEF           0U
#define DCM_DID_F186_SESSION_BOOT_PROG          0U
#define DCM_DID_F186_SESSION_BOOT_EXT           0U

/* F18A: Supplier ID - ReadOnly, all sessions */
#define DCM_DID_F18A_READABLE                   1U
#define DCM_DID_F18A_WRITABLE                   0U
#define DCM_DID_F18A_SECURITY_LEVEL             0U
#define DCM_DID_F18A_SESSION_APP_DEF            1U
#define DCM_DID_F18A_SESSION_APP_EXT            1U
#define DCM_DID_F18A_SESSION_BOOT_DEF           1U
#define DCM_DID_F18A_SESSION_BOOT_PROG          1U
#define DCM_DID_F18A_SESSION_BOOT_EXT           1U

/* F18B: Mfg Date - ReadOnly, App only */
#define DCM_DID_F18B_READABLE                   1U
#define DCM_DID_F18B_WRITABLE                   0U
#define DCM_DID_F18B_SECURITY_LEVEL             0U
#define DCM_DID_F18B_SESSION_APP_DEF            1U
#define DCM_DID_F18B_SESSION_APP_EXT            1U
#define DCM_DID_F18B_SESSION_BOOT_DEF           0U
#define DCM_DID_F18B_SESSION_BOOT_PROG          0U
#define DCM_DID_F18B_SESSION_BOOT_EXT           0U

/* F18C: Serial Number - Read App only, Write App Ext L1 */
#define DCM_DID_F18C_READABLE                   1U
#define DCM_DID_F18C_WRITABLE                   1U
#define DCM_DID_F18C_WRITE_SECURITY_LEVEL       1U
#define DCM_DID_F18C_SESSION_APP_DEF            1U
#define DCM_DID_F18C_SESSION_APP_EXT            1U
#define DCM_DID_F18C_SESSION_BOOT_DEF           0U
#define DCM_DID_F18C_SESSION_BOOT_PROG          0U
#define DCM_DID_F18C_SESSION_BOOT_EXT           0U

/* F190: VIN - Read App only, Write App Ext L1 */
#define DCM_DID_F190_READABLE                   1U
#define DCM_DID_F190_WRITABLE                   1U
#define DCM_DID_F190_WRITE_SECURITY_LEVEL       1U
#define DCM_DID_F190_SESSION_APP_DEF            1U
#define DCM_DID_F190_SESSION_APP_EXT            1U
#define DCM_DID_F190_SESSION_BOOT_DEF           0U
#define DCM_DID_F190_SESSION_BOOT_PROG          0U
#define DCM_DID_F190_SESSION_BOOT_EXT           0U

/* F193: HW Version - ReadOnly, App only */
#define DCM_DID_F193_READABLE                   1U
#define DCM_DID_F193_WRITABLE                   0U
#define DCM_DID_F193_SECURITY_LEVEL             0U
#define DCM_DID_F193_SESSION_APP_DEF            1U
#define DCM_DID_F193_SESSION_APP_EXT            1U
#define DCM_DID_F193_SESSION_BOOT_DEF           0U
#define DCM_DID_F193_SESSION_BOOT_PROG          0U
#define DCM_DID_F193_SESSION_BOOT_EXT           0U

/* F195: SW Version - ReadOnly, App only */
#define DCM_DID_F195_READABLE                   1U
#define DCM_DID_F195_WRITABLE                   0U
#define DCM_DID_F195_SECURITY_LEVEL             0U
#define DCM_DID_F195_SESSION_APP_DEF            1U
#define DCM_DID_F195_SESSION_APP_EXT            1U
#define DCM_DID_F195_SESSION_BOOT_DEF           0U
#define DCM_DID_F195_SESSION_BOOT_PROG          0U
#define DCM_DID_F195_SESSION_BOOT_EXT           0U

/* F198: Fingerprint - Read all sessions, Write only in FBL (Boot Prog, L2) */
#define DCM_DID_F198_READABLE                   1U
#define DCM_DID_F198_WRITABLE                   0U      /* Write in FBL only (L2) */
#define DCM_DID_F198_SESSION_APP_DEF            1U
#define DCM_DID_F198_SESSION_APP_EXT            1U
#define DCM_DID_F198_SESSION_BOOT_DEF           1U
#define DCM_DID_F198_SESSION_BOOT_PROG          1U
#define DCM_DID_F198_SESSION_BOOT_EXT           1U

/* F199: Programming Date - Read all sessions, Write only in FBL (Boot Prog, L2) */
#define DCM_DID_F199_READABLE                   1U
#define DCM_DID_F199_WRITABLE                   0U      /* Write in FBL only (L2) */
#define DCM_DID_F199_SESSION_APP_DEF            1U
#define DCM_DID_F199_SESSION_APP_EXT            1U
#define DCM_DID_F199_SESSION_BOOT_DEF           1U
#define DCM_DID_F199_SESSION_BOOT_PROG          1U
#define DCM_DID_F199_SESSION_BOOT_EXT           1U

/* F200: Temp Threshold - Read App only, Write App Ext L1 */
#define DCM_DID_F200_READABLE                   1U
#define DCM_DID_F200_WRITABLE                   1U
#define DCM_DID_F200_WRITE_SECURITY_LEVEL       1U
#define DCM_DID_F200_SESSION_APP_DEF            1U
#define DCM_DID_F200_SESSION_APP_EXT            1U
#define DCM_DID_F200_SESSION_BOOT_DEF           0U
#define DCM_DID_F200_SESSION_BOOT_PROG          0U
#define DCM_DID_F200_SESSION_BOOT_EXT           0U

/* F201: Author Name - Read App only, Write App Ext L1 */
#define DCM_DID_F201_READABLE                   1U
#define DCM_DID_F201_WRITABLE                   1U
#define DCM_DID_F201_WRITE_SECURITY_LEVEL       1U
#define DCM_DID_F201_SESSION_APP_DEF            1U
#define DCM_DID_F201_SESSION_APP_EXT            1U
#define DCM_DID_F201_SESSION_BOOT_DEF           0U
#define DCM_DID_F201_SESSION_BOOT_PROG          0U
#define DCM_DID_F201_SESSION_BOOT_EXT           0U

/* F300: Public Key - Read Boot only, Write only in FBL (Boot Prog, L2) */
#define DCM_DID_F300_READABLE                   0U      /* Not readable in App (Boot only) */
#define DCM_DID_F300_WRITABLE                   0U      /* Write in FBL only (L2) */
#define DCM_DID_F300_SESSION_APP_DEF            0U
#define DCM_DID_F300_SESSION_APP_EXT            0U
#define DCM_DID_F300_SESSION_BOOT_DEF           1U
#define DCM_DID_F300_SESSION_BOOT_PROG          1U
#define DCM_DID_F300_SESSION_BOOT_EXT           1U

/* F500: Reset Counter - ReadOnly, App only */
#define DCM_DID_F500_READABLE                   1U
#define DCM_DID_F500_WRITABLE                   0U
#define DCM_DID_F500_SECURITY_LEVEL             0U
#define DCM_DID_F500_SESSION_APP_DEF            1U
#define DCM_DID_F500_SESSION_APP_EXT            1U
#define DCM_DID_F500_SESSION_BOOT_DEF           0U
#define DCM_DID_F500_SESSION_BOOT_PROG          0U
#define DCM_DID_F500_SESSION_BOOT_EXT           0U

/* F501: Flash Counter - ReadOnly, all sessions */
#define DCM_DID_F501_READABLE                   1U
#define DCM_DID_F501_WRITABLE                   0U
#define DCM_DID_F501_SECURITY_LEVEL             0U
#define DCM_DID_F501_SESSION_APP_DEF            1U
#define DCM_DID_F501_SESSION_APP_EXT            1U
#define DCM_DID_F501_SESSION_BOOT_DEF           1U
#define DCM_DID_F501_SESSION_BOOT_PROG          1U
#define DCM_DID_F501_SESSION_BOOT_EXT           1U

/* ============================================================================
 * 3. ROUTINE CONTROL CONFIGURATION
 * ============================================================================
 */

/* Number of supported routines */
#define DCM_NUM_SUPPORTED_ROUTINES              10U

/* Routine timeout (milliseconds) */
#define DCM_ROUTINE_ERASE_TIMEOUT               30000U
#define DCM_ROUTINE_PRECONDITION_TIMEOUT        500U
#define DCM_ROUTINE_INTEGRITY_TIMEOUT           5000U
#define DCM_ROUTINE_CAN_TEST_TIMEOUT            2000U

/* ============================================================================
 * 4. SECURITY ACCESS CONFIGURATION
 * ============================================================================
 */

/* Security Levels */
#define DCM_SECURITY_NUM_LEVELS                 1U
#define DCM_SECURITY_SEED_LENGTH                4U      /* Bytes */
#define DCM_SECURITY_KEY_LENGTH                 4U      /* Bytes */
#define DCM_SECURITY_MAX_ATTEMPTS               3U      /* Max key validation attempts */
#define DCM_SECURITY_LOCKOUT_TIME               5000U   /* ms */

/* Security Level 1: Extended Diagnostic Access */
#define DCM_SECURITY_LEVEL_1_TIMEOUT            10000U  /* 10 seconds */
#define DCM_SECURITY_LEVEL_1_ATTEMPT_LIMIT      3U

/* Security Level 2: Programming Access */
#define DCM_SECURITY_LEVEL_2_TIMEOUT            20000U  /* 20 seconds */
#define DCM_SECURITY_LEVEL_2_ATTEMPT_LIMIT      3U

/* ============================================================================
 * 5. SESSION CONFIGURATION
 * ============================================================================
 */

#define DCM_SESSION_DEFAULT                     0x01U
#define DCM_SESSION_PROGRAMMING                 0x02U
#define DCM_SESSION_EXTENDED                    0x03U

/* Service availability per session */
#define DCM_SERVICE_0x22_IN_DEFAULT_SESSION     1U
#define DCM_SERVICE_0x22_IN_PROGRAMMING_SESSION 1U
#define DCM_SERVICE_0x22_IN_EXTENDED_SESSION    1U

#define DCM_SERVICE_0x2E_IN_DEFAULT_SESSION     0U
#define DCM_SERVICE_0x2E_IN_PROGRAMMING_SESSION 1U
#define DCM_SERVICE_0x2E_IN_EXTENDED_SESSION    1U

#define DCM_SERVICE_0x31_IN_DEFAULT_SESSION     0U
#define DCM_SERVICE_0x31_IN_PROGRAMMING_SESSION 1U
#define DCM_SERVICE_0x31_IN_EXTENDED_SESSION    1U

/* ============================================================================
 * 6. COMMUNICATION PARAMETERS
 * ============================================================================
 */

/* CAN Parameters */
#define DCM_CAN_CHANNEL                         0U      /* CAN1 */
#define DCM_CAN_REQUEST_ID                      0x7A0U  /* Physical request */
#define DCM_CAN_FUNCTIONAL_REQUEST_ID           0x7DFU  /* Functional request */
#define DCM_CAN_RESPONSE_ID                     0x7A8U  /* Response ID */

/* UDS Timing Parameters (from requirement document) */
#define DCM_P2_SERVER_MIN                       0U      /* ms */
#define DCM_P2_SERVER_MAX                       50U     /* ms */
#define DCM_P2_STAR_SERVER_MAX                  5000U   /* ms (5 seconds) */
#define DCM_S3_SERVER_TIMEOUT                   5000U   /* ms */
#define DCM_S4_SERVER_TIMEOUT                   8000U   /* ms */

/* Diagnostic Data Sizes */
#define DCM_REQUEST_BUFFER_SIZE                 256U
#define DCM_RESPONSE_BUFFER_SIZE                256U

/* ============================================================================
 * 7. FLASH/NVRAM PARAMETERS
 * ============================================================================
 */

/* Application Area */
#define DCM_APP_START_ADDRESS                   0x08010000U
#define DCM_APP_END_ADDRESS                     0x080FFFFFU
#define DCM_APP_SIZE                            (0x000F0000U)  /* 960 KB */

/* DID Storage in NVRAM (each DID has a fixed offset in NVRAM partition) */
#define DCM_NVRAM_DID_BASE_ADDRESS              0x08000000U
#define DCM_NVRAM_DID_F180_OFFSET               0x0000U   /* BOOT_SW_ID: 16 bytes */
#define DCM_NVRAM_DID_F183_OFFSET               0x0010U   /* ECU_NAME: 16 bytes */
#define DCM_NVRAM_DID_F18A_OFFSET               0x0020U   /* SUPPLIER_ID: 3 bytes */
#define DCM_NVRAM_DID_F18B_OFFSET               0x0024U   /* MFG_DATE: 4 bytes BCD */
#define DCM_NVRAM_DID_F18C_OFFSET               0x0028U   /* SERIAL_NUM: 32 bytes */
#define DCM_NVRAM_DID_F190_OFFSET               0x0048U   /* VIN: 17 bytes */
#define DCM_NVRAM_DID_F193_OFFSET               0x0060U   /* HW_VERSION: 8 bytes */
#define DCM_NVRAM_DID_F195_OFFSET               0x0068U   /* SW_VERSION: 8 bytes */
#define DCM_NVRAM_DID_F198_OFFSET               0x0070U   /* FINGERPRINT: 32 bytes */
#define DCM_NVRAM_DID_F199_OFFSET               0x0090U   /* PROG_DATE: 4 bytes BCD */
#define DCM_NVRAM_DID_F200_OFFSET               0x0094U   /* TEMP_THRESHOLD: 2 bytes */
#define DCM_NVRAM_DID_F201_OFFSET               0x0096U   /* AUTHOR_NAME: 16 bytes */
#define DCM_NVRAM_DID_F300_OFFSET               0x00A8U   /* PUBLIC_KEY: 64 bytes */
#define DCM_NVRAM_DID_F500_OFFSET               0x00E8U   /* RESET_COUNTER: 1 byte */
#define DCM_NVRAM_DID_F501_OFFSET               0x00EAU   /* FLASH_COUNTER: 2 bytes */

/* ============================================================================
 * 8. FEATURES & DEBUG
 * ============================================================================
 */

#define DCM_DEV_ERROR_DETECT                    1U      /* Enable DET */
#define DCM_DISABLE_INTERNAL_DET                0U      /* Use external DET */
#define DCM_INCLUDE_RTE                         1U      /* Use AUTOSAR RTE */
#define DCM_DEBUG_ENABLED                       0U      /* Debug logs */

/*******************************************************************************
 * TYPE DEFINITIONS
 *******************************************************************************/

/**
 * @brief Diagnostic Message Type (AUTOSAR compliant)
 */
typedef struct {
    uint8 Sdu[256];                 /* Service Data Unit buffer */
    uint16 SduLength;               /* Actual data length */
    uint8 Padding;                  /* For alignment */
} Dcm_MsgType;

/**
 * @brief DID Configuration Entry
 */
typedef struct {
    uint16 DidId;                   /* DID identifier */
    uint16 DidLength;               /* Data length for this DID */
    uint8 ReadAccessible;           /* Read permission flag */
    uint8 WriteAccessible;          /* Write permission flag */
    uint8 SecurityLevel;            /* Required security level for write */
    uint8 SessionMask;              /* Session availability */
} Dcm_DidConfigType;

/**
 * @brief Routine Configuration Entry
 */
typedef struct {
    uint16 RoutineId;               /* RID */
    uint8 RoutineType;              /* Start, Stop, RequestResults */
    uint16 MaxInputLength;
    uint16 MaxOutputLength;
    uint16 TimeoutMs;
    uint8 SecurityLevel;
} Dcm_RoutineConfigType;

/*******************************************************************************
 * PUBLIC DATA DECLARATIONS
 *******************************************************************************/

/**
 * DID Configuration Table
 */
extern const Dcm_DidConfigType Dcm_DidConfigTable[DCM_NUM_SUPPORTED_DIDS];

/**
 * Routine Configuration Table
 */
extern const Dcm_RoutineConfigType Dcm_RoutineConfigTable[DCM_NUM_SUPPORTED_ROUTINES];

#define Rng_GenerateRandomNumber Rte_Hal_Rng_GenerateRandomNumber
#define GetSystemTick Rte_Hal_GetTick
#define GetVehicleSpeed Rte_Read_VehicleCtrl_Port_Veh_Speed
/*******************************************************************************
 * INTEGRATION CHECKLIST
 *******************************************************************************/

/*
 * ============================================================================
 * STEP 1: ADD HEADER FILES
 * ============================================================================
 * 
 * In your project's include paths, add:
 * - Dcm.h
 * - Dcm_Uds_Config.h
 * - Std_Types.h (AUTOSAR standard types)
 *
 */

/*
 * ============================================================================
 * STEP 2: LINK SOURCE FILES
 * ============================================================================
 * 
 * Add to your build system (Makefile, CMake, or IDE project):
 * - Dcm.c
 * - Dcm_Uds_Services.c
 * - Dcm_Uds_RoutineControl.c
 * - Dcm_Uds_ECUReset.c
 * - Dcm_Uds_CommunicationControl.c
 * - Dcm_Uds_ControlDTCSetting.c
 * - Any dependency modules (Flash_Manager, Can_Manager, Adc_Manager, etc.)
 *
 */

/*
 * ============================================================================
 * STEP 3: IMPLEMENT DEPENDENCY MODULES
 * ============================================================================
 * 
 * The following modules need to be implemented/configured:
 * 
 * 1. Flash Management
 *    - Flash_Manager_Erase()
 *    - Flash_Manager_Read()
 *    - Flash_Manager_Write()
 *    - Flash_Manager_SetAppValid()
 *
 * 2. NVRAM Management
 *    - Nvram_Manager_Read()
 *    - Nvram_Manager_Write()
 *
 * 3. CAN Communication
 *    - Can_Manager_LoopbackTest()
 *
 * 4. Sensor Input
 *    - Adc_Manager_GetVoltage()
 *    - Adc_Manager_GetTemperature()
 *    - Adc_Manager_ReadChannel()
 *
 * 5. Actuators
 *    - Led_Manager_Test()
 *    - Motor_Manager_Test()
 *
 * 6. CRC Calculation
 *    - Crc_CalculateCRC32()
 *
 */

/*
 * ============================================================================
 * STEP 4: INITIALIZE IN APPLICATION
 * ============================================================================
 * 
 * In your main application initialization:
 *
 * void ECU_Initialize(void)
 * {
 *     // Initialize all BSW modules
 *     Dcm_Init();          // DCM (Diagnostic Communication Manager)
 *     Can_Init();          // CAN interface
 *     Adc_Init();          // ADC
 *     Flash_Init();        // Flash manager
 *     Nvram_Init();        // NVRAM manager
 *     
 *     // Configure DID table
 *     // Configure Routine table
 * }
 *
 */

/*
 * ============================================================================
 * STEP 5: CREATE DCM MAIN FUNCTION
 * ============================================================================
 * 
 * Call in your main task (10ms or 20ms cycle):
 *
 * void Dcm_MainFunction(void)
 * {
 *     // Handle incoming UDS requests from CAN
 *     if (CAN_MessageReceived(DCM_CAN_REQUEST_ID)) {
 *         Dcm_MsgType requestMsg;
 *         uint16 requestLength = CAN_GetMessage(&requestMsg);
 *         
 *         // Process UDS service
 *         Dcm_ProcessRequest(&requestMsg);
 *         
 *         // Send response
 *         CAN_SendMessage(DCM_CAN_RESPONSE_ID, &responseMsg, responseLength);
 *     }
 * }
 *
 */

/*
 * ============================================================================
 * STEP 6: CONFIGURE CAN DATABASE
 * ============================================================================
 * 
 * From the requirements document (CanMatrix sheet):
 * 
 * Diagnostic Request:  ID=0x7A0, 8 bytes (event-based)
 * Diagnostic Response: ID=0x7A8, 8 bytes (event-based)
 * Functional Request:  ID=0x7DF, 8 bytes (event-based)
 *
 */

/*
 * ============================================================================
 * STEP 7: TESTING
 * ============================================================================
 * 
 * Test each service using a UDS tester tool:
 * 
 * 1. Read DID (0x22):
 *    Request:  [22 F1 90]
 *    Expected: [62 F1 90 <17 bytes VIN>]
 *
 * 2. Write DID (0x2E):
 *    Request:  [2E F1 90 <17 bytes new VIN>]
 *    Expected: [6E F1 90]
 *
 * 3. Security Access (0x27):
 *    Request:  [27 01]  (requestSeed)
 *    Expected: [67 01 <4 bytes seed>]
 *
 * 4. Routine Control (0x31):
 *    Request:  [31 01 02 02]  (Start routine 0x0202)
 *    Expected: [71 01 02 02 <result>]
 *
 */

/*
 * ============================================================================
 * MEMORY LAYOUT EXAMPLE (STM32F407VET6)
 * ============================================================================
 * 
 * Flash Layout:
 * 0x08000000 - 0x0800FFFF:  Bootloader     (64 KB)
 * 0x08010000 - 0x080FFFFF:  Application    (960 KB)
 * 0x08100000 - 0x08101FFF:  Configuration  (8 KB)  <- DIDs stored here
 * 0x08102000 - 0x080FFFFF:  (Free space)
 *
 */

/*
 * ============================================================================
 * KEY AUTOSAR CONCEPTS
 * ============================================================================
 * 
 * 1. FUNC Macro: Specifies return type and section
 *    FUNC(return_type, memory_section)
 *
 * 2. Pointer Types:
 *    CONSTP2CONST: Pointer to constant (input parameters)
 *    CONSTP2VAR:   Pointer to variable (output parameters)
 *
 * 3. Return Values:
 *    E_OK:     Success (0x00)
 *    E_NOT_OK: Failure (0x01)
 *
 * 4. Naming Convention:
 *    Services:  Dcm_Service_<Name>_0x<ServiceID>
 *    Readers:   Dcm_RequestRead<Type>
 *    Writers:   Dcm_RequestWrite<Type>
 *    Routines:  Dcm_RequestRoutine<Operation>_<RID>
 *
 */

#ifdef __cplusplus
}
#endif

#endif /* DCM_UDS_CONFIG_H */
