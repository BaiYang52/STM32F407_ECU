/**
 * @file test_nvm.c
 * @brief W25Q16 外部Flash NVM读写测试
 * @version 1.0.0
 */

#include "main.h"
#include "stdio.h"
#include "string.h"

/* W25Q16 常量 */
#define W25Q16_SECTOR_SIZE      4096        /* 扇区大小 4KB */
#define W25Q16_PAGE_SIZE        256         /* 页大小 256B */
#define W25Q16_TOTAL_SIZE       0x200000    /* 总容量 2MB */

/* W25Q16 SPI 命令 */
#define W25Q16_CMD_READ         0x03        /* 读数据 */
#define W25Q16_CMD_WRITE        0x02        /* 页编程 */
#define W25Q16_CMD_ERASE_4K     0x20        /* 扇区擦除 4KB */
#define W25Q16_CMD_READ_STATUS  0x05        /* 读状态寄存器 */
#define W25Q16_CMD_WRITE_ENABLE 0x06        /* 写使能 */
#define W25Q16_CMD_READ_ID      0x9F        /* 读JEDEC ID */

/* NVM 内存分配 */
#define NVM_DID_ADDR            0x1000      /* DID 存储地址 */
#define NVM_DTC_ADDR            0x2000      /* DTC 存储地址 */
#define NVM_COUNTER_ADDR        0x3000      /* 刷写计数器地址 */

/**
 * @brief 片选控制 (GPIO)
 * @param[in] level GPIO_PIN_SET 或 GPIO_PIN_RESET
 */
static void Test_NVM_CS_Control(GPIO_PinState level)
{
    HAL_GPIO_WritePin(W25Q16_CS_GPIO_Port, W25Q16_CS_Pin, level);
}

/**
 * @brief SPI传输
 * @param[in] cmd 命令
 * @param[in] tx_data 发送数据
 * @param[in] tx_len 发送长度
 * @param[out] rx_data 接收数据
 * @param[in] rx_len 接收长度
 */
static HAL_StatusTypeDef Test_NVM_SPI_Transfer(uint8_t cmd,
                                                const uint8_t *tx_data, uint16_t tx_len,
                                                uint8_t *rx_data, uint16_t rx_len)
{
    HAL_StatusTypeDef ret;
    uint8_t cmd_byte = cmd;

    /* CS 低电平，选中芯片 */
    Test_NVM_CS_Control(GPIO_PIN_RESET);

    /* 发送命令 */
    ret = HAL_SPI_Transmit(&hspi1, &cmd_byte, 1, 100);
    if (ret != HAL_OK) {
        Test_NVM_CS_Control(GPIO_PIN_SET);
        return ret;
    }

    /* 发送数据 (地址等) */
    if (tx_data && tx_len > 0) {
        ret = HAL_SPI_Transmit(&hspi1, (uint8_t *)tx_data, tx_len, 100);
        if (ret != HAL_OK) {
            Test_NVM_CS_Control(GPIO_PIN_SET);
            return ret;
        }
    }

    /* 接收数据 */
    if (rx_data && rx_len > 0) {
        ret = HAL_SPI_Receive(&hspi1, rx_data, rx_len, 100);
        if (ret != HAL_OK) {
            Test_NVM_CS_Control(GPIO_PIN_SET);
            return ret;
        }
    }

    /* CS 高电平，释放芯片 */
    Test_NVM_CS_Control(GPIO_PIN_SET);

    return HAL_OK;
}

/**
 * @brief 读取W25Q16 JEDEC ID
 * @return ID值
 */
static uint32_t Test_NVM_ReadID(void)
{
    uint8_t id[3] = {0};

    Test_NVM_SPI_Transfer(W25Q16_CMD_READ_ID, NULL, 0, id, 3);

    uint32_t jedec_id = (id[0] << 16) | (id[1] << 8) | id[2];

    return jedec_id;
}

/**
 * @brief 读取状态寄存器
 * @return 状态值
 */
static uint8_t Test_NVM_ReadStatus(void)
{
    uint8_t status = 0;

    Test_NVM_SPI_Transfer(W25Q16_CMD_READ_STATUS, NULL, 0, &status, 1);

    return status;
}

/**
 * @brief 等待Flash就绪
 */
static void Test_NVM_WaitReady(void)
{
    /* WIP位=0表示就绪 */
    while (Test_NVM_ReadStatus() & 0x01) {
        HAL_Delay(1);
    }
}

/**
 * @brief 写使能
 */
static void Test_NVM_WriteEnable(void)
{
    uint8_t dummy = 0;
    Test_NVM_SPI_Transfer(W25Q16_CMD_WRITE_ENABLE, NULL, 0, &dummy, 0);
}

/**
 * @brief 读Flash数据
 * @param[in] address 地址
 * @param[out] data 数据缓冲
 * @param[in] length 长度
 */
static HAL_StatusTypeDef Test_NVM_Read(uint32_t address, uint8_t *data, uint16_t length)
{
    uint8_t addr_bytes[3];

    /* 地址转为3字节 (24-bit地址) */
    addr_bytes[0] = (address >> 16) & 0xFF;
    addr_bytes[1] = (address >> 8) & 0xFF;
    addr_bytes[2] = address & 0xFF;

    return Test_NVM_SPI_Transfer(W25Q16_CMD_READ, addr_bytes, 3, data, length);
}

/**
 * @brief 写Flash数据 (需要先擦除)
 * @param[in] address 地址
 * @param[in] data 数据
 * @param[in] length 长度 (必须 ≤ 256)
 */
