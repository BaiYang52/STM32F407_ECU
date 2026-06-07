/**
 * @file    Platform_Types.h
 * @brief   AUTOSAR Platform Specific Types
 * @details This file contains all platform specific type definitions.
 * @version 4.6.0
 * @date    2020-11-30
 */

#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

/*=======================================================================*/
/* Version Information                                                   */
/*=======================================================================*/
#define PLATFORM_TYPES_VENDOR_ID                0x00U
#define PLATFORM_TYPES_MODULE_ID                0x02U
#define PLATFORM_TYPES_SW_MAJOR_VERSION         1U
#define PLATFORM_TYPES_SW_MINOR_VERSION         0U
#define PLATFORM_TYPES_SW_PATCH_VERSION         0U
#define PLATFORM_TYPES_AR_RELEASE_MAJOR_VERSION 4U
#define PLATFORM_TYPES_AR_RELEASE_MINOR_VERSION 0U
#define PLATFORM_TYPES_AR_RELEASE_REVISION_VERSION 3U

/*=======================================================================*/
/* CPU Type Configuration                                                */
/*=======================================================================*/
/**
 * @defgroup CPU_Type CPU Type Definitions
 * @{
 */
#define CPU_TYPE_8       8U      /**< 8-bit CPU */
#define CPU_TYPE_16      16U     /**< 16-bit CPU */
#define CPU_TYPE_32      32U     /**< 32-bit CPU */
#define CPU_TYPE_64      64U     /**< 64-bit CPU */

#ifndef CPU_TYPE
    #define CPU_TYPE CPU_TYPE_32 /**< Default CPU type (32-bit) */
#endif
/** @} */

/*=======================================================================*/
/* CPU Bit Order Configuration                                           */
/*=======================================================================*/
/**
 * @defgroup CPU_Bit_Order CPU Bit Order Definitions
 * @{
 */
#define CPU_BIT_ORDER_LSB_FIRST  0U   /**< LSB first (little endian) */
#define CPU_BIT_ORDER_MSB_FIRST  1U   /**< MSB first (big endian) */

#ifndef CPU_BIT_ORDER
    #define CPU_BIT_ORDER CPU_BIT_ORDER_LSB_FIRST /**< Default LSB first */
#endif
/** @} */

/*=======================================================================*/
/* CPU Byte Order Configuration                                          */
/*=======================================================================*/
/**
 * @defgroup CPU_Byte_Order CPU Byte Order Definitions
 * @{
 */
#define CPU_BYTE_ORDER_LITTLE_ENDIAN  0U   /**< Little endian */
#define CPU_BYTE_ORDER_BIG_ENDIAN     1U   /**< Big endian */

#ifndef CPU_BYTE_ORDER
    #define CPU_BYTE_ORDER CPU_BYTE_ORDER_LITTLE_ENDIAN /**< Default little endian */
#endif
/** @} */

/*=======================================================================*/
/* Boolean Type                                                          */
/*=======================================================================*/
#ifndef TRUE
    #define TRUE    1U   /**< Boolean true value */
#endif

#ifndef FALSE
    #define FALSE   0U   /**< Boolean false value */
#endif

#ifndef __cplusplus
    #ifndef _Bool
        typedef unsigned char boolean; /**< Boolean type for C */
    #else
        typedef _Bool boolean;         /**< Boolean type for C99 */
    #endif
#endif

/*=======================================================================*/
/* AUTOSAR Integer Data Types (Fixed Size)                               */
/*=======================================================================*/
/**
 * @defgroup Fixed_Size_Types Fixed Size Integer Types
 * @{
 */
