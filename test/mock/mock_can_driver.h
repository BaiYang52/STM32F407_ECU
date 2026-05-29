/**
 * @file mock_can_driver.h
 * @brief CAN驱动Mock对象头文件
 * @version 1.0.0
 * @date 2024-01-01
 */

#ifndef MOCK_CAN_DRIVER_H
#define MOCK_CAN_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "can_driver.h"
#include "types.h"

    /* ============= Mock Global Variables ============= */

    /** 初始化调用标志 */
    extern uint8 mock_can_init_called;

    /** 初始化的波特率 */
    extern uint32 mock_can_baudrate;

    /** 发送计数 */
    extern uint32 mock_can_tx_count;

    /** 接收计数 */
    extern uint32 mock_can_rx_count;

    /* ============= Mock Function Prototypes ============= */

    /**
     * @brief Mock初始化设置
     */
    void mock_can_init_setup(void);

    /**
     * @brief Mock接收数据设置
     */
    void mock_can_receive_data(const Can_FrameType *Frame);

    /**
     * @brief Mock重置
     */
    void mock_can_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_CAN_DRIVER_H */