#ifndef __TEST_PWM_H
#define __TEST_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* 函数声明 */
void Test_PWM_Init(void);
void Test_PWM_10ms_Task(void);
void Test_PWM_SetMode(uint8_t mode);  /* 0: 呼吸, 1: 速度扫描, 2: 正反转 */

#ifdef __cplusplus
}
#endif

#endif