static HAL_StatusTypeDef Test_NVM_WritePage(uint32_t address, const uint8_t *data, uint16_t length)
{
    HAL_StatusTypeDef ret;
    uint8_t addr_bytes[3];

    if (length > W25Q16_PAGE_SIZE) {
        return HAL_ERROR;  /* 单页最多256字节 */
    }

    /* 写使能 */
    Test_NVM_WriteEnable();

    /* 地址转为3字节 */
    addr_bytes[0] = (address >> 16) & 0xFF;
    addr_bytes[1] = (address >> 8) & 0xFF;
    addr_bytes[2] = address & 0xFF;

    /* CS 低 */
    Test_NVM_CS_Control(GPIO_PIN_RESET);

    /* 发送写命令 */
    uint8_t cmd = W25Q16_CMD_WRITE;
    ret = HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    if (ret != HAL_OK) {
        Test_NVM_CS_Control(GPIO_PIN_SET);
        return ret;
    }

    /* 发送地址 */
    ret = HAL_SPI_Transmit(&hspi1, addr_bytes, 3, 100);
    if (ret != HAL_OK) {
        Test_NVM_CS_Control(GPIO_PIN_SET);
        return ret;
    }

    /* 发送数据 */
    ret = HAL_SPI_Transmit(&hspi1, (uint8_t *)data, length, 100);
    if (ret != HAL_OK) {
        Test_NVM_CS_Control(GPIO_PIN_SET);
        return ret;
    }

    /* CS 高 */
    Test_NVM_CS_Control(GPIO_PIN_SET);

    /* 等待写入完成 */
    Test_NVM_WaitReady();

    return HAL_OK;
}

/**
 * @brief 擦除扇区
 * @param[in] address 扇区地址
 */
static HAL_StatusTypeDef Test_NVM_EraseSector(uint32_t address)
{
    HAL_StatusTypeDef ret;
    uint8_t addr_bytes[3];

    /* 写使能 */
    Test_NVM_WriteEnable();

    /* 地址转为3字节 */
    addr_bytes[0] = (address >> 16) & 0xFF;
    addr_bytes[1] = (address >> 8) & 0xFF;
    addr_bytes[2] = address & 0xFF;

    /* CS 低 */
    Test_NVM_CS_Control(GPIO_PIN_RESET);

    /* 发送擦除命令 */
    uint8_t cmd = W25Q16_CMD_ERASE_4K;
    ret = HAL_SPI_Transmit(&hspi1, &cmd, 1, 100);
    if (ret != HAL_OK) {
        Test_NVM_CS_Control(GPIO_PIN_SET);
        return ret;
    }

    /* 发送地址 */
    ret = HAL_SPI_Transmit(&hspi1, addr_bytes, 3, 100);
    if (ret != HAL_OK) {
        Test_NVM_CS_Control(GPIO_PIN_SET);
        return ret;
    }

    /* CS 高 */
    Test_NVM_CS_Control(GPIO_PIN_SET);

    /* 等待擦除完成 */
    Test_NVM_WaitReady();

    return HAL_OK;
}

/**
 * @brief NVM初始化
 */
void Test_NVM_Init(void)
{
    printf("[NVM] W25Q16初始化...\n");

    /* 读取JEDEC ID验证SPI通信 */
    uint32_t id = Test_NVM_ReadID();
    printf("[NVM] JEDEC ID: 0x%06lX\n", id);

    if (id == 0xEF4015) {
        printf("[NVM] W25Q16识别成功 (EF=Winbond, 40=16M)\n");
    } else {
        printf("[NVM] W25Q16识别失败! 可能SPI通信有问题\n");
        return;
    }

    printf("[NVM] 初始化完成\n");
}

/**
 * @brief NVM 1000ms 周期任务
 * @details 定期测试读写
 */
void Test_NVM_1000ms_Task(void)
{
    static uint32_t test_counter = 0;

    test_counter++;

    if (test_counter >= 3) {  /* 每3秒执行一次测试 */
        test_counter = 0;

        printf("[NVM] ========== NVM 读写测试 ==========\n");

        /* 测试1: 写入VIN */
        uint8_t vin_data[17] = "N00000000000001\0";  /* 17字节 */
        printf("[NVM] 写入VIN: %s\n", vin_data);
        Test_NVM_EraseSector(NVM_DID_ADDR);
        Test_NVM_WritePage(NVM_DID_ADDR, vin_data, 17);

        /* 读回VIN */
        uint8_t vin_read[17] = {0};
        Test_NVM_Read(NVM_DID_ADDR, vin_read, 17);
        printf("[NVM] 读取VIN: %s\n", vin_read);

        /* 测试2: 写入计数器 */
        uint8_t counter_data[2] = {0x03, 0xE8};  /* 1000 */
        printf("[NVM] 写入刷写计数器: %d\n",
               (counter_data[0] << 8) | counter_data[1]);
        Test_NVM_EraseSector(NVM_COUNTER_ADDR);
        Test_NVM_WritePage(NVM_COUNTER_ADDR, counter_data, 2);

        /* 读回计数器 */
        uint8_t counter_read[2] = {0};
        Test_NVM_Read(NVM_COUNTER_ADDR, counter_read, 2);
        uint16_t counter_val = (counter_read[0] << 8) | counter_read[1];
        printf("[NVM] 读取刷写计数器: %d\n", counter_val);

        printf("[NVM] 测试完成\n");
    }
}
