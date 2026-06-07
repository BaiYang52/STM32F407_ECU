/**
 * @file Dcm_Uds_CommunicationControl.c
 * @brief UDS Communication Control Service (0x28) Implementation - AUTOSAR Style
 * @version 1.0.0
 *
 * UDS Service 0x28: CommunicationControl
 * 实现通信控制功能, 遵循 ISO 14229-1 规范。
 *
 * 根据需求文档, 支持:
 *   SubFunction 0x00: EnableRxAndTx (使能收发) - 支持
 *   SubFunction 0x01: EnableRxAndDisableTx (使能收禁止发) - 支持
 *   SubFunction 0x02: DisableRxAndEnableTx (禁止收使能发) - 不支持
 *   SubFunction 0x03: DisableRxAndTx (禁止收发) - 支持
 *
 * 通信类型 (communicationType):
 *   Byte 2: Bit 0-3: communicationType
 *            0x01: normalCommunicationMessages
 *            0x02: networkManagementCommunicationMessages
 *            0x03: allCommunicationMessages
 *           Bit 4-7: 保留
 *   Byte 3: nodeIdentificationNumber (高字节) - 可选
 *   Byte 4: nodeIdentificationNumber (低字节) - 可选
 *
 * 会话支持 (来自需求文档):
 *   - 仅在 Extended Session 和 Default Session 支持
 *   - Programming Session 不支持
 *
 * NRC:
 *   0x12: 子功能不支持
 *   0x13: 消息长度错误
 *   0x22: 条件不满足 (Voltage < 9V, Voltage > 16V)
 *   0x31: 请求超出范围
 */

/*******************************************************************************
 * INCLUDES
 *******************************************************************************/
#include "Std_Types.h"
#include "compiler.h"
#include "types.h"
#include "common.h"
#include <bsw/dcm/Dcm.h>
#include <bsw/dcm/Dcm_Uds_Config.h>

/*******************************************************************************
 * DEFINES
 *******************************************************************************/
#define DCM_COMM_CONTROL_C_VERSION          1U
#define DCM_COMM_CONTROL_PATCH_VERSION      0U

/* 子功能定义 */
#define COMM_CONTROL_ENABLE_RX_TX           0x00U
#define COMM_CONTROL_ENABLE_RX_DISABLE_TX   0x01U
#define COMM_CONTROL_DISABLE_RX_ENABLE_TX   0x02U
#define COMM_CONTROL_DISABLE_RX_TX          0x03U

/* 通信类型 */
#define COMM_TYPE_NORMAL                    0x01U
#define COMM_TYPE_NETWORK_MGMT              0x02U
#define COMM_TYPE_ALL                       0x03U

/* 肯定响应 SID */
#define COMM_CONTROL_POSITIVE_RESPONSE_SID  0x68U   /* 0x28 + 0x40 */

/* NRC 引用 */
extern uint8 Dcm_Global_NegativeResponseCode;

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATION
 *******************************************************************************/

