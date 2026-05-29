/**
 * @file types.h
 * @brief 标准类型定义
 * @version 1.0.0
 * @date $(date +%Y-%m-%d)
 */

#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============= Basic Types ============= */
typedef uint8_t     uint8;
typedef int8_t      int8;
typedef uint16_t    uint16;
typedef int16_t     int16;
typedef uint32_t    uint32;
typedef int32_t     int32;
typedef float       float32;
typedef double      float64;

typedef uint8 boolean;
#define TRUE  1U
#define FALSE 0U

/* ============= Return Types ============= */
typedef uint8 Std_ReturnType;
#define STD_OK     0U
#define STD_NOT_OK 1U

#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */
