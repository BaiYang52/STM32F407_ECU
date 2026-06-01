#ifndef __TEST_KEY_H
#define __TEST_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

void Test_Key_Init(void);
void Test_Key_10ms_Task(void);
uint8_t Test_Key_GetState(uint8_t key_id);  /* 返回: 0=未按, 1=按下, 2=无效 */
uint32_t Test_Key_GetPressTime(uint8_t key_id);  /* 单位: ms */

#ifdef __cplusplus
}
#endif

#endif
