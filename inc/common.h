/**
 * @file common.h
 * @brief 通用宏定义和函数
 * @version 1.0.0
 * @date $(date +%Y-%m-%d)
 */

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/* ============= Memory Operations ============= */
#define MEMSET(dst, val, len)    memset((dst), (val), (len))
#define MEMCPY(dst, src, len)    memcpy((dst), (src), (len))
#define MEMCMP(dst, src, len)    memcmp((dst), (src), (len))

/* ============= Bit Operations ============= */
#define SET_BIT(val, bit)        ((val) |= (1U << (bit)))
#define CLR_BIT(val, bit)        ((val) &= ~(1U << (bit)))
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
