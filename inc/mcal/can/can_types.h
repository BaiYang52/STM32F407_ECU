/**
 * @file can_types.h
 * @brief AUTOSAR CAN Driver 类型定义
 * @version 1.0.0
 *
 * 遵循 AUTOSAR_SWS_CANDriver 规范，定义 CAN 驱动层使用的基本数据类型。
 */

#ifndef CAN_TYPES_H
#define CAN_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "types.h"
#include "compiler.h"

/* ==================== 常量定义 ==================== */

/**
 * @brief CAN 最大数据长度 (经典 CAN，不含 CAN-FD)
 */
#define CAN_MAX_DLC              8U

/**
 * @brief CAN 硬件通道数
 */
#define CAN_NUM_OF_CHANNELS     2U

/* ==================== 枚举类型 ==================== */

/**
 * @enum Can_ChannelType
 * @brief CAN 硬件通道枚举
 */
typedef enum
{
    CAN_CHANNEL_1 = 0U,         /**< CAN1 (PB8/PB9) */
    CAN_CHANNEL_2 = 1U,         /**< CAN2 (PB12/PB13) */
    CAN_CHANNEL_MAX             /**< 通道总数 */
} Can_ChannelType;

/**
 * @enum Can_IdType
 * @brief CAN 标识符类型
 */
typedef enum
{
    CAN_ID_STANDARD = 0U,       /**< 11-bit 标准 ID */
    CAN_ID_EXTENDED = 1U        /**< 29-bit 扩展 ID */
} Can_IdType;

/**
 * @enum Can_FrameType
 * @brief CAN 帧类型
 */
typedef enum
{
    CAN_FRAME_DATA   = 0U,      /**< 数据帧 */
    CAN_FRAME_REMOTE = 1U       /**< 远程帧 */
} Can_FrameType;

/**
 * @enum Can_StateType
 * @brief CAN 控制器状态
 */
typedef enum
{
    CAN_STATE_UNINIT = 0U,      /**< 未初始化 */
    CAN_STATE_STOPPED,          /**< 停止 */
    CAN_STATE_STARTED,          /**< 启动运行 */
    CAN_STATE_SLEEP,            /**< 休眠 */
    CAN_STATE_BUSOFF            /**< Bus-Off */
} Can_StateType;

/**
 * @enum Can_ErrorType
 * @brief CAN 错误类型
 */
typedef enum
{
    CAN_ERR_OK          = 0U,   /**< 无错误 */
    CAN_ERR_BUSOFF,             /**< Bus-Off */
    CAN_ERR_OVERRUN,            /**< FIFO 溢出 */
    CAN_ERR_TRANSMIT_FAIL,      /**< 发送失败 */
    CAN_ERR_RECEIVE_FAIL,       /**< 接收失败 */
    CAN_ERR_PARAM,              /**< 参数错误 */
    CAN_ERR_UNINIT              /**< 未初始化 */
} Can_ErrorType;

/**
 * @enum Can_ObjectType
 * @brief CAN 硬件对象类型 (发送 / 接收)
 */
typedef enum
{
    CAN_OBJECT_TYPE_TX = 0U,    /**< 发送对象 */
    CAN_OBJECT_TYPE_RX = 1U     /**< 接收对象 */
} Can_ObjectType;

/* ==================== 结构体类型 ==================== */

/**
 * @struct Can_Frame
 * @brief AUTOSAR CanFrame 结构体
 */
typedef struct
{
    uint32            id;                     /**< CAN ID (11-bit 或 29-bit) */
    uint8             dlc;                    /**< 数据长度 (0-8) */
    uint8             sdu[CAN_MAX_DLC];       /**< 数据字节 */
    Can_IdType        idType;                 /**< ID 类型 (标准/扩展) */
    Can_FrameType     frameType;              /**< 帧类型 (数据/远程) */
} Can_Frame;

/**
 * @struct Can_HwType
 * @brief CAN 硬件对象句柄 (参考 AUTOSAR Can_HwType)
 */
typedef struct
{
    uint8             channel;                /**< 硬件通道 (0=CAN1, 1=CAN2) */
    uint8             objectId;               /**< 硬件对象 ID (邮箱) */
    Can_ObjectType    objectType;             /**< 对象类型 (TX/RX) */
} Can_HwType;

/**
 * @struct Can_Config
 * @brief CAN 控制器配置结构体
 */
typedef struct
{
    uint32            baudrate;               /**< 波特率 (kbps): 125/250/500/1000 */
    uint8             channel;                /**< 通道号 @ref Can_ChannelType */
    boolean           autoBusOff;             /**< 自动恢复 BusOff */
} Can_Config;

/* ==================== CAN 接收回调函数指针 ==================== */

/**
 * @brief CAN 接收通知回调类型 (供 BSW 层注册)
 * @param[in] Channel  CAN 通道号
 * @param[in] Frame    接收到的帧指针 (只读)
 */
typedef void (*Can_RxNotification)(uint8 Channel, const Can_Frame *Frame);

#ifdef __cplusplus
}
#endif

#endif /* CAN_TYPES_H */