/**
 * @file Dcm_Uds_RoutineControl.c
 * @brief UDS Routine Control Service (0x31) Implementation - AUTOSAR Style
 * @author STM32 ECU Demo
 * @date 2024
 * 
 * Implementation of UDS Routine Control service (0x31) following AUTOSAR DCM module standards.
 * Supports routine execution for programming, testing, and diagnostic operations.
 */

/*******************************************************************************
 * INCLUDES
 *******************************************************************************/
#include "Std_Types.h"
//#include "Dcm.h"
#include "E:\Project\Github\STM32F407_ECU\cubeide_project\Core\Inc\bsw\dcm\Dcm.h"
//#include "Flash_Manager.h"
//#include "Can_Manager.h"
//#include "Led_Manager.h"
//#include "Motor_Manager.h"
//#include "Adc_Manager.h"
//#include "Mem.h"
//#include "Crc.h"

/*******************************************************************************
 * DEFINES
 *******************************************************************************/
#define ROUTINE_CONTROL_MAX_INPUT_LENGTH        256U
#define ROUTINE_CONTROL_MAX_OUTPUT_LENGTH       256U

#define FLASH_ERASE_SUCCESS                     0x00U
#define FLASH_ERASE_FAILED                      0x01U

#define PRECONDITION_SUCCESS                    0x00U
#define PRECONDITION_VOLTAGE_LOW                0x01U
#define PRECONDITION_VOLTAGE_HIGH               0x02U
#define PRECONDITION_WRONG_SESSION              0x03U
#define PRECONDITION_TEMP_HIGH                  0x04U

#define CRC_VERIFY_SUCCESS                      0x00U
#define CRC_VERIFY_FAILED                       0x01U

#define CAN_TEST_SUCCESS                        0x00U
#define CAN_TEST_TIMEOUT                        0x01U
#define CAN_TEST_FRAME_ERROR                    0x02U

/* Programming address ranges */
#define APP_START_ADDRESS                       0x08010000U
#define APP_END_ADDRESS                         0x080FFFFFU
#define APP_SIZE                                (APP_END_ADDRESS - APP_START_ADDRESS + 1U)

/*******************************************************************************
 * PUBLIC FUNCTIONS - SERVICE 0x31: ROUTINE CONTROL
 *******************************************************************************/

FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_RoutineControl_0x31(
    CONSTP2CONST(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) Dcm_Ptr,
    CONSTP2VAR(Dcm_MsgType, AUTOMATIC, DCM_APPL_DATA) RespData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) RespData_Len_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType retVal = E_OK;
    uint16 routineId;
    uint8 subFunction;
    uint16 outputLength = 0U;
    uint8 outputBuffer[ROUTINE_CONTROL_MAX_OUTPUT_LENGTH];
    
    /* Validate input parameters */
    if ((NULL_PTR == Dcm_Ptr) || (NULL_PTR == RespData_Ptr) || 
        (NULL_PTR == RespData_Len_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check minimum request length: SID(1) + SubFunction(1) + RID(2) */
    if (Dcm_Ptr->SduLength < 4U) {
        *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }
    
    /* Extract SubFunction and Routine ID */
    subFunction = Dcm_Ptr->Sdu[1];
    routineId = (uint16)((Dcm_Ptr->Sdu[2] << 8U) | Dcm_Ptr->Sdu[3]);
    
    /* Response SID = Service ID + 0x40 */
    RespData_Ptr->Sdu[0] = 0x31U + 0x40U;  /* 0x71 */
    RespData_Ptr->Sdu[1] = subFunction;
    RespData_Ptr->Sdu[2] = (uint8)(routineId >> 8U);
    RespData_Ptr->Sdu[3] = (uint8)(routineId & 0xFFU);
    
    /* Process routine based on RID */
    switch (routineId) {
        case RID_ERASE_MEMORY:
            retVal = Dcm_RequestRoutineStart_EraseMemory_0x0201(
                &Dcm_Ptr->Sdu[4],
                Dcm_Ptr->SduLength - 4U,
                outputBuffer,
                &outputLength,
                ErrorCode_Ptr
            );
            break;
            
        case RID_CHECK_PROGRAMMING_PRECONDITION:
            retVal = Dcm_RequestRoutineStart_CheckPrecondition_0x0202(
                outputBuffer,
                &outputLength,
                ErrorCode_Ptr
            );
            break;
            
        case RID_CHECK_APP_INTEGRITY:
            retVal = Dcm_RequestRoutineStart_CheckIntegrity_0x0203(
                &Dcm_Ptr->Sdu[4],
                Dcm_Ptr->SduLength - 4U,
                outputBuffer,
                &outputLength,
                ErrorCode_Ptr
            );
            break;
            
        case RID_CAN_BUS_TEST:
            retVal = Dcm_RequestRoutineStart_CANBusTest_0x0303(
                &Dcm_Ptr->Sdu[4],
                Dcm_Ptr->SduLength - 4U,
                outputBuffer,
                &outputLength,
                ErrorCode_Ptr
            );
            break;
            
        default:
            *ErrorCode_Ptr = DCM_E_SUBFUNCTION_NOT_SUPPORTED;
            return E_NOT_OK;
    }
    
    if (E_OK != retVal) {
        return E_NOT_OK;
    }
    
    /* Copy output data to response */
    for (uint16 i = 0U; i < outputLength; i++) {
        RespData_Ptr->Sdu[4U + i] = outputBuffer[i];
    }
    
    *RespData_Len_Ptr = 4U + outputLength;
    return E_OK;
}

/**
 * ============================================================================
 * ROUTINE 0x0201: ERASE MEMORY
 * ============================================================================
 * 
 * Input:  Memory Address (4 bytes) + Size (4 bytes)
 * Output: Erase Result (1 byte)
 *         0x00 = Success
 *         0x01 = Failed
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_EraseMemory_0x0201(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    uint32 memAddress;
    uint32 memSize;
    Std_ReturnType flashResult;
    
    if ((NULL_PTR == InputData_Ptr) || (NULL_PTR == OutputData_Ptr) || 
        (NULL_PTR == OutputLength_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check input length: Address(4) + Size(4) */
    if (InputLength < 8U) {
        *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }
    
    /* Extract memory address (big-endian) */
    memAddress = (uint32)((InputData_Ptr[0] << 24U) | (InputData_Ptr[1] << 16U) | 
                          (InputData_Ptr[2] << 8U) | InputData_Ptr[3]);
    
    /* Extract memory size (big-endian) */
    memSize = (uint32)((InputData_Ptr[4] << 24U) | (InputData_Ptr[5] << 16U) | 
                       (InputData_Ptr[6] << 8U) | InputData_Ptr[7]);
    
    /* Validate memory address and size */
    if ((memAddress < APP_START_ADDRESS) || 
        ((memAddress + memSize) > APP_END_ADDRESS)) {
        *ErrorCode_Ptr = DCM_E_REQUEST_OUT_OF_RANGE;
        OutputData_Ptr[0] = FLASH_ERASE_FAILED;
        *OutputLength_Ptr = 1U;
        return E_NOT_OK;
    }
    
    /* Perform flash erase */
    flashResult = Flash_Manager_Erase(memAddress, memSize);
    
    if (E_OK == flashResult) {
        OutputData_Ptr[0] = FLASH_ERASE_SUCCESS;
    } else {
        OutputData_Ptr[0] = FLASH_ERASE_FAILED;
        *ErrorCode_Ptr = DCM_E_GENERAL_PROGRAMMING_FAILURE;
    }
    
    *OutputLength_Ptr = 1U;
    return flashResult;
}

/**
 * ============================================================================
 * ROUTINE 0x0202: CHECK PROGRAMMING PRECONDITION
 * ============================================================================
 * 
 * Input:  None
 * Output: Condition Result (1 byte)
 *         0x00 = All conditions met
 *         0x01 = Voltage low
 *         0x02 = Voltage high
 *         0x03 = Wrong session
 *         0x04 = Temperature high
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_CheckPrecondition_0x0202(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    uint16 voltage;
    sint8 temperature;
    uint8 conditionResult = PRECONDITION_SUCCESS;
    
    if ((NULL_PTR == OutputData_Ptr) || (NULL_PTR == OutputLength_Ptr) || 
        (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check system voltage: Requirement 9V - 16V */
    voltage = Adc_Manager_GetVoltage();
    
    if (voltage < 90U) {  /* 90 * 0.1V = 9V */
        conditionResult = PRECONDITION_VOLTAGE_LOW;
    } else if (voltage > 160U) {  /* 160 * 0.1V = 16V */
        conditionResult = PRECONDITION_VOLTAGE_HIGH;
    }
    
    /* Check ECU temperature */
    temperature = Adc_Manager_GetTemperature();
    
    if (temperature > 80U) {  /* 80°C threshold */
        conditionResult = PRECONDITION_TEMP_HIGH;
    }
    
    /* Check current session (should be Programming Session 0x02) */
    /* This should be checked in Dcm session manager */
    /* if (Dcm_CurrentSession != DCM_PROGRAMMING_SESSION) {
     *     conditionResult = PRECONDITION_WRONG_SESSION;
     * }
     */
    
    OutputData_Ptr[0] = conditionResult;
    *OutputLength_Ptr = 1U;
    
    return E_OK;
}

