/**
 * @file common.h
 * @brief 通用宏定义和函数
 * @version 2.0.0
 *
 * 包含标准头文件 <string.h> 提供 memcpy 声明。
 * SET_BIT/CLR_BIT 等宏增加 Include Guard 避免与 CMSIS 冲突。
 */

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include <string.h>          /* 提供 memcpy, memset 等 */

/* ============= NULL_PTR (AUTOSAR 标准) ============= */
#ifndef NULL_PTR
#define NULL_PTR             ((void *)0)
#endif

/* ============= Memory Operations ============= */
#define MEMSET(dst, val, len)    memset((dst), (val), (len))
#define MEMCPY(dst, src, len)    memcpy((dst), (src), (len))
#define MEMCMP(dst, src, len)    memcmp((dst), (src), (len))

/* ============= Bit Operations ============= */
/* 使用 ifndef 保护，避免与 CMSIS 的 SET_BIT/CLEAR_BIT 冲突 */
#ifndef SET_BIT
#define SET_BIT(val, bit)        ((val) |= (1U << (bit)))
#endif

#ifndef CLEAR_BIT
#define CLR_BIT(val, bit)        ((val) &= ~(1U << (bit)))
#endif

#define TST_BIT(val, bit)        (((val) >> (bit)) & 1U)
#define TGL_BIT(val, bit)        ((val) ^= (1U << (bit)))

/* ============= Byte Operations ============= */
#define GET_LOW_BYTE(word)       ((uint8)((word) & 0xFFU))
#define GET_HIGH_BYTE(word)      ((uint8)(((word) >> 8) & 0xFFU))
#define MAKE_WORD(h, l)          (((uint16)(h) << 8) | ((uint16)(l) & 0xFFU))

/* ============= Min/Max ============= */
#define MIN(a, b)                (((a) < (b)) ? (a) : (b))
#define MAX(a, b)                (((a) > (b)) ? (a) : (b))

/* ============= Array Size ============= */
#define ARRAY_SIZE(arr)          (sizeof(arr) / sizeof((arr)[0]))

/* ============= Assert ============= */
#ifdef DEBUG
#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf("Assertion failed at %s:%d\n", __FILE__, __LINE__); \
        } \
    } while(0)
#else
#define ASSERT(condition)
#endif

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H */