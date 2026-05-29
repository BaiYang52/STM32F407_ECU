/**
 * @file can_driver.h
 * @brief CAN驱动程序头文件
 * @version 1.0.0
 * @date 2024-01-01
 */

#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* ============= Includes ============= */
#include "can_types.h"
#include "stm32f4xx_hal.h"
#include "types.h"

/* ============= Defines ============= */

/**
 * @defgroup CAN_ID CAN消息ID定义
 * @{
 */
#define MCAL_CAN1_CHANNEL 0U
#define MCAL_CAN2_CHANNEL 1U

#define MCAL_CAN_RX_BUFFER_SIZE 256U
#define MCAL_CAN_TX_BUFFER_SIZE 64U
#define MCAL_CAN_MAX_DLC 8U
    /** @} */

    /* ============= Enums ============= */

    /**
     * @enum Can_ChannelType
     * @brief CAN通道枚举
     */
    typedef enum
    {
        CAN_CHANNEL_1 = 0, /**< CAN1通道 */
        CAN_CHANNEL_2,     /**< CAN2通道 */
        CAN_CHANNEL_MAX
    } Can_ChannelType;

    /**
     * @enum Can_MessageStateType
     * @brief CAN消息状态枚举
     */
    typedef enum
    {
        CAN_MSG_IDLE = 0,
        CAN_MSG_SENDING,
        CAN_MSG_SENT,
        CAN_MSG_SEND_FAILED
    } Can_MessageStateType;

    /* ============= Structures ============= */

    /**
     * @struct Can_FrameType
     * @brief CAN帧结构体
     */
    typedef struct
    {
        uint32 id;                    /**< CAN消息ID */
        uint8 dlc;                    /**< 数据长度代码 (0-8) */
        uint8 data[MCAL_CAN_MAX_DLC]; /**< 数据字节数组 */
        uint8 ide;                    /**< IDE标志 (0: 11bit, 1: 29bit) */
        uint8 rtr;                    /**< RTR标志 */
    } Can_FrameType;

    /**
     * @struct Can_ConfigType
     * @brief CAN驱动配置结构体
     */
    typedef struct
    {
        uint32 baudrate;    /**< 波特率 (kbps): 125, 250, 500, 1000 */
        uint8 channel;      /**< CAN通道号 */
        uint8 rxFilterMode; /**< 接收过滤模式 */
    } Can_ConfigType;

    /* ============= Public Functions ============= */

    /**
     * @brief CAN驱动初始化
     * @param[in] Config CAN配置指针
     * @return Std_ReturnType
     *   @retval STD_OK    初始化成功
     *   @retval STD_NOT_OK 初始化失败
     * @details
     *   - 配置CAN硬件参数
     *   - 初始化发送/接收缓冲区
     *   - 使能CAN中断
     */
    Std_ReturnType Mcal_Can_Init(const Can_ConfigType *Config);

    /**
     * @brief CAN消息发送
     * @param[in] Channel CAN通道号
     * @param[in] Frame CAN帧指针
     * @return Std_ReturnType
     *   @retval STD_OK        发送请求成功
     *   @retval STD_NOT_OK    发送缓冲区满或其他错误
     * @details
     *   - 将消息放入发送队列
     *   - 硬件自动发送
     */
    Std_ReturnType Mcal_Can_Send(uint8 Channel, const Can_FrameType *Frame);

    /**
     * @brief CAN消息接收
     * @param[in] Channel CAN通道号
     * @param[out] Frame 接收帧指针
     * @return Std_ReturnType
     *   @retval STD_OK        接收缓冲区有新数据
     *   @retval STD_NOT_OK    接收缓冲区为空
     * @details
     *   - 从接收缓冲区取出消息
     */
    Std_ReturnType Mcal_Can_Receive(uint8 Channel, Can_FrameType *Frame);

    /**
     * @brief 获取CAN驱动状态
     * @param[in] Channel CAN通道号
     * @return Can_MessageStateType CAN当前状态
     */
    Can_MessageStateType Mcal_Can_GetState(uint8 Channel);

    /**
     * @brief 使能CAN接收器
     * @param[in] Channel CAN通道号
     */
    void Mcal_Can_EnableReceiver(uint8 Channel);

    /**
     * @brief 禁能CAN接收器
     * @param[in] Channel CAN通道号
     */
    void Mcal_Can_DisableReceiver(uint8 Channel);

    /**
     * @brief CAN接收中断处理程序
     */
    void Mcal_Can_RxISR(void);

    /**
     * @brief CAN发送中断处理程序
     */
    void Mcal_Can_TxISR(void);

    /**
     * @brief CAN错误中断处理程序
     */
    void Mcal_Can_ErrorISR(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_DRIVER_H */