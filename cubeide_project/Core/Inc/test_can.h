#ifndef __TEST_CAN_H
#define __TEST_CAN_H

#ifdef __cplusplus
extern "C" {
#endif

void Test_CAN_Init(void);
void Test_CAN_Send(uint32_t id, const uint8_t *data, uint8_t dlc);
void Test_CAN_100ms_Task(void);
void Test_CAN_SetMode(uint8_t mode);  /* 0: 循环发送, 1: 等待接收, 2: 回显 */
void Test_CAN_GetStats(uint32_t *tx_count, uint32_t *rx_count);

#ifdef __cplusplus
}
#endif

#endif
