/**
 * @file test_can_driver.c
 * @brief CAN驱动单元测试
 * @version 1.0.0
 * @date 2024-01-01
 */

#include "CUnit/Basic.h"
#include "can_driver.h"
#include "mock_can_driver.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ============= Test Suite Setup ============= */

/**
 * @brief 测试初始化
 */
int setUp(void)
{
    /* 初始化Mock对象 */
    mock_can_init_setup();
    return 0;
}

/**
 * @brief 测试清理
 */
int tearDown(void)
{
    /* 清理Mock对象 */
    return 0;
}

/* ============= Test Cases ============= */

/**
 * @brief 测试CAN驱动初始化 - 成功case
 */
void test_mcal_can_init_success(void)
{
    Can_ConfigType config;
    config.baudrate = 500; /* 500kbps */
    config.channel = CAN_CHANNEL_1;

    Std_ReturnType result = Mcal_Can_Init(&config);

    CU_ASSERT_EQUAL(result, STD_OK);
    CU_ASSERT_EQUAL(mock_can_init_called, TRUE);
    CU_ASSERT_EQUAL(mock_can_baudrate, 500);
}

/**
 * @brief 测试CAN驱动初始化 - 参数为NULL
 */
void test_mcal_can_init_null_config(void)
{
    Std_ReturnType result = Mcal_Can_Init(NULL);

    CU_ASSERT_EQUAL(result, STD_NOT_OK);
}

/**
 * @brief 测试CAN驱动初始化 - 无效波特率
 */
void test_mcal_can_init_invalid_baudrate(void)
{
    Can_ConfigType config;
    config.baudrate = 999; /* 无效波特率 */
    config.channel = CAN_CHANNEL_1;

    Std_ReturnType result = Mcal_Can_Init(&config);

    CU_ASSERT_EQUAL(result, STD_NOT_OK);
}

/**
 * @brief 测试CAN消息发送 - 成功case
 */
void test_mcal_can_send_success(void)
{
    Can_FrameType frame;
    frame.id = 0x123;
    frame.dlc = 8;
    frame.data[0] = 0x01;

    Std_ReturnType result = Mcal_Can_Send(CAN_CHANNEL_1, &frame);

    CU_ASSERT_EQUAL(result, STD_OK);
    CU_ASSERT_EQUAL(mock_can_tx_count, 1);
}

/**
 * @brief 测试CAN消息发送 - 缓冲区满
 */
void test_mcal_can_send_buffer_full(void)
{
    Can_FrameType frame;

    /* 填满发送缓冲区 */
    for (int i = 0; i < 64; i++)
    {
        Mcal_Can_Send(CAN_CHANNEL_1, &frame);
    }

    /* 再次发送应该失败 */
    Std_ReturnType result = Mcal_Can_Send(CAN_CHANNEL_1, &frame);

    CU_ASSERT_EQUAL(result, STD_NOT_OK);
}

/**
 * @brief 测试CAN消息接收 - 成功case
 */
void test_mcal_can_receive_success(void)
{
    Can_FrameType sendFrame, recvFrame;
    sendFrame.id = 0x456;
    sendFrame.dlc = 4;
    sendFrame.data[0] = 0xAA;
    sendFrame.data[1] = 0xBB;
    sendFrame.data[2] = 0xCC;
    sendFrame.data[3] = 0xDD;

    /* 模拟接收 (通过Mock) */
    mock_can_receive_data(&sendFrame);

    /* 读取接收到的数据 */
    Std_ReturnType result = Mcal_Can_Receive(CAN_CHANNEL_1, &recvFrame);

    CU_ASSERT_EQUAL(result, STD_OK);
    CU_ASSERT_EQUAL(recvFrame.id, 0x456);
    CU_ASSERT_EQUAL(recvFrame.dlc, 4);
    CU_ASSERT_EQUAL(recvFrame.data[0], 0xAA);
}

/**
 * @brief 测试CAN消息接收 - 缓冲区空
 */
void test_mcal_can_receive_buffer_empty(void)
{
    Can_FrameType recvFrame;

    Std_ReturnType result = Mcal_Can_Receive(CAN_CHANNEL_1, &recvFrame);

    CU_ASSERT_EQUAL(result, STD_NOT_OK);
}

/* ============= Test Suite Registration ============= */

int main(void)
{
    CU_pSuite pSuite = NULL;

    /* 初始化CUnit框架 */
    if (CU_initialize_registry() != CUE_SUCCESS)
    {
        return CU_get_error();
    }

    /* 创建测试套件 */
    pSuite = CU_add_suite("Can_Driver_Suite", setUp, tearDown);
    if (pSuite == NULL)
    {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* 添加测试用例 */
    CU_add_test(pSuite, "test_mcal_can_init_success", test_mcal_can_init_success);
    CU_add_test(pSuite, "test_mcal_can_init_null_config", test_mcal_can_init_null_config);
    CU_add_test(pSuite, "test_mcal_can_init_invalid_baudrate", test_mcal_can_init_invalid_baudrate);
    CU_add_test(pSuite, "test_mcal_can_send_success", test_mcal_can_send_success);
    CU_add_test(pSuite, "test_mcal_can_send_buffer_full", test_mcal_can_send_buffer_full);
    CU_add_test(pSuite, "test_mcal_can_receive_success", test_mcal_can_receive_success);
    CU_add_test(pSuite, "test_mcal_can_receive_buffer_empty", test_mcal_can_receive_buffer_empty);

    /* 运行测试 */
    CU_basic_run_tests();

    /* 打印测试报告 */
    CU_basic_show_failures(CU_get_failure_list());

    /* 清理 */
    CU_cleanup_registry();

    return CU_get_error();
}