/**
 * ============================================================================
 * SERVICE 0x28: CommunicationControl (通信控制)
 * ============================================================================
 *
 * UDS 请求格式:
 *   Byte 0: 0x28 (SID)
 *   Byte 1: SubFunction (controlType)
 *            0x00: EnableRxAndTx
 *            0x01: EnableRxAndDisableTx
 *            0x03: DisableRxAndTx
 *   Byte 2: communicationType
 *   Byte 3-4: nodeIdentificationNumber (可选)
 *
 * UDS 肯定响应格式:
 *   Byte 0: 0x68 (SID + 0x40)
 *   Byte 1: SubFunction (回显)
 *
 * 会话要求:
 *   Default Session: 支持
 *   Extended Session: 支持
 *   Programming Session: 不支持
 *
 * NRC:
 *   0x12: 子功能不支持 (0x02)
 *   0x13: 消息长度错误
 *   0x22: 条件不满足
 *   0x31: 请求超出范围
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_CommunicationControl_0x28(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) RequestData,
    uint16 RequestLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) ResponseData,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) ResponseLength)
{
    uint8 subFunction;
    uint8 communicationType;
    uint8 currentSession;

    /* ========================================================================
     * 参数校验
     * ======================================================================== */
    if ((RequestData == NULL_PTR) || (ResponseData == NULL_PTR) ||
        (ResponseLength == NULL_PTR)) {
        Dcm_Global_NegativeResponseCode = DCM_E_GENERAL_REJECT;
        return E_NOT_OK;
    }

    /* ========================================================================
     * 消息长度校验: SID(1) + SubFunction(1) + CommunicationType(1) = 3 最小
     * ======================================================================== */
    if (RequestLength < 3U) {
        /* NRC 0x13: Incorrect Message Length or Format */
        Dcm_Global_NegativeResponseCode = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }

    /* ========================================================================
     * 提取子功能和通信类型
     * ======================================================================== */
    subFunction       = RequestData[1];
    communicationType = RequestData[2];

    /* ========================================================================
     * 会话校验
     *
     * 根据需求文档:
     *   Default Session:  支持 0x00, 0x01, 0x03
     *   Extended Session: 支持 0x00, 0x01, 0x03
     *   Programming Session: 不支持任何子功能
     * ======================================================================== */
    currentSession = Dcm_GetCurrentSession();

    if (currentSession == DCM_SESSION_PROGRAMMING) {
        /* Programming 会话不支持通信控制 */
        Dcm_Global_NegativeResponseCode = DCM_E_CONDITIONS_NOT_CORRECT;
        return E_NOT_OK;
    }

    /* ========================================================================
     * 校验子功能支持
     *
     * 根据需求文档:
     *   0x00 EnableRxAndTx: 支持
     *   0x01 EnableRxAndDisableTx: 支持
     *   0x02 DisableRxAndEnableTx: 不支持
     *   0x03 DisableRxAndTx: 支持
     * ======================================================================== */
    switch (subFunction) {
    case COMM_CONTROL_ENABLE_RX_TX:
    case COMM_CONTROL_ENABLE_RX_DISABLE_TX:
    case COMM_CONTROL_DISABLE_RX_TX:
        /* 支持: 继续处理 */
        break;

    case COMM_CONTROL_DISABLE_RX_ENABLE_TX:
    default:
        /* NRC 0x12: SubFunction Not Supported */
        Dcm_Global_NegativeResponseCode = DCM_E_SUBFUNCTION_NOT_SUPPORTED;
        return E_NOT_OK;
    }

    /* ========================================================================
     * 校验通信类型
     *
     * 根据 ISO 14229-1:
     *   0x01: normalCommunicationMessages (常规应用报文)
     *   0x02: networkManagementMessages (网络管理报文)
     *   0x03: allMessages (全部报文)
     *
     * 此处简化: 支持全部通信类型, 统一处理
     * ======================================================================== */
    switch (communicationType) {
    case COMM_TYPE_NORMAL:
    case COMM_TYPE_NETWORK_MGMT:
    case COMM_TYPE_ALL:
        /* 支持 */
        break;
    default:
        /* NRC 0x31: Request Out Of Range */
        Dcm_Global_NegativeResponseCode = DCM_E_REQUEST_OUT_OF_RANGE;
        return E_NOT_OK;
    }

    /* ========================================================================
     * 校验条件 (根据需求文档 NRC 0x22 检测条件):
     *   1) Voltage < 9V
     *   2) Voltage > 16V
     *
     * 此处简化实现: 暂不调用 ADC, 由应用层决定
     * ======================================================================== */

    /* ========================================================================
     * 执行通信控制
     *
     * 根据子功能设置 DCM 的通信控制状态:
     *   0x00: RX=Enable, TX=Enable
     *   0x01: RX=Enable, TX=Disable
     *   0x03: RX=Disable, TX=Disable
     * ======================================================================== */
    switch (subFunction) {
    case COMM_CONTROL_ENABLE_RX_TX:
        Dcm_SetCommState(0x00U);  /* RX/TX 使能 */
        break;

    case COMM_CONTROL_ENABLE_RX_DISABLE_TX:
        Dcm_SetCommState(0x01U);  /* RX 使能, TX 禁止 */
        break;

    case COMM_CONTROL_DISABLE_RX_TX:
        Dcm_SetCommState(0x03U);  /* RX/TX 禁止 */
        break;

    default:
        /* 不应到达此处 */
        break;
    }

    /* ========================================================================
     * 构建肯定响应
     *
     * 格式: [0x68] [SubFunction]
     * ======================================================================== */
    ResponseData[0] = COMM_CONTROL_POSITIVE_RESPONSE_SID;   /* 0x68 */
    ResponseData[1] = subFunction;                           /* 回显 */
    *ResponseLength = 2U;

    return E_OK;
}

/* End of file */
