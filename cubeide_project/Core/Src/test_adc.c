/**
 * @file test_adc.c
 * @brief ADC温度传感器测试
 * @version 1.0.0
 */

#include "main.h"
#include "stdio.h"

#define ADC_SAMPLE_COUNT 10  /* 采样次数 */

typedef struct {
    uint16_t raw_value;           /* 原始ADC值 (0-4095) */
    float temperature;            /* 转换后温度 (℃) */
    uint16_t sample_buffer[ADC_SAMPLE_COUNT];
    uint8_t sample_index;
} ADC_InfoType;

static ADC_InfoType adc_info = {0};

/**
 * @brief ADC初始化
 */
void Test_ADC_Init(void)
{

//	if (HAL_ADC_Start(&hadc1) == HAL_OK) {
//		printf("[ADC] ADC1初始化完成 (PE5/PC5 温度传感器)\n");
//	}
//	else
//	{
//		printf("[ADC] ADC1初始化失败 (PE5/PC5 温度传感器)\n");
//	}
}

/**
 * @brief ADC转换完成回调 (由HAL驱动调用)
 */
//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
//{
//    if (hadc->Instance != ADC1) {
//        return;
//    }
//
//    /* 读取ADC值 */
//    uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
//
//    /* 存入采样缓冲 */
//    adc_info.sample_buffer[adc_info.sample_index] = adc_value;
//    adc_info.sample_index++;
//
//    if (adc_info.sample_index >= ADC_SAMPLE_COUNT) {
//        adc_info.sample_index = 0;
//
//        /* 计算平均值 */
//        uint32_t sum = 0;
//        for (int i = 0; i < ADC_SAMPLE_COUNT; i++) {
//            sum += adc_info.sample_buffer[i];
//        }
//        adc_info.raw_value = sum / ADC_SAMPLE_COUNT;
//    }
//}

/**
 * @brief ADC原始值转换为温度
 * @details 公式: T(℃) = ADC_Value × 0.0806 - 40
 *         即: T = (ADC_Value * 3.3V / 4096) 的某个转换
 *
 * 对于DS18B20:
 * 典型映射:
 *   ADC=0x000 (0V)        → -40℃ (或其他冷值)
 *   ADC=0x800 (2048)      → 0℃
 *   ADC=0xFFF (4095)      → +125℃
 *
 * 这里使用线性近似:
 * T = (ADC_Value / 4095) * 165 - 40
 *   其中 165 = 125 - (-40), 4095是12bit最大值
 */
static float Test_ADC_RawToTemp(uint16_t raw_value)
{
    /* 线性转换: -40℃ 到 +125℃ 对应 ADC 0 到 4095 */
    float temp = ((float)raw_value / 4095.0f) * 165.0f - 40.0f;
    return temp;
}

/**
 * @brief ADC 100ms 周期任务
 * @details 采样并转换温度，输出到串口
 */
void Test_ADC_100ms_Task(void)
{
    /* 读取当前ADC值 (来自中断中保存的平均值) */
    adc_info.temperature = Test_ADC_RawToTemp(adc_info.raw_value);

    printf("[ADC] 温度: %.2f℃ (原始值: 0x%04X = %d)\n",
           adc_info.temperature,
           adc_info.raw_value,
           adc_info.raw_value);
}

/**
 * @brief 获取温度值
 * @return 温度 (℃)
 */
float Test_ADC_GetTemperature(void)
{
    return adc_info.temperature;
}

/**
 * @brief 获取原始ADC值
 * @return ADC值 (0-4095)
 */
uint16_t Test_ADC_GetRaw(void)
{
    return adc_info.raw_value;
}
