/**
 * @file    Std_Types.h
 * @brief   AUTOSAR Standard Types
 * @details This file contains all types that are used across several modules 
 *          of the basic software and that are platform and compiler independent.
 * @version 4.6.0
 * @date    2020-11-30
 */

#ifndef STD_TYPES_H
#define STD_TYPES_H

/*=======================================================================*/
/* Version Information                                                   */
/*=======================================================================*/
#define STD_TYPES_VENDOR_ID                0x00U
#define STD_TYPES_MODULE_ID                0x01U
#define STD_TYPES_SW_MAJOR_VERSION         1U
#define STD_TYPES_SW_MINOR_VERSION         0U
#define STD_TYPES_SW_PATCH_VERSION         0U
#define STD_TYPES_AR_RELEASE_MAJOR_VERSION 4U
#define STD_TYPES_AR_RELEASE_MINOR_VERSION 0U
#define STD_TYPES_AR_RELEASE_REVISION_VERSION 3U

/*=======================================================================*/
/* Include Files                                                         */
/*=======================================================================*/
#include "Platform_Types.h"
#include "Compiler.h"

/*=======================================================================*/
/* Development Errors                                                    */
/*=======================================================================*/
/**
 * @defgroup Std_DevelopmentErrors Standard Development Errors
 * @{
 */
#define STD_E_OK                   0x00U   /**< Development error code for OK */
#define STD_E_INVALID_FNC          0x01U   /**< Invalid function called */
#define STD_E_INVALID_DATA         0x02U   /**< Invalid data provided */
#define STD_E_LOST_DATA            0x03U   /**< Data lost */
#define STD_E_PARAM                0x04U   /**< Invalid parameter */
#define STD_E_UNINIT               0x05U   /**< Module not initialized */
#define STD_E_TRANSF_FAILED        0x06U   /**< Transformation failed */
#define STD_E_DOMAIN               0x07U   /**< Domain error */
#define STD_E_RANGE                0x08U   /**< Range error */
/** @} */

/*=======================================================================*/
/* Standard Return Type                                                  */
/*=======================================================================*/
/**
 * @brief Standard return type for AUTOSAR services
 * @details This type can be used as standard API return type which is 
 *          shared between the RTE and the BSW modules.
 */
typedef uint8 Std_ReturnType;

/**
 * @brief Standard version information type
 * @details This type shall be used to request the version of a BSW module
 *          using the <ModuleName>_GetVersionInfo() function.
 */
typedef struct {
    uint16 vendorID;              /**< Vendor ID */
    uint16 moduleID;              /**< Module ID */
    uint8  sw_major_version;      /**< Software major version */
    uint8  sw_minor_version;      /**< Software minor version */
    uint8  sw_patch_version;      /**< Software patch version */
} Std_VersionInfoType;

/*=======================================================================*/
/* Standard Constants                                                    */
/*=======================================================================*/
#ifndef E_OK
#define E_OK                      0x00U   /**< Function return OK */
#endif

#ifndef E_NOT_OK
#define E_NOT_OK                  0x01U   /**< Function return NOT OK */
#endif

#ifndef STD_HIGH
#define STD_HIGH                  0x01U   /**< Physical state 5V or 3.3V */
#endif

#ifndef STD_LOW
#define STD_LOW                   0x00U   /**< Physical state 0V */
#endif

#ifndef STD_ACTIVE
#define STD_ACTIVE                0x01U   /**< Logical state active */
#endif

#ifndef STD_IDLE
#define STD_IDLE                  0x00U   /**< Logical state idle */
#endif

#ifndef STD_ON
#define STD_ON                    0x01U   /**< Standard ON */
#endif

#ifndef STD_OFF
#define STD_OFF                   0x00U   /**< Standard OFF */
#endif

#ifndef NULL_PTR
#define NULL_PTR                  ((void *)0) /**< NULL pointer */
#endif

/*=======================================================================*/
/* Transformer Types                                                     */
/*=======================================================================*/
/**
 * @brief Transformer error code type
 */
typedef uint8 Std_TransformerErrorCode;

/**
 * @brief Transformer class enumeration
 */
typedef enum {
    STD_TRANSFORMER_UNSPECIFIED   = 0x00U, /**< Unspecified transformer class */
    STD_TRANSFORMER_SERIALIZER    = 0x01U, /**< Serializer transformer class */
    STD_TRANSFORMER_SAFETY        = 0x02U, /**< Safety transformer class */
    STD_TRANSFORMER_SECURITY      = 0x03U, /**< Security transformer class */
    STD_TRANSFORMER_CUSTOM        = 0xFFU  /**< Custom transformer class */
} Std_TransformerClass;

/**
 * @brief Transformer error structure
 */
typedef struct {
    Std_TransformerErrorCode errorCode;    /**< Error code */
    Std_TransformerClass transformerClass; /**< Transformer class */
} Std_TransformerError;

/**
 * @brief Transformer forward code type
 */
typedef enum {
    STD_TRANSFORMER_FORWARD_OK            = 0x00U, /**< No specific error */
    STD_TRANSFORMER_FORWARD_INVALID_REP   = 0x01U, /**< Repeat last used sequence number */
    STD_TRANSFORMER_FORWARD_INVALID_SEQ   = 0x02U, /**< Use wrong sequence number */
    STD_TRANSFORMER_FORWARD_INVALID_CRC   = 0x03U  /**< Generate intentionally wrong CRC */
} Std_TransformerForwardCode;

/**
 * @brief Message type type
 */
typedef uint8 Std_MessageTypeType;

#define STD_MESSAGETYPE_REQUEST   0x00U   /**< Request message type */
#define STD_MESSAGETYPE_RESPONSE  0x01U   /**< Response message type */

/**
 * @brief Message result type
 */
typedef uint8 Std_MessageResultType;

/*=======================================================================*/
/* File Version Check                                                    */
/*=======================================================================*/
#if (STD_TYPES_AR_RELEASE_MAJOR_VERSION != 4U)
    #error "Std_Types.h: AUTOSAR major version mismatch!"
#endif

#if (STD_TYPES_AR_RELEASE_MINOR_VERSION != 0U)
    #error "Std_Types.h: AUTOSAR minor version mismatch!"
#endif

#if (STD_TYPES_AR_RELEASE_REVISION_VERSION != 3U)
    #error "Std_Types.h: AUTOSAR revision version mismatch!"
#endif

#endif /* STD_TYPES_H */