/**
 * ============================================================================
 * ROUTINE 0x0203: CHECK APPLICATION INTEGRITY
 * ============================================================================
 * 
 * Input:  Expected CRC32 (4 bytes)
 * Output: Verify Result (1 byte)
 *         0x00 = CRC verified successfully
 *         0x01 = CRC verification failed
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_CheckIntegrity_0x0203(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    uint32 expectedCrc;
    uint32 calculatedCrc;
    
    if ((NULL_PTR == InputData_Ptr) || (NULL_PTR == OutputData_Ptr) || 
        (NULL_PTR == OutputLength_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check input length: CRC32 (4 bytes) */
    if (InputLength < 4U) {
        *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }
    
    /* Extract expected CRC (big-endian) */
    expectedCrc = (uint32)((InputData_Ptr[0] << 24U) | (InputData_Ptr[1] << 16U) | 
                           (InputData_Ptr[2] << 8U) | InputData_Ptr[3]);
    
    /* Calculate CRC32 of application area */
    calculatedCrc = Crc_CalculateCRC32((uint8 *)APP_START_ADDRESS, APP_SIZE);
    
    /* Compare CRC values */
    if (expectedCrc == calculatedCrc) {
        OutputData_Ptr[0] = CRC_VERIFY_SUCCESS;
    } else {
        OutputData_Ptr[0] = CRC_VERIFY_FAILED;
        *ErrorCode_Ptr = DCM_E_GENERAL_PROGRAMMING_FAILURE;
    }
    
    *OutputLength_Ptr = 1U;
    return E_OK;
}

/**
 * ============================================================================
 * ROUTINE 0x0205: ACTIVATE APPLICATION
 * ============================================================================
 * 
 * Input:  None
 * Output: Activation Result (1 byte)
 *         0x00 = Application activated
 *         0x01 = Activation failed
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_ActivateApplication_0x0205(
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    Std_ReturnType flashResult;
    
    if ((NULL_PTR == OutputData_Ptr) || (NULL_PTR == OutputLength_Ptr) || 
        (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Set application valid flag in flash */
    flashResult = Flash_Manager_SetAppValid();
    
    if (E_OK == flashResult) {
        OutputData_Ptr[0] = 0x00U;  /* Success */
    } else {
        OutputData_Ptr[0] = 0x01U;  /* Failed */
        *ErrorCode_Ptr = DCM_E_GENERAL_PROGRAMMING_FAILURE;
    }
    
    *OutputLength_Ptr = 1U;
    return flashResult;
}

/**
 * ============================================================================
 * ROUTINE 0x0303: CAN BUS TEST
 * ============================================================================
 * 
 * Input:  CAN Channel (1 byte)
 * Output: Test Result (1 byte)
 *         0x00 = Test passed
 *         0x01 = Timeout
 *         0x02 = Frame error
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_CANBusTest_0x0303(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    uint8 canChannel;
    Std_ReturnType canResult;
    
    if ((NULL_PTR == InputData_Ptr) || (NULL_PTR == OutputData_Ptr) || 
        (NULL_PTR == OutputLength_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check input length: Channel (1 byte) */
    if (InputLength < 1U) {
        *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }
    
    /* Extract CAN channel */
    canChannel = InputData_Ptr[0];
    
    /* Validate CAN channel (0 = CAN1, 1 = CAN2) */
    if (canChannel > 1U) {
        *ErrorCode_Ptr = DCM_E_REQUEST_OUT_OF_RANGE;
        OutputData_Ptr[0] = CAN_TEST_FRAME_ERROR;
        *OutputLength_Ptr = 1U;
        return E_NOT_OK;
    }
    
    /* Perform CAN bus self-test */
    canResult = Can_Manager_LoopbackTest(canChannel);
    
    if (E_OK == canResult) {
        OutputData_Ptr[0] = CAN_TEST_SUCCESS;
    } else {
        OutputData_Ptr[0] = CAN_TEST_TIMEOUT;
        *ErrorCode_Ptr = DCM_E_GENERAL_PROGRAMMING_FAILURE;
    }
    
    *OutputLength_Ptr = 1U;
    return canResult;
}

