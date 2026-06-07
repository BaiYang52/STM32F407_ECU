/**
 * @file Dcm_Uds_ControlDTCSetting.c
 * @brief UDS Control DTC Setting Service (0x85) Implementation - AUTOSAR Style
 * @version 1.0.0
 *
 * UDS Service 0x85: ControlDTCSetting
 * 实现 DTC 设置控制功能, 遵循 ISO 14229-1 规范。
 *
 * 根据需求文档, 支持:
 *   SubFunction 0x01: ON (打开 DTC 设置) - 支持
 *   SubFunction 0x02: OFF (关闭 DTC 设置) - 支持
 *
 * 功能说明:
 *   - 控制 ECU 是否存储/更新诊断故障码 (DTC)
 *   - DTC OFF 时, ECU 不记录新的 DTC, 但已有 DTC 不会清除
 *   - DTC ON 时, 恢复正常 DTC 记录功能
 *
 * 会话支持 (来自需求文档):
 *   Default Session: 支持
 *   Extended Session: 支持
 *   Programming Session: 不支持
 *
 * DTC Setting 类型 (DTCSettingType):
 *   Byte 2 (可选):
 *     0x01: DTC_SETTING_ON  (仅此 DTC 设置类型)
 *     0x02: DTC_SETTING_OFF (仅此 DTC 设置类型)
 *     0x03-0x40: 保留 (ISO 14229-1)
 *     0x41-0x5F: 车辆制造商自定义
 *     0x61-0x7E: 系统供应商自定义
 *
 * DTC Setting 控制选项记录 (DTCSettingControlOptionRecord):
 *   Byte 3-N: 可选, 用于指定哪些 DTC 或 DTC 组
 *   此处简化: 控制全部 DTC
 *
 * NRC:
 *   0x12: 子功能不支持
 *   0x13: 消息长度错误
 *   0x22: 条件不满足 (Voltage < 9V, Voltage > 16V, VehicleSpeed > 5km/h)
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
#define DCM_CONTROL_DTC_C_VERSION           1U
#define DCM_CONTROL_DTC_PATCH_VERSION       0U

/* 子功能定义 */
#define DTC_SETTING_ON                      0x01U
#define DTC_SETTING_OFF                     0x02U

/* 肯定响应 SID */
#define CONTROL_DTC_POSITIVE_RESPONSE_SID   0xC5U   /* 0x85 + 0x40 */

/* NRC 引用 */
extern uint8 Dcm_Global_NegativeResponseCode;

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATION
 *******************************************************************************/

/**
 * ============================================================================
 * SERVICE 0x85: ControlDTCSetting (控制 DTC 设置)
 * ============================================================================
 *
 * UDS 请求格式:
 *   Byte 0: 0x85 (SID)
 *   Byte 1: SubFunction
 *            0x01: ON (打开 DTC 设置)
 *            0x02: OFF (关闭 DTC 设置)
 *   Byte 2: DTCSettingType (可选)
 *            Bit 0-6: DTC 设置类型
 *            Bit 7: 保留
 *   Byte 3-N: DTCSettingControlOptionRecord (可选)
 *
 * UDS 肯定响应格式:
 *   Byte 0: 0xC5 (SID + 0x40)
 *   Byte 1: SubFunction (回显)
 *
 * 会话要求:
 *   Default Session: 支持
 *   Extended Session: 支持
 *   Programming Session: 不支持
 *
 * NRC:
 *   0x12: 子功能不支持
 *   0x13: 消息长度错误
 *   0x22: 条件不满足
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_ControlDTCSetting_0x85(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) RequestData,
    uint16 RequestLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) ResponseData,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) ResponseLength)
{
    uint8 subFunction;
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
     * 提取子功能
     * ======================================================================== */
    // subFunction = RequestData[1];
    subFunction = RequestData[1] & 0x7FU;
    /* ========================================================================
     * 会话校验
     *
     * 根据需求文档:
     *   Default Session: 支持
     *   Extended Session: 支持
     *   Programming Session: 不支持
     * ======================================================================== */
    currentSession = Dcm_GetCurrentSession();

    if (currentSession == DCM_SESSION_PROGRAMMING) { /* Service not support in fbl */
        Dcm_Global_NegativeResponseCode = DCM_E_SERVICE_NOT_SUPPORTED;
        return E_NOT_OK;
    }else if (currentSession == DCM_SESSION_DEFAULT) { /* Service not support in Default session */
        Dcm_Global_NegativeResponseCode = DCM_E_SERVICE_NOT_SUPPORT_IN_CURRENT_SESSION;
        return E_NOT_OK;
    }else { /*do nothing */ }

    /* ========================================================================
     * 消息长度校验: SID(1) + SubFunction(1) = 2 最小
     * 可选: DTCSettingType(1) + DTCSettingControlOptionRecord(1-N)
     * ======================================================================== */
    if (RequestLength < 2U) {
        /* NRC 0x13: Incorrect Message Length or Format */
        Dcm_Global_NegativeResponseCode = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }

    /* ========================================================================
     * 校验子功能支持
     *
     * 根据需求文档:
     *   0x01 ON: 支持
     *   0x02 OFF: 支持
     * ======================================================================== */
    switch (subFunction) {
    case DTC_SETTING_ON:
        /* 打开 DTC 设置: 允许 DTC 记录和更新 */
        Dcm_SetDTCSettingEnabled(TRUE);
        break;

    case DTC_SETTING_OFF:
        /* 关闭 DTC 设置: 禁止 DTC 记录和更新
         * 注意: 已存储的 DTC 不会被清除, 仅停止记录新 DTC */
        Dcm_SetDTCSettingEnabled(FALSE);
        break;

    default:
        /* NRC 0x12: SubFunction Not Supported */
        Dcm_Global_NegativeResponseCode = DCM_E_SUBFUNCTION_NOT_SUPPORTED;
        return E_NOT_OK;
    }

    if (RequestLength > 2U) {
        /* NRC 0x13: Incorrect Message Length or Format */
        Dcm_Global_NegativeResponseCode = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }

    Dcm_CheckIfSuppressPositiveResponse(RequestData[1]);
    /* ========================================================================
     * 构建肯定响应
     *
     * 格式: [0xC5] [SubFunction]
     * ======================================================================== */
    ResponseData[0] = CONTROL_DTC_POSITIVE_RESPONSE_SID;   /* 0xC5 */
    ResponseData[1] = subFunction;                          /* 0x01 或 0x02 */
    *ResponseLength = 2U;

    return E_OK;
}

/* End of file */
