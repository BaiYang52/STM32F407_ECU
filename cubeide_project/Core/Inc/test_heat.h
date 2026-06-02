/**
 * @file types.h
 * @brief AUTOSAR标准类型定义
 */

#ifndef __TYPES_H
#define __TYPES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 标准返回值类型 */
typedef uint8_t Std_ReturnType;

#define STD_OK      0x00U
#define STD_NOT_OK  0x01U

/* 布尔类型 */
typedef uint8_t boolean;
#define TRUE        1U
#define FALSE       0U

/* 标准整数类型 */
typedef int8_t      sint8;
typedef uint8_t     uint8;
typedef int16_t     sint16;
typedef uint16_t    uint16;
typedef int32_t     sint32;
typedef uint32_t    uint32;
typedef int64_t     sint64;
typedef uint64_t    uint64;

/* 浮点类型 */
typedef float       float32;
typedef double      float64;

Std_ReturnType Mcal_DS18B20_Init(void);
Std_ReturnType Mcal_DS18B20_ReadTemperatureRaw(int16_t *TemperatureRaw);
Std_ReturnType Mcal_DS18B20_StartConversion(void);
Std_ReturnType Mcal_DS18B20_ReadTemperature(float *Temperature);
float Mcal_DS18B20_GetLastTemperature(void);
uint8_t Mcal_DS18B20_IsInitialized(void);

#ifdef __cplusplus
}
#endif

#endif /* __TYPES_H */
