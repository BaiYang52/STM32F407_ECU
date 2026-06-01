#ifndef __TEST_ADC_H
#define __TEST_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

void Test_ADC_Init(void);
void Test_ADC_100ms_Task(void);
float Test_ADC_GetTemperature(void);
uint16_t Test_ADC_GetRaw(void);

#ifdef __cplusplus
}
#endif

#endif
