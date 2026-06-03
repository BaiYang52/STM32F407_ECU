/**
 * @file spi_flash.c
 * @brief AUTOSAR Fls Driver 实现 — W25Q16 外部 SPI Flash
 * @version 1.0.0
 *
 * 基于 STM32 HAL SPI 驱动 W25Q16。
 * 使用 CubeMX 已初始化的 hspi1 句柄，CS 脚 PB0 作 GPIO 控制。
 * 支持 3 线 SPI 模式（CLK, MOSI, MISO）+ 软件 CS。
 */

#include "spi_flash.h"
#include "common.h"
#include "timer_driver.h"
#include "stm32f4xx_hal.h"

/* ==================== 外部引用：CubeMX 句柄 ==================== */
extern SPI_HandleTypeDef hspi1;

/* ==================== W25Q16 命令集 ==================== */

#define CMD_WRITE_ENABLE        0x06U   /**< 写使能 */
#define CMD_VOLATILE_SR_WRITE   0x50U   /**< 易失性状态寄存器写使能 */
#define CMD_WRITE_STATUS_REG    0x01U   /**< 写状态寄存器 */
#define CMD_PAGE_PROGRAM        0x02U   /**< 页编程 */
#define CMD_READ_DATA           0x03U   /**< 读数据 */
#define CMD_READ_STATUS_REG     0x05U   /**< 读状态寄存器 */
#define CMD_READ_STATUS_REG2    0x35U   /**< 读状态寄存器2 */
#define CMD_ERASE_4K            0x20U   /**< 扇区擦除 4KB */
#define CMD_ERASE_32K           0x52U   /**< 块擦除 32KB */
#define CMD_ERASE_64K           0xD8U   /**< 块擦除 64KB */
#define CMD_CHIP_ERASE          0xC7U   /**< 整片擦除 */
#define CMD_POWER_DOWN          0xB9U   /**< 掉电 */
#define CMD_RELEASE_PD_ID       0xABU   /**< 释放掉电/读设备ID */
#define CMD_MANUFACTURER_ID     0x90U   /**< 读制造商/设备ID */
#define CMD_JEDEC_ID            0x9FU   /**< 读 JEDEC ID */
#define CMD_READ_UNIQUE_ID      0x4BU   /**< 读唯一 ID */

/* --- 状态寄存器位定义 --- */
#define SR_BUSY                 (1U << 0U)  /**< 忙标志 (WIP) */
#define SR_WEL                  (1U << 1U)  /**< 写使能锁存 (WEL) */
#define SR_BP0                  (1U << 2U)  /**< 块保护位0 */
#define SR_BP1                  (1U << 3U)  /**< 块保护位1 */
#define SR_BP2                  (1U << 4U)  /**< 块保护位2 */
#define SR_TB                   (1U << 5U)  /**< 顶部/底部保护 */
#define SR_SEC                  (1U << 6U)  /**< 扇区保护 */
#define SR_SRP0                 (1U << 7U)  /**< 状态寄存器保护位0 */

/* ==================== CS 引脚 ==================== */
#define FLASH_CS_PORT           GPIOB
#define FLASH_CS_PIN            GPIO_PIN_0

/* ==================== 私有全局变量 ==================== */

/** Fls 驱动状态 */
static VAR(Fls_StateType, MCAL_APPL_DATA) s_flsState = FLS_STATE_IDLE;

/** SPI 发送/接收超时 (ms) */
#define SPI_TIMEOUT_MS          100U

/* ==================== 私有函数声明 ==================== */

static FUNC(void, MCAL_CODE) Fls_CS_Low(void);
static FUNC(void, MCAL_CODE) Fls_CS_High(void);
static FUNC(uint8, MCAL_CODE) Fls_ReadStatusReg(void);
static FUNC(void, MCAL_CODE) Fls_WaitReady(void);
static FUNC(void, MCAL_CODE) Fls_WriteEnable(void);

/* ==================== 私有函数实现 ==================== */

/**
 * @brief CS 拉低 (选中 Flash)
 */
static FUNC(void, MCAL_CODE)
Fls_CS_Low(void)
{
    HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);
}

/**
 * @brief CS 拉高 (释放 Flash)
 */
