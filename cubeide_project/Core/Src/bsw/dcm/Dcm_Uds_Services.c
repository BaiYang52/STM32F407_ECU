/**
 * @file Dcm_Uds_Services.c
 * @brief UDS Diagnostic Services Implementation - AUTOSAR Style
 * @author STM32 ECU Demo
 * @date 2024
 * 
 * Implementation of UDS services following AUTOSAR DCM module standards.
 */

/*******************************************************************************
 * INCLUDES
 *******************************************************************************/
#include "Std_Types.h"
#include <bsw/dcm/Dcm.h>
#include <bsw/dcm/Dcm_Uds_Config.h>

/*******************************************************************************
 * DEFINES
 *******************************************************************************/
#define DCM_UDS_SERVICES_C_VERSION  1U
#define DCM_UDS_SERVICES_PATCH_VERSION  0U

/* Memory addresses for DIDs storage */
#define NVRAM_DID_F190_VIN_ADDR             0x0000U
#define NVRAM_DID_F18C_SERIAL_ADDR          0x0020U
#define NVRAM_DID_F183_ECU_NAME_ADDR        0x0050U
#define NVRAM_DID_F195_SW_VERSION_ADDR      0x0070U
#define NVRAM_DID_F501_FLASH_COUNTER_ADDR   0x0090U

/* Security Access Seed Length */
#define SECURITY_SEED_LENGTH                DCM_SECURITY_SEED_LENGTH
#define SECURITY_KEY_LENGTH                 DCM_SECURITY_KEY_LENGTH

/*******************************************************************************
 * LOCAL VARIABLES
 *******************************************************************************/

/* Security Access Variables */
static uint8 Dcm_SecuritySeed_Level1[SECURITY_SEED_LENGTH];
static uint8 Dcm_SecuritySeed_Level2[SECURITY_SEED_LENGTH];
static uint32 Dcm_SecurityAttemptCounter = 0U;

/*******************************************************************************
 * LOCAL FUNCTION PROTOTYPES
 *******************************************************************************/

/**
 * @brief Get DID data from NVRAM
 */
static FUNC(Std_ReturnType, DCM_CODE) Dcm_GetDidFromNvram(
    uint16 DidId,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) DidData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) DidLength_Ptr
);

/**
 * @brief Write DID data to NVRAM
 */
static FUNC(Std_ReturnType, DCM_CODE) Dcm_WriteDidToNvram(
    uint16 DidId,
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) DidData_Ptr,
    uint16 DidLength
);

/**
 * @brief Generate random seed
 */
static FUNC(void, DCM_CODE) Dcm_GenerateSecuritySeed(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) SeedBuffer_Ptr,
    uint16 SeedLength
);

/**
 * @brief Calculate security key from seed
 */
static FUNC(uint32, DCM_CODE) Dcm_CalculateSecurityKey(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) SeedBuffer_Ptr,
    uint16 SeedLength
);

/*******************************************************************************
 * PUBLIC FUNCTIONS
 *******************************************************************************/

