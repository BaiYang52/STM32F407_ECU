/**
 * @file can_driver.h
 * @brief AUTOSAR CAN Driver 接口头文件
 * @version 2.0.0
 *
 * 遵循 AUTOSAR_SWS_CANDriver v4.4 规范。
 * 本驱动直接使用 CubeMX 已通过 MX_CANx_Init() 初始化的 hcan1/hcan2 句柄，
 * 不重复初始化硬件。
 *
 * 依赖（由 CubeMX 生成）:
 *   - hcan1 / hcan2 (全局变量，声明于 main.h)
 *   - HAL_CAN_RxFifo0MsgPendingCallback (CubeMX 可回调)
 *   - HAL_CAN_TxMailbox0CompleteCallback (需在 stm32f4xx_it.c 中使能)
 */

#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "can_types.h"
#include "compiler.h"

/* ==================== 宏定义 ==================== */

/**
 * @brief 各通道接收 FIFO 最大深度
 */
#define CAN_RX_FIFO_DEPTH       64U

/**
 * @brief 各通道发送缓冲区最大深度
 */
#define CAN_TX_BUFFER_DEPTH     32U

/* ==================== 公开函数声明 ==================== */

/**
 * @brief CAN 驱动初始化
 * @param[in] Config   CAN 配置指针 (波特率、通道、自动恢复 BusOff)
 * @return Std_ReturnType
 *   @retval STD_OK      初始化成功
 *   @retval STD_NOT_OK  参数错误或硬件失败
 *
 * @note 本函数不重复初始化 CAN 硬件寄存器（CubeMX 已完成），
 *       仅安装中断回调、启动 CAN、初始化内部缓冲区。
 */
FUNC(Std_ReturnType, CAN_CODE)
Can_Init(
    CONSTP2VAR(Can_Config, AUTOMATIC, CAN_APPL_DATA) Config
);

/**
 * @brief CAN 消息发送（放入发送缓冲区）
 * @param[in] Channel  CAN 通道号 (0=CAN1, 1=CAN2)
 * @param[in] Frame    CAN 帧指针
 * @return Std_ReturnType
 *   @retval STD_OK      消息已加入发送队列
 *   @retval STD_NOT_OK  缓冲区满或通道无效
 */
FUNC(Std_ReturnType, CAN_CODE)
Can_Write(
    uint8                                    Channel,
    CONSTP2CONST(Can_Frame, AUTOMATIC, CAN_APPL_CONST) Frame
);

/**
 * @brief CAN 消息接收（从接收 FIFO 取出）
 * @param[in]      Channel  CAN 通道号
 * @param[in,out]  Frame    接收帧指针
 * @return Std_ReturnType
 *   @retval STD_OK      成功取出一条消息
 *   @retval STD_NOT_OK  FIFO 为空或通道无效
 */
FUNC(Std_ReturnType, CAN_CODE)
Can_Read(
    uint8                    Channel,
    P2VAR(Can_Frame, AUTOMATIC, CAN_APPL_DATA) Frame
);

/**
 * @brief 获取 CAN 控制器状态
 * @param[in]  Channel  CAN 通道号
 * @return Can_StateType 控制器当前状态
 */
FUNC(Can_StateType, CAN_CODE)
Can_GetControllerState(
    uint8 Channel
);

/**
 * @brief 获取 CAN 错误计数
 * @param[in]  Channel     CAN 通道号
 * @param[out] TxErrCntPtr 发送错误计数器指针 (可为 NULL)
 * @param[out] RxErrCntPtr 接收错误计数器指针 (可为 NULL)
 */
FUNC(void, CAN_CODE)
Can_GetErrorCounters(
    uint8           Channel,
    P2VAR(uint8, AUTOMATIC, CAN_APPL_DATA) TxErrCntPtr,
    P2VAR(uint8, AUTOMATIC, CAN_APPL_DATA) RxErrCntPtr
);

/**
 * @brief 注册接收通知回调（供 BSW 层 CanIf 使用）
 * @param[in] Callback 回调函数指针
 *
 * @note 回调在 ISR 上下文中执行，必须简短。
 *       收到一帧时，回调被调用一次。
 */
FUNC(void, CAN_CODE)
Can_SetRxNotification(
    Can_RxNotification Callback
);

/* ==================== 中断处理函数声明 ==================== */

/**
 * @brief CAN RX FIFO0 中断处理（由 HAL 回调触发）
 * @param[in] hcan   HAL CAN 句柄指针
 *
 * @note 从外部 EXTERN 声明，由 stm32f4xx_it.c 中注册的
 *       HAL_CAN_RxFifo0MsgPendingCallback() 调用。
 */
FUNC(void, CAN_CODE)
Can_RxISR(
    P2VAR(void, AUTOMATIC, CAN_APPL_DATA) hcan
);

/**
 * @brief CAN TX 完成中断处理（由 HAL 回调触发）
 * @param[in] hcan   HAL CAN 句柄指针
 */
FUNC(void, CAN_CODE)
Can_TxISR(
    P2VAR(void, AUTOMATIC, CAN_APPL_DATA) hcan
);

/**
 * @brief CAN 错误状态改变中断处理
 * @param[in] hcan   HAL CAN 句柄指针
 */
FUNC(void, CAN_CODE)
Can_ErrorISR(
    P2VAR(void, AUTOMATIC, CAN_APPL_DATA) hcan
);

/**
 * @brief CAN 主函数轮询（发送缓冲调度）
 * @note 在 10ms 任务中调用，将 TX 缓冲区的消息真正写入硬件 TX 邮箱。
 */
FUNC(void, CAN_CODE)
Can_MainFunction_Write(void);

/**
 * @brief CAN 主函数轮询（BusOff 恢复监测）
 * @note 在 100ms 任务中调用，监测 BusOff 状态并尝试恢复。
 */
FUNC(void, CAN_CODE)
Can_MainFunction_BusOff(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_DRIVER_H */