typedef unsigned char         uint8;      /**< Unsigned 8-bit integer  (0 .. 255) */
typedef signed char           sint8;      /**< Signed 8-bit integer    (-128 .. 127) */
typedef unsigned short        uint16;     /**< Unsigned 16-bit integer (0 .. 65535) */
typedef signed short          sint16;     /**< Signed 16-bit integer   (-32768 .. 32767) */
typedef unsigned long         uint32;     /**< Unsigned 32-bit integer (0 .. 4294967295) */
typedef signed long           sint32;     /**< Signed 32-bit integer   (-2147483648 .. 2147483647) */
typedef unsigned long long    uint64;     /**< Unsigned 64-bit integer */
typedef signed long long      sint64;     /**< Signed 64-bit integer */
/** @} */

/*=======================================================================*/
/* AUTOSAR Integer Data Types (Minimum Size)                             */
/*=======================================================================*/
/**
 * @defgroup Minimum_Size_Types Minimum Size Integer Types
 * @{
 */
typedef unsigned char         uint8_least;    /**< At least 8-bit unsigned integer */
typedef signed char           sint8_least;    /**< At least 8-bit signed integer */
typedef unsigned short        uint16_least;   /**< At least 16-bit unsigned integer */
typedef signed short          sint16_least;   /**< At least 16-bit signed integer */
typedef unsigned long         uint32_least;   /**< At least 32-bit unsigned integer */
typedef signed long           sint32_least;   /**< At least 32-bit signed integer */
/** @} */

/*=======================================================================*/
/* Floating Point Types                                                  */
/*=======================================================================*/
/**
 * @defgroup Floating_Point_Types Floating Point Types
 * @{
 */
typedef float                 float32;    /**< 32-bit floating point (IEEE 754) */
typedef double                float64;    /**< 64-bit floating point (IEEE 754) */
/** @} */

/*=======================================================================*/
/* Pointer Types                                                         */
/*=======================================================================*/
/**
 * @defgroup Pointer_Types Pointer Types
 * @{
 */
typedef void*                 VoidPtr;    /**< Generic void pointer */
typedef const void*           ConstVoidPtr; /**< Generic const void pointer */
/** @} */

/*=======================================================================*/
/* Maximum and Minimum Values                                            */
/*=======================================================================*/
/**
 * @defgroup Limits Limits for Integer Types
 * @{
 */
#define UINT8_MIN     0U                  /**< Minimum value for uint8 */
#define UINT8_MAX     255U                /**< Maximum value for uint8 */
#define SINT8_MIN     (-128)              /**< Minimum value for sint8 */
#define SINT8_MAX     127                 /**< Maximum value for sint8 */

#define UINT16_MIN    0U                  /**< Minimum value for uint16 */
#define UINT16_MAX    65535U              /**< Maximum value for uint16 */
#define SINT16_MIN    (-32768)            /**< Minimum value for sint16 */
#define SINT16_MAX    32767               /**< Maximum value for sint16 */

#define UINT32_MIN    0UL                 /**< Minimum value for uint32 */
#define UINT32_MAX    4294967295UL        /**< Maximum value for uint32 */
#define SINT32_MIN    (-2147483648L)      /**< Minimum value for sint32 */
#define SINT32_MAX    2147483647L         /**< Maximum value for sint32 */

#define UINT64_MIN    0ULL                /**< Minimum value for uint64 */
#define UINT64_MAX    18446744073709551615ULL /**< Maximum value for uint64 */
#define SINT64_MIN    (-9223372036854775808LL) /**< Minimum value for sint64 */
#define SINT64_MAX    9223372036854775807LL    /**< Maximum value for sint64 */
/** @} */

/*=======================================================================*/
/* File Version Check                                                    */
/*=======================================================================*/
#if (PLATFORM_TYPES_AR_RELEASE_MAJOR_VERSION != 4U)
    #error "Platform_Types.h: AUTOSAR major version mismatch!"
#endif

#if (PLATFORM_TYPES_AR_RELEASE_MINOR_VERSION != 0U)
    #error "Platform_Types.h: AUTOSAR minor version mismatch!"
#endif

#if (PLATFORM_TYPES_AR_RELEASE_REVISION_VERSION != 3U)
    #error "Platform_Types.h: AUTOSAR revision version mismatch!"
#endif

#endif /* PLATFORM_TYPES_H */