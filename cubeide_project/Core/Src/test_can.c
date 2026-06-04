/**
 * @file test_can.c
 * @brief CAN通信测试
 * @version 1.0.0
 */

#include "main.h"
#include "stdio.h"
#include "string.h"

/* CAN 测试状态 */
typedef enum {
    CAN_TEST_IDLE = 0,
    CAN_TEST_TX_LOOP,      /* 循环发送固定数据 */
    CAN_TEST_RX_WAIT,      /* 等待接收 */
    CAN_TEST_ECHO,         /* 回显测试 */
} CAN_TestStateType;

static CAN_TestStateType can_state = CAN_TEST_IDLE;
static uint32_t can_rx_count = 0;
static uint32_t can_tx_count = 0;

/* CAN过滤器配置 */
static CAN_FilterTypeDef sFilterConfig = {0};

/**
 * @brief CAN滤波器配置
 * @details 配置接收所有消息
 */
static void Test_CAN_ConfigFilter(void)
{
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;

    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
        printf("[CAN] 滤波器配置失败!\n");
        return;
    }

    printf("[CAN] 滤波器配置完成，接收所有消息\n");
}

/**
 * @brief CAN初始化测试
 */
void Test_CAN_Init(void)
{
    printf("[CAN] CAN1初始化...\n");

    /* 配置过滤器 */
    Test_CAN_ConfigFilter();

    /* 启动CAN */
    if (HAL_CAN_Start(&hcan1) != HAL_OK) {
        printf("[CAN] 启动失败!\n");
        return;
    }
    printf("[CAN] CAN1启动成功\n");

    /* 启用接收中断 */
    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        printf("[CAN] 启用接收中断失败!\n");
        return;
    }
    printf("[CAN] 接收中断已启用\n");

    can_state = CAN_TEST_TX_LOOP;  /* 默认循环发送模式 */
}

/**
 * @brief CAN发送测试
 * @param[in] id CAN消息ID
 * @param[in] data 数据指针
 * @param[in] dlc 数据长度 (0-8)
 */
void Test_CAN_Send(uint32_t id, const uint8_t *data, uint8_t dlc)
{
    CAN_TxHeaderTypeDef tx_header = {0};
    uint8_t tx_data[8];
    uint32_t tx_mailbox;

    /* 配置发送头 */
    tx_header.StdId = id;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = dlc;

    /* 复制数据 */
    memcpy(tx_data, data, dlc);

    /* 发送 */
    if (HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox) == HAL_OK) {
        can_tx_count++;
//        printf("[CAN] 发送成功 (ID: 0x%03X, DLC: %d, 总数: %lu)\n", id, dlc, can_tx_count);
    } else {
        printf("[CAN] 发送失败! (ID: 0x%03lX)\n", id);
    }
}


/**
 * @brief CAN 100ms 周期任务
 * @details 根据测试模式执行不同的CAN操作
 */
void Test_CAN_100ms_Task(void)
{
    static uint8_t tx_counter = 0;
    switch (can_state) {
        case CAN_TEST_TX_LOOP: {
            /* 循环发送测试帧 */
            uint8_t data[8] = {
                0x01, 0x02, 0x03, 0x04,
                0x05, 0x06, 0x07, (uint8_t)tx_counter++
            };
            Test_CAN_Send(0x123, data, 8);  /* 发送 ID=0x123 */
            break;
        }

        case CAN_TEST_RX_WAIT:
            /* 只接收，不主动发送 */
            printf("[CAN] 等待接收... (已收 %lu 帧)\n", can_rx_count);
            break;

        case CAN_TEST_ECHO:
            printf("[CAN] 回显模式 (发 %lu, 收 %lu)\n", can_tx_count, can_rx_count);
            break;

        default:
            break;
    }
}

/**
 * @brief 设置CAN测试模式
 * @param[in] state 测试状态
 */
void Test_CAN_SetMode(CAN_TestStateType state)
{
    can_state = state;
    printf("[CAN] 切换测试模式: %d\n", state);
}

/**
 * @brief 获取CAN统计信息
 */
void Test_CAN_GetStats(uint32_t *tx_count, uint32_t *rx_count)
{
    if (tx_count) *tx_count = can_tx_count;
    if (rx_count) *rx_count = can_rx_count;
}
