/**
 * @file spi_flash.h
 * @brief AUTOSAR Fls Driver 接口头文件 — W25Q16 外部 SPI Flash
 * @version 1.0.0
 *
 * 遵循 AUTOSAR_SWS_FLS 规范定义外部 Flash 驱动接口。
 * W25Q16 参数：
 *   - 容量: 2MB (16Mbit)
 *   - 扇区大小: 4KB
 *   - 页大小: 256B
 *   - 擦除时间(扇区): 典型 45ms, 最坏 400ms
 *   - 编程时间(页): 典型 0.8ms, 最坏 3ms
 *
 * @note 本驱动使用 CubeMX 已初始化的 hspi1 句柄 (extern)。
 *       片选 (CS) 引脚为 PB0，作普通 GPIO 控制。
 */

#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Includes ==================== */
#include "types.h"
#include "compiler.h"

/* ==================== 常量定义 ==================== */

/** W25Q16 容量 (2MB) */
#define FLS_TOTAL_SIZE            0x200000UL

/** 扇区大小 (4KB) */
#define FLS_SECTOR_SIZE           4096U

/** 页大小 (256B) */
#define FLS_PAGE_SIZE             256U

/** 扇区总数 */
#define FLS_SECTOR_COUNT          512U

/** 整片擦除超时 (ms) */
#define FLS_ERASE_TIMEOUT_MS      1000U

/** 页编程超时 (ms) */
#define FLS_WRITE_TIMEOUT_MS      100U

/* ==================== 类型定义 ==================== */

/**
 * @enum Fls_StateType
 * @brief Fls 驱动状态
 */
typedef enum
{
    FLS_STATE_IDLE = 0U,        /**< 空闲 */
    FLS_STATE_ERASE,            /**< 正在擦除 */
    FLS_STATE_WRITE,            /**< 正在写入 */
    FLS_STATE_READ,             /**< 正在读取 */
    FLS_STATE_ERROR             /**< 错误 */
} Fls_StateType;

/* ==================== 公开函数声明 ==================== */

/**
 * @brief Fls 驱动初始化
 *
 * 检测 W25Q16 的 JEDEC ID (0xEF4015) 验证 SPI 通信。
 *
 * @return Std_ReturnType
 *   @retval STD_OK      Flash 识别成功
 *   @retval STD_NOT_OK  识别失败 (SPI 通信问题)
 */
FUNC(Std_ReturnType, MCAL_CODE)
Fls_Init(void);

/**
 * @brief 读取 Flash 数据
 *
 * @param[in]  Address  字节地址 (0x000000 ~ 0x1FFFFF)
 * @param[out] Data     数据缓冲区指针
 * @param[in]  Length   读取长度 (字节)
 * @return Std_ReturnType
 *   @retval STD_OK      读取成功
 *   @retval STD_NOT_OK  参数错误
 */
FUNC(Std_ReturnType, MCAL_CODE)
Fls_Read(
    uint32                         Address,
    P2VAR(uint8, AUTOMATIC, MCAL_APPL_DATA) Data,
    uint32                         Length
);

/**
 * @brief 写入 Flash 一页 (非阻塞)
 *
 * 将数据写入一个 256B 页。调用者需保证：
 *   1. 目标扇区已被擦除
 *   2. 不超过页边界 (Address % 256 + Length ≤ 256)
 *   3. Length ≤ 256
 *
 * @param[in] Address  页内起始地址
 * @param[in] Data     数据缓冲区指针
 * @param[in] Length   写入长度 (1-256)
 * @return Std_ReturnType
 *   @retval STD_OK      写入请求已提交
 *   @retval STD_NOT_OK  参数错误
 */
FUNC(Std_ReturnType, MCAL_CODE)
Fls_Write(
    uint32                               Address,
    CONSTP2VAR(uint8, AUTOMATIC, MCAL_APPL_DATA) Data,
    uint32                               Length
);

/**
 * @brief 擦除一个扇区 (4KB)
 *
 * @param[in] Address  扇区内任意地址
 * @return Std_ReturnType
 *   @retval STD_OK      擦除请求已提交
 *   @retval STD_NOT_OK  地址越界
 */
FUNC(Std_ReturnType, MCAL_CODE)
Fls_Erase(
    uint32 Address
);

/**
 * @brief 获取 Fls 驱动状态
 */
FUNC(Fls_StateType, MCAL_CODE)
Fls_GetState(void);

/**
 * @brief Fls 主函数轮询 (非阻塞)
 *
 * 在 10ms 任务中调用，检查 Flash 操作是否完成。
 * 操作完成时自动清除状态回到 FLS_STATE_IDLE。
 */
FUNC(void, MCAL_CODE)
Fls_MainFunction(void);

/**
 * @brief 读取 JEDEC ID (用于诊断/验证)
 *
 * @return uint32 JEDEC ID (W25Q16 = 0xEF4015)
 */
FUNC(uint32, MCAL_CODE)
Fls_ReadJEDECId(void);

#ifdef __cplusplus
}
#endif

#endif /* SPI_FLASH_H */