/**
 * ============================================================================
 * SERVICE 0x22: ReadDataByIdentifier
 * ============================================================================
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_ReadDataByIdentifier_0x22(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    uint16 requestIndex = 1U;  /* Skip service ID byte */
    uint16 responseIndex = 1U; /* Skip service ID byte */
    uint16 requestLength = Dcm_Ptr->SduLength;
    uint16 didId;
    uint16 didLength;
    uint8 tempBuffer[256];
    
    /* Validate input parameters */
    if ((NULL_PTR == Dcm_Ptr) || (NULL_PTR == RespData_Ptr) || 
        (NULL_PTR == RespData_Len_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check minimum request length: SID(1) + DID(2) */
    if (requestLength < 3U) {
        *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }
    
    /* Response SID = Service ID + 0x40 */
    RespData_Ptr->Sdu[0] = 0x22U + 0x40U;  /* 0x62 */
    
    /* Process all requested DIDs */
    while (requestIndex < requestLength) {
        /* Extract DID (2 bytes, big-endian) */
        didId = (uint16)((Dcm_Ptr->Sdu[requestIndex] << 8U) | 
                         Dcm_Ptr->Sdu[requestIndex + 1U]);
        requestIndex += 2U;
        
        /* Get DID data from NVRAM or internal storage */
        retVal = Dcm_GetDidFromNvram(didId, tempBuffer, &didLength);
        
        if (E_OK != retVal) {
            *ErrorCode_Ptr = DCM_E_NO_ACCESS_TO_REQUESTED_DID;
            return E_NOT_OK;
        }
        
        /* Check response buffer size */
        if ((responseIndex + 2U + didLength) > 256U) {
            *ErrorCode_Ptr = DCM_E_RESPONSE_TOO_LONG;
            return E_NOT_OK;
        }
        
        /* Add DID and data to response */
        RespData_Ptr->Sdu[responseIndex] = (uint8)(didId >> 8U);
        RespData_Ptr->Sdu[responseIndex + 1U] = (uint8)(didId & 0xFFU);
        responseIndex += 2U;
        
        /* Copy DID data */
        for (uint16 i = 0U; i < didLength; i++) {
            RespData_Ptr->Sdu[responseIndex + i] = tempBuffer[i];
        }
        responseIndex += didLength;
    }
    
    *RespData_Len_Ptr = responseIndex;
    return E_OK;
}

/**
 * Read specific DIDs implementation
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF190_Vin(
    CONSTP2VAR(Dcm_Did_F190_VinType, AUTOMATIC, DCM_APPL_DATA) VinData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    uint16 didLength = 17U;
    
    if ((NULL_PTR == VinData_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Read VIN from NVRAM */
    retVal = Dcm_GetDidFromNvram(DID_VIN, VinData_Ptr->Vin, &didLength);
    
    if (E_OK != retVal) {
        *ErrorCode_Ptr = DCM_E_NO_ACCESS_TO_REQUESTED_DID;
        return E_NOT_OK;
    }
    
    return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF18C_Serial(
    CONSTP2VAR(Dcm_Did_F18C_SerialType, AUTOMATIC, DCM_APPL_DATA) SerialData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    uint16 didLength = 32U;
    
    if ((NULL_PTR == SerialData_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Read Serial Number from NVRAM */
    retVal = Dcm_GetDidFromNvram(DID_ECU_SERIAL_NUMBER, 
                                   SerialData_Ptr->SerialNumber, &didLength);
    
    if (E_OK != retVal) {
        *ErrorCode_Ptr = DCM_E_NO_ACCESS_TO_REQUESTED_DID;
        return E_NOT_OK;
    }
    
    return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF183_EcuName(
    CONSTP2VAR(Dcm_Did_F183_EcuNameType, AUTOMATIC, DCM_APPL_DATA) EcuNameData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    uint16 didLength = 16U;
    
    if ((NULL_PTR == EcuNameData_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Read ECU Name from NVRAM */
    retVal = Dcm_GetDidFromNvram(DID_ECU_NAME, EcuNameData_Ptr->EcuName, &didLength);
    
    if (E_OK != retVal) {
        *ErrorCode_Ptr = DCM_E_NO_ACCESS_TO_REQUESTED_DID;
        return E_NOT_OK;
    }
    
    return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF195_SwVersion(
    CONSTP2VAR(Dcm_Did_F195_SwVersionType, AUTOMATIC, DCM_APPL_DATA) SwVersionData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    uint16 didLength = 8U;
    
    if ((NULL_PTR == SwVersionData_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Read SW Version from NVRAM */
    retVal = Dcm_GetDidFromNvram(DID_SYSTEM_SUPPLIER_SW_VERSION, 
                                   SwVersionData_Ptr->SwVersion, &didLength);
    
    if (E_OK != retVal) {
        *ErrorCode_Ptr = DCM_E_NO_ACCESS_TO_REQUESTED_DID;
        return E_NOT_OK;
    }
    
    return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestReadDidF501_FlashCounter(
    CONSTP2VAR(Dcm_Did_F501_FlashCounterType, AUTOMATIC, DCM_APPL_DATA) FlashCounterData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    uint16 didLength = 2U;
    uint8 tempBuffer[2];
    
    if ((NULL_PTR == FlashCounterData_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Read Flash Counter from NVRAM */
    retVal = Dcm_GetDidFromNvram(DID_FLASH_COUNTER, tempBuffer, &didLength);
    
    if (E_OK != retVal) {
        *ErrorCode_Ptr = DCM_E_NO_ACCESS_TO_REQUESTED_DID;
        return E_NOT_OK;
    }
    
    /* Convert bytes to uint16 (big-endian) */
    FlashCounterData_Ptr->FlashCounter = (uint16)((tempBuffer[0] << 8U) | tempBuffer[1]);
    
    return E_OK;
}

/**
 * ============================================================================
 * SERVICE 0x2E: WriteDataByIdentifier
 * ============================================================================
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_WriteDataByIdentifier_0x2E(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    uint16 requestIndex = 1U;  /* Skip service ID byte */
    uint16 responseIndex = 1U; /* Skip service ID byte */
    uint16 requestLength = Dcm_Ptr->SduLength;
    uint16 didId;
    uint16 dataLength;
    
    /* Validate input parameters */
    if ((NULL_PTR == Dcm_Ptr) || (NULL_PTR == RespData_Ptr) || 
        (NULL_PTR == RespData_Len_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check minimum request length: SID(1) + DID(2) + DataLength(1) */
    if (requestLength < 4U) {
        *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }
    
    /* Check security access for write operations */
    if (!Dcm_IsSecurityLevelUnlocked(1U)) {
        *ErrorCode_Ptr = DCM_E_SECURITY_ACCESS_DENIED;
        return E_NOT_OK;
    }
    
    /* Response SID = Service ID + 0x40 */
    RespData_Ptr->Sdu[0] = 0x2EU + 0x40U;  /* 0x6E */
    
    /* Extract DID (2 bytes, big-endian) */
    didId = (uint16)((Dcm_Ptr->Sdu[requestIndex] << 8U) | 
                     Dcm_Ptr->Sdu[requestIndex + 1U]);
    requestIndex += 2U;
    
    /* Get data length */
    dataLength = requestLength - requestIndex;
    
    /* Write DID data to NVRAM */
    retVal = Dcm_WriteDidToNvram(didId, &Dcm_Ptr->Sdu[requestIndex], dataLength);
    
    if (E_OK != retVal) {
        *ErrorCode_Ptr = DCM_E_GENERAL_PROGRAMMING_FAILURE;
        return E_NOT_OK;
    }
    
    /* Response: SID + DID */
    RespData_Ptr->Sdu[1] = (uint8)(didId >> 8U);
    RespData_Ptr->Sdu[2] = (uint8)(didId & 0xFFU);
    
    *RespData_Len_Ptr = 3U;
    return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestWriteDidF190_Vin(
    CONSTP2CONST(Dcm_Did_F190_VinType, AUTOMATIC, DCM_APPL_DATA) VinData_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    
    if ((NULL_PTR == VinData_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check security access */
    if (!Dcm_IsSecurityLevelUnlocked(1U)) {
        *ErrorCode_Ptr = DCM_E_SECURITY_ACCESS_DENIED;
        return E_NOT_OK;
    }
    
    /* Write VIN to NVRAM */
    retVal = Dcm_WriteDidToNvram(DID_VIN, VinData_Ptr->Vin, 17U);
    
    if (E_OK != retVal) {
        *ErrorCode_Ptr = DCM_E_GENERAL_PROGRAMMING_FAILURE;
        return E_NOT_OK;
    }
    
    return E_OK;
}

/**
 * ============================================================================
 * SERVICE 0x27: SecurityAccess
 * ============================================================================
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_SecurityAccess_0x27(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    uint8 subFunction;
    uint8 securityLevel;
    
    /* Validate input parameters */
    if ((NULL_PTR == Dcm_Ptr) || (NULL_PTR == RespData_Ptr) || 
        (NULL_PTR == RespData_Len_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check minimum request length: SID(1) + SubFunction(1) */
    if (Dcm_Ptr->SduLength < 2U) {
        *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }
    
    /* Response SID = Service ID + 0x40 */
    RespData_Ptr->Sdu[0] = 0x27U + 0x40U;  /* 0x67 */
    
    subFunction = Dcm_Ptr->Sdu[1];
    
    /* Determine security level from subfunction */
    switch (subFunction) {
        case 0x01U: /* requestSeed Level 1 */
        case 0x02U: /* sendKey Level 1 */
            securityLevel = 1U;
            break;
        case 0x03U: /* requestSeed Level 2 */
        case 0x04U: /* sendKey Level 2 */
            securityLevel = 2U;
            break;
        case 0x05U: /* requestSeed Level 3 */
        case 0x06U: /* sendKey Level 3 */
            securityLevel = 3U;
            break;
        case 0x07U: /* requestSeed Level 4 */
        case 0x08U: /* sendKey Level 4 */
            securityLevel = 4U;
            break;
        default:
            *ErrorCode_Ptr = DCM_E_SUBFUNCTION_NOT_SUPPORTED;
            return E_NOT_OK;
    }
    
    /* Process requestSeed (odd subfunctions) */
    if ((subFunction & 0x01U) == 0x01U) {
        retVal = Dcm_RequestSeed_Level1(
            &RespData_Ptr->Sdu[2],
            (uint16 *)RespData_Len_Ptr,
            ErrorCode_Ptr
        );
        
        if (E_OK != retVal) {
            return E_NOT_OK;
        }
        
        /* Response: SID + SubFunction + Seed */
        RespData_Ptr->Sdu[1] = subFunction;
        *RespData_Len_Ptr += 2U;  /* Include SID and SubFunction */
        
    } else {  /* Process sendKey (even subfunctions) */
        
        /* Check minimum request length for sendKey: SID(1) + SubFunction(1) + Key(4) */
        if (Dcm_Ptr->SduLength < 6U) {
            *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
            return E_NOT_OK;
        }
        
        retVal = Dcm_SendKey_Level1(
            &Dcm_Ptr->Sdu[2],
            Dcm_Ptr->SduLength - 2U,
            ErrorCode_Ptr
        );
        
        if (E_OK != retVal) {
            return E_NOT_OK;
        }
        
        /* Response: SID + SubFunction */
        RespData_Ptr->Sdu[1] = subFunction;
        *RespData_Len_Ptr = 2U;
    }
    
    return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestSeed_Level1(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) SeedData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) SeedLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    if ((NULL_PTR == SeedData_Ptr) || (NULL_PTR == SeedLength_Ptr) || 
        (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Generate random seed */
    Dcm_GenerateSecuritySeed(Dcm_SecuritySeed_Level1, SECURITY_SEED_LENGTH);
    
    /* Copy seed to response */
    for (uint8 i = 0U; i < SECURITY_SEED_LENGTH; i++) {
        SeedData_Ptr[i] = Dcm_SecuritySeed_Level1[i];
    }
    
    *SeedLength_Ptr = SECURITY_SEED_LENGTH;
    
    return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) Dcm_SendKey_Level1(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) KeyData_Ptr,
    uint16 KeyLength,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    uint32 calculatedKey;
    uint32 receivedKey;
    
    if ((NULL_PTR == KeyData_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check key length */
    if (KeyLength != SECURITY_KEY_LENGTH) {
        *ErrorCode_Ptr = DCM_E_INVALID_KEY;
        return E_NOT_OK;
    }
    
    /* Calculate expected key from seed */
    calculatedKey = Dcm_CalculateSecurityKey(Dcm_SecuritySeed_Level1, 
                                              SECURITY_SEED_LENGTH);
    
    /* Extract received key (big-endian) */
    receivedKey = (uint32)((KeyData_Ptr[0] << 24U) | (KeyData_Ptr[1] << 16U) | 
                           (KeyData_Ptr[2] << 8U) | KeyData_Ptr[3]);
    
    /* Validate key */
    if (calculatedKey != receivedKey) {
        Dcm_SecurityAttemptCounter++;
        
        if (Dcm_SecurityAttemptCounter >= 3U) {
            *ErrorCode_Ptr = DCM_E_EXCEEDED_NUMBER_OF_ATTEMPTS;
            return E_NOT_OK;
        }
        
        *ErrorCode_Ptr = DCM_E_INVALID_KEY;
        return E_NOT_OK;
    }
    
    /* Unlock security level 1 */
    Dcm_SetSecurityLevel(1U, TRUE);
    Dcm_SecurityAttemptCounter = 0U;
    
    return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestSeed_Level2(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) SeedData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) SeedLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    if ((NULL_PTR == SeedData_Ptr) || (NULL_PTR == SeedLength_Ptr) || 
        (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Generate random seed for Level 2 */
    Dcm_GenerateSecuritySeed(Dcm_SecuritySeed_Level2, SECURITY_SEED_LENGTH);
    
    /* Copy seed to response */
    for (uint8 i = 0U; i < SECURITY_SEED_LENGTH; i++) {
        SeedData_Ptr[i] = Dcm_SecuritySeed_Level2[i];
    }
    
    *SeedLength_Ptr = SECURITY_SEED_LENGTH;
    
    return E_OK;
}

FUNC(Std_ReturnType, DCM_CODE) Dcm_SendKey_Level2(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) KeyData_Ptr,
    uint16 KeyLength,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    uint32 calculatedKey;
    uint32 receivedKey;
    
    if ((NULL_PTR == KeyData_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check key length */
    if (KeyLength != SECURITY_KEY_LENGTH) {
        *ErrorCode_Ptr = DCM_E_INVALID_KEY;
        return E_NOT_OK;
    }
    
    /* Calculate expected key from seed */
    calculatedKey = Dcm_CalculateSecurityKey(Dcm_SecuritySeed_Level2, 
                                              SECURITY_SEED_LENGTH);
    
    /* Extract received key (big-endian) */
    receivedKey = (uint32)((KeyData_Ptr[0] << 24U) | (KeyData_Ptr[1] << 16U) | 
                           (KeyData_Ptr[2] << 8U) | KeyData_Ptr[3]);
    
    /* Validate key */
    if (calculatedKey != receivedKey) {
        Dcm_SecurityAttemptCounter++;
        
        if (Dcm_SecurityAttemptCounter >= 3U) {
            *ErrorCode_Ptr = DCM_E_EXCEEDED_NUMBER_OF_ATTEMPTS;
            return E_NOT_OK;
        }
        
        *ErrorCode_Ptr = DCM_E_INVALID_KEY;
        return E_NOT_OK;
    }
    
    /* Unlock security level 2 */
    Dcm_SetSecurityLevel(2U, TRUE);
    Dcm_SecurityAttemptCounter = 0U;
    
    return E_OK;
}

/**
 * ============================================================================
 * LOCAL FUNCTIONS
 * ============================================================================
 */

static FUNC(Std_ReturnType, DCM_CODE) Dcm_GetDidFromNvram(
    uint16 DidId,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) DidData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) DidLength_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    
    if ((NULL_PTR == DidData_Ptr) || (NULL_PTR == DidLength_Ptr)) {
        return E_NOT_OK;
    }
    
    switch (DidId) {
        case DID_VIN:
            /* Default VIN: "1G1YY12V347175635" */
            // Nvram_Manager_Read(NVRAM_DID_F190_VIN_ADDR, DidData_Ptr, 17U);
            *DidLength_Ptr = 17U;
            break;
            
        case DID_ECU_SERIAL_NUMBER:
            /* Serial number: 32 bytes */
            // Nvram_Manager_Read(NVRAM_DID_F18C_SERIAL_ADDR, DidData_Ptr, 32U);
            *DidLength_Ptr = 32U;
            break;
            
        case DID_ECU_NAME:
            /* ECU Name: "STM32F407VET6    " (16 bytes) */
            // Nvram_Manager_Read(NVRAM_DID_F183_ECU_NAME_ADDR, DidData_Ptr, 16U);
            *DidLength_Ptr = 16U;
            break;
            
        case DID_SYSTEM_SUPPLIER_SW_VERSION:
            /* SW Version: "SW-V1.1.0   " (8 bytes) */
            // Nvram_Manager_Read(NVRAM_DID_F195_SW_VERSION_ADDR, DidData_Ptr, 8U);
            *DidLength_Ptr = 8U;
            break;
            
        case DID_FLASH_COUNTER:
            /* Flash Counter: 2 bytes */
            // Nvram_Manager_Read(NVRAM_DID_F501_FLASH_COUNTER_ADDR, DidData_Ptr, 2U);
            *DidLength_Ptr = 2U;
            break;
            
        case DID_SYSTEM_SUPPLIER_ID:
            /* Supplier ID: "ST " (3 bytes) */
            DidData_Ptr[0] = 'S';
            DidData_Ptr[1] = 'T';
            DidData_Ptr[2] = ' ';
            *DidLength_Ptr = 3U;
            break;
            
        case DID_ACTIVE_DIAGNOSTIC_SESSION:
            /* Current diagnostic session */
            DidData_Ptr[0] = 0x01U;  /* Default session */
            *DidLength_Ptr = 1U;
            break;
            
        default:
            retVal = E_NOT_OK;
            break;
    }
    
    return retVal;
}

static FUNC(Std_ReturnType, DCM_CODE) Dcm_WriteDidToNvram(
    uint16 DidId,
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) DidData_Ptr,
    uint16 DidLength
)
{
    Std_ReturnType retVal = E_OK;
    
    if (NULL_PTR == DidData_Ptr) {
        return E_NOT_OK;
    }
    
    switch (DidId) {
        case DID_VIN:
            if (DidLength == 17U) {
                // Nvram_Manager_Write(NVRAM_DID_F190_VIN_ADDR, (uint8 *)DidData_Ptr, 17U);
            } else {
                retVal = E_NOT_OK;
            }
            break;
            
        case DID_ECU_SERIAL_NUMBER:
            if (DidLength <= 32U) {
                // Nvram_Manager_Write(NVRAM_DID_F18C_SERIAL_ADDR, (uint8 *)DidData_Ptr, DidLength);
            } else {
                retVal = E_NOT_OK;
            }
            break;
            
        case DID_FLASH_COUNTER:
            if (DidLength == 2U) {
                // Nvram_Manager_Write(NVRAM_DID_F501_FLASH_COUNTER_ADDR, (uint8 *)DidData_Ptr, 2U);
            } else {
                retVal = E_NOT_OK;
            }
            break;
            
        default:
            retVal = E_NOT_OK;
            break;
    }
    
    return retVal;
}

static FUNC(void, DCM_CODE) Dcm_GenerateSecuritySeed(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) SeedBuffer_Ptr,
    uint16 SeedLength
)
{
    /* Simple pseudo-random seed generation using system tick counter */
    /* In production, use a proper RNG */
    uint32 tickCounter = 0U;  /* Should be replaced with SysTick_Get() or similar */
    
    for (uint16 i = 0U; i < SeedLength; i++) {
        tickCounter = (tickCounter * 1103515245U + 12345U) & 0x7fffffffU;
        SeedBuffer_Ptr[i] = (uint8)(tickCounter >> (8U * (i % 4U)));
    }
}

static FUNC(uint32, DCM_CODE) Dcm_CalculateSecurityKey(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) SeedBuffer_Ptr,
    uint16 SeedLength
)
{
    /* Simple XOR-based key calculation from seed */
    /* In production, use a proper cryptographic algorithm */
    uint32 key = 0U;
    
    for (uint16 i = 0U; i < SeedLength; i++) {
        key ^= (uint32)(SeedBuffer_Ptr[i] << (8U * (i % 4U)));
    }
    
    /* Add constant for obfuscation */
    key = key ^ 0x12345678U;
    
    return key;
}

/* End of file */
