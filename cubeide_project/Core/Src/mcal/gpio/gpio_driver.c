/**
 * @file gpio_driver.c
 * @brief AUTOSAR Dio Driver 实现
 * @version 1.0.0
 *
 * 包装 CubeMX HAL_GPIO 函数，提供 AUTOSAR 风格 Dio 接口。
 * 引脚映射表将 DIO 通道 ID 转换为 (GPIO_Port, GPIO_Pin) 对。
 */

#include "gpio_driver.h"
#include "stm32f4xx_hal.h"

/* ==================== 引脚映射表 ==================== */

/**
 * @struct Dio_PinMapType
 * @brief 引脚映射条目
 */
typedef struct
{
    GPIO_TypeDef   *port;          /**< GPIO 端口基地址 */
    uint16          pin;           /**< GPIO 引脚号 */
} Dio_PinMapType;

/**
 * @brief 通道 ID → (端口, 引脚) 映射表
 *
 * 对应 CubeMX 配置文件 (stm32f407ve_cfg.ioc) 中的引脚分配。
 * 索引 = DIO_CH_xxx 宏定义值。
 */
static const Dio_PinMapType s_pinMap[] = {
    /* DIO_CH_DS18B20_DQ  */  {GPIOE, GPIO_PIN_0},
    /* DIO_CH_KEY1        */  {GPIOE, GPIO_PIN_3},
    /* DIO_CH_KEY0        */  {GPIOE, GPIO_PIN_4},
    /* DIO_CH_LED_PWM     */  {GPIOD, GPIO_PIN_12},
    /* DIO_CH_MOTOR_PWM   */  {GPIOD, GPIO_PIN_13},
    /* DIO_CH_MOTOR_DIR   */  {GPIOD, GPIO_PIN_14},
    /* DIO_CH_FLASH_CS    */  {GPIOB, GPIO_PIN_0},
    /* DIO_CH_CAN1_STBY   */  {GPIOA, GPIO_PIN_8},
};

/** 映射表条目数 */
#define DIO_PIN_MAP_COUNT  (sizeof(s_pinMap) / sizeof(s_pinMap[0]))

/* ==================== 端口 ID → 寄存器基址 ==================== */

#define DIO_PORT_MAX  8U

/** GPIO 端口基址表 */
static GPIO_TypeDef * const s_portBases[] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH
};

/* ==================== 公开函数实现 ==================== */

/**
 * @brief DIO 驱动初始化
 *
 * CubeMX 已初始化所有 GPIO 时钟和模式寄存器。
 * 此处仅做驱动内部状态记录。
 */
FUNC(void, MCAL_CODE)
Dio_Init(void)
{
    /* CubeMX 已通过 MX_GPIO_Init() 完成硬件初始化 */
    /* 此处仅作为驱动层初始化入口，方便后续扩展 */
}

/**
 * @brief 读取单个引脚电平
 */
FUNC(Dio_LevelType, MCAL_CODE)
Dio_ReadChannel(
    Dio_ChannelType ChannelId
)
{
    if (ChannelId >= DIO_PIN_MAP_COUNT) {
        return DIO_LVL_LOW;
    }

    GPIO_PinState halLevel = HAL_GPIO_ReadPin(s_pinMap[ChannelId].port,
                                               s_pinMap[ChannelId].pin);
    return (halLevel == GPIO_PIN_SET) ? DIO_LVL_HIGH : DIO_LVL_LOW;
}

/**
 * @brief 写入单个引脚电平
 */
FUNC(void, MCAL_CODE)
Dio_WriteChannel(
    Dio_ChannelType  ChannelId,
    Dio_LevelType    Level
)
{
    if (ChannelId >= DIO_PIN_MAP_COUNT) {
        return;
    }

    GPIO_PinState halLevel = (Level == DIO_LVL_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(s_pinMap[ChannelId].port,
                      s_pinMap[ChannelId].pin,
                      halLevel);
}

/**
 * @brief 读取整个端口电平
 */
FUNC(Dio_PortLevelType, MCAL_CODE)
Dio_ReadPort(
    Dio_PortType PortId
)
{
    if (PortId >= DIO_PORT_MAX) {
        return 0U;
    }

    return (Dio_PortLevelType)(s_portBases[PortId]->IDR);
}

/**
 * @brief 写入整个端口电平
 */
FUNC(void, MCAL_CODE)
Dio_WritePort(
    Dio_PortType       PortId,
    Dio_PortLevelType  Level
)
{
    if (PortId >= DIO_PORT_MAX) {
        return;
    }

    s_portBases[PortId]->ODR = Level;
}

/**
 * @brief 翻转单个引脚电平
 *
 * 使用 BSRR 寄存器实现原子翻转。
 * BSRR 低 16 位 = 置位，高 16 位 = 复位。
 */
FUNC(void, MCAL_CODE)
Dio_FlipChannel(
    Dio_ChannelType ChannelId
)
{
    if (ChannelId >= DIO_PIN_MAP_COUNT) {
        return;
    }

    GPIO_TypeDef *port = s_pinMap[ChannelId].port;
    uint16         pin  = s_pinMap[ChannelId].pin;

    /* 读当前 ODR 对应位，如果为 1 则复位，否则置位 */
    if (port->ODR & pin) {
        port->BSRR = ((uint32)pin) << 16U;  /* BR 位 */
    } else {
        port->BSRR = pin;                    /* BS 位 */
    }
}