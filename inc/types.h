/**
 * @file types.h
 * @brief 标准类型和返回值定义
 * @version 1.0.0
 * @date 2024-01-01
 */

#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

    /* ============= Basic Types ============= */

    typedef unsigned char uint8;       /**< 无符号8位整数 */
    typedef signed char int8;          /**< 有符号8位整数 */
    typedef unsigned short uint16;     /**< 无符号16位整数 */
    typedef signed short int16;        /**< 有符号16位整数 */
    typedef unsigned long uint32;      /**< 无符号32位整数 */
    typedef signed long int32;         /**< 有符号32位整数 */
    typedef unsigned long long uint64; /**< 无符号64位整数 */
    typedef signed long long int64;    /**< 有符号64位整数 */

    typedef float float32;  /**< 单精度浮点 */
    typedef double float64; /**< 双精度浮点 */

    /* ============= Boolean Types ============= */

    typedef uint8 boolean; /**< 布尔类型 */

#ifndef TRUE
#define TRUE 1U
#endif

#ifndef FALSE
#define FALSE 0U
#endif

    /* ============= Return Types ============= */

    /**
     * @enum Std_ReturnType
     * @brief 标准返回类型
     */
    typedef uint8 Std_ReturnType;

#define STD_OK 0U     /**< 操作成功 */
#define STD_NOT_OK 1U /**< 操作失败 */

    /**
     * @enum Std_OkType
     * @brief 标准OK返回类型
     */
    typedef uint8 Std_OkType;

#define E_OK 0U

    /* ============= Size Types ============= */

    typedef uint32 size_t_emu; /**< Size emulation */

    /* ============= NULL Pointer ============= */

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* TYPES_H */