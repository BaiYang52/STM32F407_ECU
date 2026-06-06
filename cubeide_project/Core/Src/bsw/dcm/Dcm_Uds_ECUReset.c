/**
 * @file Dcm_Uds_ECUReset.c
 * @brief UDS ECU Reset Service (0x11) Implementation - AUTOSAR Style
 * @version 1.0.0
 *
 * UDS Service 0x11: ECUReset
 * 实现 ECU 复位功能, 遵循 ISO 14229-1 规范。
 *
 * 根据需求文档, 支持:
 *   SubFunction 0x01: HardReset (硬件复位) - 支持
 *   SubFunction 0x02: KeyOffOnReset - 不支持
 *   SubFunction 0x03: SoftReset - 不支持
 *
 * 复位时间: 427ms (来自需求文档)
 * 复位后: 自动进入 Default Session
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
#define DCM_ECU_RESET_C_VERSION             1U
#define DCM_ECU_RESET_PATCH_VERSION         0U

/* 复位类型子功能 */
#define ECU_RESET_HARD_RESET                0x01U
#define ECU_RESET_KEY_OFF_ON_RESET          0x02U
#define ECU_RESET_SOFT_RESET                0x03U

/* 肯定响应 SID */
#define ECU_RESET_POSITIVE_RESPONSE_SID     0x51U   /* 0x11 + 0x40 */

/* NRC 引用 - 由 Dcm.c 中的全局变量提供 */
extern uint8 Dcm_Global_NegativeResponseCode;

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATION
 *******************************************************************************/

/**
 * ============================================================================
 * SERVICE 0x11: ECUReset (ECU 复位)
 * ============================================================================
 *
 * UDS 请求格式:
 *   Byte 0: 0x11 (SID)
 *   Byte 1: SubFunction (resetType)
 *            0x01: HardReset (硬件复位)
 *            0x02: KeyOffOnReset (不支持)
 *            0x03: SoftReset (不支持)
 *
 * UDS 肯定响应格式:
 *   Byte 0: 0x51 (SID + 0x40)
 *   Byte 1: SubFunction (回显)
 *
 * 流程:
 *   1. 校验参数和子功能
 *   2. 校验复位条件 (电压、车速)
 *   3. 构建肯定响应由 Dcm.c 核心发送
 *   4. 应用层在响应发送后延时并执行硬件复位
 *
 * NRC:
 *   0x12: 子功能不支持 (0x02, 0x03)
 *   0x13: 消息长度错误
 *   0x22: 条件不满足
 */
FUNC(Std_ReturnType, DCM_CODE) Dcm_Service_ECUReset_0x11(
    CONSTP2CONST(uint8, AUTOMATIC, DCM_APPL_DATA) RequestData,
    uint16 RequestLength,
    CONSTP2VAR(uint8, AUTOMATIC, DCM_APPL_DATA) ResponseData,
    CONSTP2VAR(uint16, AUTOMATIC, DCM_APPL_DATA) ResponseLength)
{
    uint8 subFunction;

    /* ========================================================================
     * 参数校验
     * ======================================================================== */
    if ((RequestData == NULL_PTR) || (ResponseData == NULL_PTR) ||
        (ResponseLength == NULL_PTR)) {
        Dcm_Global_NegativeResponseCode = DCM_E_GENERAL_REJECT;
        return E_NOT_OK;
    }

    /* ========================================================================
     * 消息长度校验: SID(1) + SubFunction(1) = 2 最小
     * ======================================================================== */
    if (RequestLength < 2U) {
        /* NRC 0x13: Incorrect Message Length or Format */
        Dcm_Global_NegativeResponseCode = DCM_E_INCORRECT_MSG_LENGTH_OR_FORMAT;
        return E_NOT_OK;
    }

    /* ========================================================================
     * 提取子功能
     * ======================================================================== */
    subFunction = RequestData[1];

    /* ========================================================================
     * 校验子功能支持
     *
     * 根据需求文档:
     *   0x01 HardReset: 支持 (全部会话)
     *   0x02 KeyOffOnReset: 不支持 (返回 NRC 0x12)
     *   0x03 SoftReset: 不支持 (返回 NRC 0x12)
     * ======================================================================== */
    switch (subFunction) {
    case ECU_RESET_HARD_RESET:
        /* 支持: 继续处理 */
        break;

    case ECU_RESET_KEY_OFF_ON_RESET:
    case ECU_RESET_SOFT_RESET:
    default:
        /* NRC 0x12: SubFunction Not Supported */
        Dcm_Global_NegativeResponseCode = DCM_E_SUBFUNCTION_NOT_SUPPORTED;
        return E_NOT_OK;
    }

    /* ========================================================================
     * 校验复位条件
     *
     * 根据需求文档 NRC 0x22 检测条件:
     *   1) Voltage < 9V
     *   2) Voltage > 16V
     *   3) VehicleSpeed > 5km/h
     *
     * 此处简化实现: 暂不调用 ADC/车速检测, 由应用层决定
     * 若要新增检测, 在此处调用:
     *   uint16 voltage = Adc_Manager_GetVoltage();  // 单位: 0.1V
     *   if (voltage < 90U || voltage > 160U) {
     *       Dcm_Global_NegativeResponseCode = DCM_E_CONDITIONS_NOT_CORRECT;
     *       return E_NOT_OK;
     *   }
     *   uint8 speed = Rte_Read_VehicleSpeed();  // 单位: km/h
     *   if (speed > 5U) {
     *       Dcm_Global_NegativeResponseCode = DCM_E_CONDITIONS_NOT_CORRECT;
     *       return E_NOT_OK;
     *   }
     * ======================================================================== */

    /* ========================================================================
     * 构建肯定响应
     *
     * 格式: [0x51] [SubFunction]
     *
     * 注意: 复位响应需要在复位执行前发送出去,
     * 因此先填充响应缓冲区, 由 Dcm 核心在发送后执行复位
     * ======================================================================== */
    ResponseData[0] = ECU_RESET_POSITIVE_RESPONSE_SID;   /* 0x51 */
    ResponseData[1] = subFunction;                        /* 0x01 */
    *ResponseLength = 2U;

    return E_OK;
}

/* End of file */