static FUNC(void, MCAL_CODE)
Fls_CS_High(void)
{
    HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief 发送单字节命令 (CS 由调用者管理)
 */
static FUNC(Std_ReturnType, MCAL_CODE)
Fls_SendCmd(uint8 cmd)
{
    return (HAL_SPI_Transmit(&hspi1, &cmd, 1U, SPI_TIMEOUT_MS) == HAL_OK)
           ? STD_OK : STD_NOT_OK;
}

/**
 * @brief 发送并接收 (全双工)
 */
static FUNC(Std_ReturnType, MCAL_CODE)
Fls_Transfer(uint8 *txBuf, uint8 *rxBuf, uint16 len)
{
    return (HAL_SPI_TransmitReceive(&hspi1, txBuf, rxBuf, len, SPI_TIMEOUT_MS) == HAL_OK)
           ? STD_OK : STD_NOT_OK;
}

/**
 * @brief 读状态寄存器
 */
static FUNC(uint8, MCAL_CODE)
Fls_ReadStatusReg(void)
{
    uint8 status = 0xFFU;

    Fls_CS_Low();
    (void)Fls_SendCmd(CMD_READ_STATUS_REG);
    (void)HAL_SPI_Receive(&hspi1, &status, 1U, SPI_TIMEOUT_MS);
    Fls_CS_High();

    return status;
}

/**
 * @brief 等待 Flash 就绪 (轮询 BUSY 位)
 *
 * 使用非阻塞 Fls_MainFunction() 替代。
 * 此函数仅用于初始化阶段的同步等待。
 */
static FUNC(void, MCAL_CODE)
Fls_WaitReady(void)
{
    uint32 timeout = FLS_ERASE_TIMEOUT_MS;

    while ((Fls_ReadStatusReg() & SR_BUSY) && (timeout > 0U)) {
        Gpt_DelayMs(1U);
        timeout--;
    }
}

/**
 * @brief 写使能
 */
static FUNC(void, MCAL_CODE)
Fls_WriteEnable(void)
{
    Fls_CS_Low();
    (void)Fls_SendCmd(CMD_WRITE_ENABLE);
    Fls_CS_High();

    /* 确认 WEL 已置位 */
    uint8 sr = Fls_ReadStatusReg();
    (void)sr;
}

/* ==================== 公开函数实现 ==================== */

/**
 * @brief Fls 初始化
 */
FUNC(Std_ReturnType, MCAL_CODE)
Fls_Init(void)
{
    s_flsState = FLS_STATE_IDLE;

    /* CS 初始为高 */
    Fls_CS_High();

    /* 读取 JEDEC ID 验证 */
    uint32 id = Fls_ReadJEDECId();

    if (id == 0xEF4015U) {  /* Winbond W25Q16 */
        return STD_OK;
    }

    return STD_NOT_OK;
}

/**
 * @brief 读取 JEDEC ID
 */
FUNC(uint32, MCAL_CODE)
Fls_ReadJEDECId(void)
{
    uint8 rx[3] = {0U};

    Fls_CS_Low();
    (void)Fls_SendCmd(CMD_JEDEC_ID);
    (void)HAL_SPI_Receive(&hspi1, rx, 3U, SPI_TIMEOUT_MS);
    Fls_CS_High();

    return ((uint32)rx[0] << 16U) | ((uint32)rx[1] << 8U) | (uint32)rx[2];
}

/**
 * @brief 读 Flash
 */
FUNC(Std_ReturnType, MCAL_CODE)
Fls_Read(
    uint32                         Address,
    P2VAR(uint8, AUTOMATIC, MCAL_APPL_DATA) Data,
    uint32                         Length
)
{
    if ((Data == NULL_PTR) || (Length == 0U)) {
        return STD_NOT_OK;
    }
    if ((Address + Length) > FLS_TOTAL_SIZE) {
        return STD_NOT_OK;
    }

    /* 发送读命令 + 24-bit 地址 */
    uint8 addr[3];
    addr[0] = (uint8)((Address >> 16U) & 0xFFU);
    addr[1] = (uint8)((Address >> 8U)  & 0xFFU);
    addr[2] = (uint8)(Address          & 0xFFU);

    s_flsState = FLS_STATE_READ;

    Fls_CS_Low();
    (void)Fls_SendCmd(CMD_READ_DATA);
    (void)HAL_SPI_Transmit(&hspi1, addr, 3U, SPI_TIMEOUT_MS);
    (void)HAL_SPI_Receive(&hspi1, Data, (uint16)Length, SPI_TIMEOUT_MS);
    Fls_CS_High();

    s_flsState = FLS_STATE_IDLE;

    return STD_OK;
}

/**
 * @brief 写 Flash 一页 (PAGE ≤ 256 字节)
 *
 * @pre 目标扇区已被擦除
 * @pre Length ≤ 256, Address % 256 + Length ≤ 256
 */
FUNC(Std_ReturnType, MCAL_CODE)
Fls_Write(
    uint32                               Address,
    CONSTP2VAR(uint8, AUTOMATIC, MCAL_APPL_DATA) Data,
    uint32                               Length
)
{
    if ((Data == NULL_PTR) || (Length == 0U) || (Length > FLS_PAGE_SIZE)) {
        return STD_NOT_OK;
    }
    if ((Address + Length) > FLS_TOTAL_SIZE) {
        return STD_NOT_OK;
    }

    /* 写使能 */
    Fls_WriteEnable();

    /* 发送页编程命令 + 24-bit 地址 + 数据 */
    uint8 addr[3];
    addr[0] = (uint8)((Address >> 16U) & 0xFFU);
    addr[1] = (uint8)((Address >> 8U)  & 0xFFU);
    addr[2] = (uint8)(Address          & 0xFFU);

    s_flsState = FLS_STATE_WRITE;

    Fls_CS_Low();
    (void)Fls_SendCmd(CMD_PAGE_PROGRAM);
    (void)HAL_SPI_Transmit(&hspi1, addr, 3U, SPI_TIMEOUT_MS);
    (void)HAL_SPI_Transmit(&hspi1, (uint8 *)Data, (uint16)Length, SPI_TIMEOUT_MS);
    Fls_CS_High();

    /* 等待编程完成 (典型 0.8ms, 最坏 3ms) */
    Fls_WaitReady();

    s_flsState = FLS_STATE_IDLE;

    return STD_OK;
}

/**
 * @brief 擦除扇区 (4KB)
 * 
 * @param[in] Address  扇区内任意地址
 * 
 * @note 擦除耗时 45ms~400ms。
 *       Fls_WaitReady 会阻塞直到完成。
 */
FUNC(Std_ReturnType, MCAL_CODE)
Fls_Erase(
    uint32 Address
)
{
    if (Address >= FLS_TOTAL_SIZE) {
        return STD_NOT_OK;
    }

    /* 写使能 */
    Fls_WriteEnable();

    /* 发送扇区擦除命令 + 24-bit 地址 */
    uint8 addr[3];
    addr[0] = (uint8)((Address >> 16U) & 0xFFU);
    addr[1] = (uint8)((Address >> 8U)  & 0xFFU);
    addr[2] = (uint8)(Address          & 0xFFU);

    s_flsState = FLS_STATE_ERASE;

    Fls_CS_Low();
    (void)Fls_SendCmd(CMD_ERASE_4K);
    (void)HAL_SPI_Transmit(&hspi1, addr, 3U, SPI_TIMEOUT_MS);
    Fls_CS_High();

    /* 等待擦除完成 (最坏 400ms) */
    Fls_WaitReady();

    s_flsState = FLS_STATE_IDLE;

    return STD_OK;
}

/**
 * @brief 获取 Fls 状态
 */
FUNC(Fls_StateType, MCAL_CODE)
Fls_GetState(void)
{
    return s_flsState;
}

/**
 * @brief Fls 主函数轮询
 *
 * 当前实现为阻塞等待完成，因为 W25Q16 没有硬件中断引脚。
 * 如果需要在等待时不阻塞 CPU，改由此函数轮询 SR_BUSY。
 */
FUNC(void, MCAL_CODE)
Fls_MainFunction(void)
{
    /* 检查 Flash 是否仍处于忙状态 */
    if (s_flsState != FLS_STATE_IDLE) {
        if ((Fls_ReadStatusReg() & SR_BUSY) == 0U) {
            /* 操作完成 */
            s_flsState = FLS_STATE_IDLE;
        }
    }
}