/**
 * ============================================================================
 * ROUTINE 0x0304: LED TEST
 * ============================================================================
 * 
 * Input:  LED ID (1 byte)
 * Output: Test Result (1 byte)
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_LEDTest_0x0304(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    uint8 ledId;
    Std_ReturnType ledResult;
    
    if ((NULL_PTR == InputData_Ptr) || (NULL_PTR == OutputData_Ptr) || 
        (NULL_PTR == OutputLength_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check input length: LED ID (1 byte) */
    if (InputLength < 1U) {
        *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }
    
    ledId = InputData_Ptr[0];
    
    /* Perform LED test: blink for 500ms */
    ledResult = Led_Manager_Test(ledId);
    
    if (E_OK == ledResult) {
        OutputData_Ptr[0] = 0x00U;  /* Test passed */
    } else {
        OutputData_Ptr[0] = 0x01U;  /* Test failed */
        *ErrorCode_Ptr = DCM_E_GENERAL_PROGRAMMING_FAILURE;
    }
    
    *OutputLength_Ptr = 1U;
    return ledResult;
}

/**
 * ============================================================================
 * ROUTINE 0x0305: MOTOR TEST
 * ============================================================================
 * 
 * Input:  PWM Duty (1 byte)
 * Output: Motor Status (1 byte)
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_MotorTest_0x0305(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    uint8 pwmDuty;
    Std_ReturnType motorResult;
    
    if ((NULL_PTR == InputData_Ptr) || (NULL_PTR == OutputData_Ptr) || 
        (NULL_PTR == OutputLength_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check input length: PWM Duty (1 byte) */
    if (InputLength < 1U) {
        *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }
    
    pwmDuty = InputData_Ptr[0];
    
    /* Validate PWM duty (0-100%) */
    if (pwmDuty > 100U) {
        *ErrorCode_Ptr = DCM_E_REQUEST_OUT_OF_RANGE;
        return E_NOT_OK;
    }
    
    /* Start motor test with specified duty cycle */
    motorResult = Motor_Manager_Test(pwmDuty);
    
    if (E_OK == motorResult) {
        OutputData_Ptr[0] = 0x00U;  /* Motor running */
    } else {
        OutputData_Ptr[0] = 0x01U;  /* Motor failed */
        *ErrorCode_Ptr = DCM_E_GENERAL_PROGRAMMING_FAILURE;
    }
    
    *OutputLength_Ptr = 1U;
    return motorResult;
}

/**
 * ============================================================================
 * ROUTINE 0x0308: ADC INPUT TEST
 * ============================================================================
 * 
 * Input:  ADC Channel (1 byte)
 * Output: ADC Value (2 bytes)
 */

FUNC(Std_ReturnType, DCM_CODE) Dcm_RequestRoutineStart_ADCTest_0x0308(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) InputData_Ptr,
    uint16 InputLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) OutputData_Ptr,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) OutputLength_Ptr,
    CONSTP2VAR(Dcm_NegativeResponseCodeType, AUTOMATIC, DCM_APPL_DATA) ErrorCode_Ptr
)
{
    uint8 adcChannel;
    uint16 adcValue;
    Std_ReturnType adcResult;
    
    if ((NULL_PTR == InputData_Ptr) || (NULL_PTR == OutputData_Ptr) || 
        (NULL_PTR == OutputLength_Ptr) || (NULL_PTR == ErrorCode_Ptr)) {
        return E_NOT_OK;
    }
    
    /* Check input length: ADC Channel (1 byte) */
    if (InputLength < 1U) {
        *ErrorCode_Ptr = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }
    
    adcChannel = InputData_Ptr[0];
    
    /* Read ADC value */
    adcResult = Adc_Manager_ReadChannel(adcChannel, &adcValue);
    
    if (E_OK == adcResult) {
        /* Return ADC value as 2 bytes (big-endian) */
        OutputData_Ptr[0] = (uint8)(adcValue >> 8U);
        OutputData_Ptr[1] = (uint8)(adcValue & 0xFFU);
        *OutputLength_Ptr = 2U;
    } else {
        *ErrorCode_Ptr = DCM_E_GENERAL_PROGRAMMING_FAILURE;
        return E_NOT_OK;
    }
    
    return E_OK;
}

/* End of file */
