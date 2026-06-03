/**
 * @file can_interrupt.c
 * @brief CAN 中断处理 — HAL 回调桥接层
 * @version 1.0.0
 *
 * 本文件作为 CubeMX HAL 回调与 AUTOSAR CAN Driver 之间的桥接。
 *
 * CubeMX 在 stm32f4xx_it.c 中自动生成以下 HAL 回调弱定义：
 *   - HAL_CAN_RxFifo0MsgPendingCallback()
 *   - HAL_CAN_TxMailbox0CompleteCallback()
 *   - HAL_CAN_ErrorCallback()
 *
 * 我们在此处覆盖这些弱定义，将控制权转交给 Can_RxISR / Can_TxISR / Can_ErrorISR。
 *
 * @note 这些函数由 HAL 在中断上下文中调用，必须保持简短。
 */

#include "can_driver.h"
#include "stm32f4xx_hal.h"

/* ==================== HAL 回调覆盖 ==================== */

/**
 * @brief CAN RX FIFO0 消息等待回调
 *
 * 当 CAN 外设接收 FIFO0 中有新消息时，HAL 调用此函数。
 * 这里将其桥接到 AUTOSAR Can_RxISR。
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    Can_RxISR((void *)hcan);
}

/**
 * @brief CAN TX 邮箱0 发送完成回调
 *
 * 当 TX 邮箱0 成功发送一帧后，HAL 调用此函数。
 * 桥接到 Can_TxISR 以释放发送缓冲区。
 */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
    Can_TxISR((void *)hcan);
}

/**
 * @brief CAN TX 邮箱1 发送完成回调
 */
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
    Can_TxISR((void *)hcan);
}

/**
 * @brief CAN TX 邮箱2 发送完成回调
 */
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
    Can_TxISR((void *)hcan);
}

/**
 * @brief CAN 错误状态改变回调
 *
 * 当 CAN 外设检测到错误状态变化时，HAL 调用此函数。
 * 桥接到 Can_ErrorISR 以记录 TEC/REC 和 BusOff 状态。
 */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    Can_ErrorISR((void *)hcan);
}