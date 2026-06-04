/**
 * @file dcm_service.h
 * @brief DCM诊断服务头文件
 * @version 1.0.0
 * @date 2024-01-01
 */

#ifndef DCM_SERVICE_H
#define DCM_SERVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

//#include "dcm_types.h"
#include "types.h"

    /* ============= UDS Service IDs ============= */

#define DCM_SERVICE_0x10 0x10U /**< DiagnosticSessionControl */
#define DCM_SERVICE_0x11 0x11U /**< ECUReset */
#define DCM_SERVICE_0x14 0x14U /**< ClearDiagnosticInformation */
#define DCM_SERVICE_0x19 0x19U /**< ReadDTCInformation */
#define DCM_SERVICE_0x22 0x22U /**< ReadDataByIdentifier */
#define DCM_SERVICE_0x2E 0x2EU /**< WriteDataByIdentifier */
#define DCM_SERVICE_0x27 0x27U /**< SecurityAccess */
#define DCM_SERVICE_0x28 0x28U /**< CommunicationControl */
#define DCM_SERVICE_0x31 0x31U /**< RoutineControl */
#define DCM_SERVICE_0x34 0x34U /**< RequestDownload */
#define DCM_SERVICE_0x36 0x36U /**< TransferData */
#define DCM_SERVICE_0x37 0x37U /**< RequestTransferExit */
#define DCM_SERVICE_0x3E 0x3EU /**< TesterPresent */
#define DCM_SERVICE_0x85 0x85U /**< ControlDTCSetting */

    /* ============= Negative Response Codes (NRC) ============= */

#define DCM_NRC_GENERAL_REJECT 0x31U
#define DCM_NRC_SERVICE_NOT_SUPPORTED 0x11U
#define DCM_NRC_SUBFUNC_NOT_SUPPORTED 0x12U
#define DCM_NRC_WRONG_MSG_LENGTH 0x13U
#define DCM_NRC_COND_NOT_CORRECT 0x22U
#define DCM_NRC_SECURITY_DENIED 0x33U
#define DCM_NRC_INVALID_SESSION 0x7EU

    /* ============= Session IDs ============= */

#define DCM_SESSION_DEFAULT 0x01U
#define DCM_SESSION_PROGRAMMING 0x02U
#define DCM_SESSION_EXTENDED 0x03U

    /* ============= Security Levels ============= */

#define DCM_SECURITY_LEVEL_0 0x00U /**< 无安全保护 */
#define DCM_SECURITY_LEVEL_1 0x01U /**< L1: APP扩展权限 */
#define DCM_SECURITY_LEVEL_2 0x02U /**< L2: FBL编程权限 */

    /* ============= Function Declarations ============= */

    /**
     * @brief DCM初始化
     * @return Std_ReturnType STD_OK或STD_NOT_OK
     */
    Std_ReturnType Bsw_Dcm_Init(void);

    /**
     * @brief DCM主处理函数（周期调用）
     */
    void Bsw_Dcm_MainFunction(void);

    /**
     * @brief 处理诊断请求
     * @param[in] Request 请求数据指针
     * @param[in] RequestLength 请求数据长度
     * @param[out] Response 响应数据指针
     * @param[out] ResponseLength 响应数据长度
     * @return Std_ReturnType
     */
    Std_ReturnType Bsw_Dcm_ProcessRequest(const uint8 *Request,
                                          uint16 RequestLength,
                                          uint8 *Response,
                                          uint16 *ResponseLength);

    /**
     * @brief 0x10 服务处理：会话控制
     */
    Std_ReturnType Bsw_Dcm_SessionControl(const uint8 *Request,
                                          uint16 RequestLength,
                                          uint8 *Response,
                                          uint16 *ResponseLength);

    /**
     * @brief 0x22 服务处理：读DID
     */
    Std_ReturnType Bsw_Dcm_ReadDid(const uint8 *Request,
                                   uint16 RequestLength,
                                   uint8 *Response,
                                   uint16 *ResponseLength);

    /**
     * @brief 0x2E 服务处理：写DID
     */
    Std_ReturnType Bsw_Dcm_WriteDid(const uint8 *Request,
                                    uint16 RequestLength,
                                    uint8 *Response,
                                    uint16 *ResponseLength);

    /**
     * @brief 0x27 服务处理：安全访问
     */
    Std_ReturnType Bsw_Dcm_SecurityAccess(const uint8 *Request,
                                          uint16 RequestLength,
                                          uint8 *Response,
                                          uint16 *ResponseLength);

    /**
     * @brief 获取当前会话
     * @return 当前会话ID
     */
    uint8 Bsw_Dcm_GetCurrentSession(void);

    /**
     * @brief 获取安全级别
     * @return 当前安全级别
     */
    uint8 Bsw_Dcm_GetSecurityLevel(void);

#ifdef __cplusplus
}
#endif

#endif /* DCM_SERVICE_H */
