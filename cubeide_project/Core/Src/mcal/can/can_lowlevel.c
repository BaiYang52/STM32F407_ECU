/**
 * @file can_lowlevel.c
 * @brief CAN 底层寄存器操作实现
 * @version 1.0.0
 *
 * 提供对 STM32F407 bxCAN 控制器寄存器的直接操作，
 * 用于 Can_Init 之外的底层控制、故障恢复、位时序调整等场景。
 *
 * 参考: RM0090 Reference Manual §23 (bxCAN)
 */

#include "can_types.h"
#include "compiler.h"
#include "stm32f4xx_hal.h"

/* ==================== 外部句柄引用 ==================== */
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

/* ==================== 寄存器地址映射 ==================== */

/** CAN1 寄存器基地址 */
#define CAN1_BASE               0x40006400UL
/** CAN2 寄存器基地址 */
#define CAN2_BASE               0x40006800UL

/** bxCAN 主控制寄存器 (MCR) 偏移 */
#define CAN_MCR_OFFSET          0x00U
/** bxCAN 状态寄存器 (MSR) 偏移 */
#define CAN_MSR_OFFSET          0x04U
/** bxCAN 发送状态寄存器 (TSR) 偏移 */
#define CAN_TSR_OFFSET          0x08U
/** bxCAN 错误状态寄存器 (ESR) 偏移 */
#define CAN_ESR_OFFSET          0x18U
/** bxCAN 位时序寄存器 (BTR) 偏移 */
#define CAN_BTR_OFFSET          0x1CU

/* --- MCR 关键位 --- */
#define CAN_MCR_INRQ            (1U << 0U)      /**< 初始化请求 */
#define CAN_MCR_SLEEP           (1U << 1U)      /**< 睡眠模式请求 */
#define CAN_MCR_TXFP            (1U << 2U)      /**< 发送 FIFO 优先级 */
#define CAN_MCR_RFLM            (1U << 3U)      /**< 接收 FIFO 锁定模式 */
#define CAN_MCR_NART            (1U << 4U)      /**< 禁止自动重发 */
#define CAN_MCR_AWUM            (1U << 5U)      /**< 自动唤醒 */
#define CAN_MCR_ABOM            (1U << 6U)      /**< 自动 BusOff 管理 */
#define CAN_MCR_TTCM            (1U << 7U)      /**< 时间触发通信模式 */
#define CAN_MCR_RESET           (1U << 15U)     /**< 软件复位 */

/* --- MSR 关键位 --- */
#define CAN_MSR_INAK            (1U << 0U)      /**< 初始化确认 */
#define CAN_MSR_SLAK            (1U << 1U)      /**< 睡眠确认 */
#define CAN_MSR_ERRI            (1U << 2U)      /**< 错误中断标志 */
#define CAN_MSR_WKUI            (1U << 3U)      /**< 唤醒中断标志 */
#define CAN_MSR_SLAKI           (1U << 4U)      /**< 睡眠中断标志 */
#define CAN_MSR_TXM             (1U << 8U)      /**< 发送模式 */
#define CAN_MSR_RXM             (1U << 9U)      /**< 接收模式 */

/* --- ESR 关键位 --- */
#define CAN_ESR_EWGF            (1U << 0U)      /**< 错误警告标志 */
#define CAN_ESR_EPVF            (1U << 1U)      /**< 错误被动标志 */
#define CAN_ESR_BOFF            (1U << 2U)      /**< BusOff 标志 */
#define CAN_ESR_LEC_POS         4U              /**< 最后错误代码位偏移 */
#define CAN_ESR_LEC_MASK        (7U << 4U)
#define CAN_ESR_TEC_POS         16U             /**< 发送错误计数位偏移 */
#define CAN_ESR_REC_POS         24U             /**< 接收错误计数位偏移 */

/* --- BTR 关键位 --- */
#define CAN_BTR_BRP_POS         0U              /**< 波特率分频器位偏移 */
#define CAN_BTR_BRP_MASK        (0x3FU << 0U)
#define CAN_BTR_TS1_POS         16U             /**< 时间段1 位偏移 */
#define CAN_BTR_TS2_POS         20U             /**< 时间段2 位偏移 */
#define CAN_BTR_SJW_POS         24U             /**< 同步跳转宽度位偏移 */
#define CAN_BTR_LBKM            (1U << 30U)     /**< 回环模式 */
#define CAN_BTR_SILM            (1U << 31U)     /**< 静默模式 */

/* ==================== 私有辅助函数 ==================== */

/**
 * @brief 获取通道对应的寄存器基址
 */
static FUNC(uint32, CAN_CODE)
Can_GetBaseAddr(uint8 Channel)
{
    return (Channel == CAN_CHANNEL_1) ? CAN1_BASE : CAN2_BASE;
}

/**
 * @brief 读 32-bit 寄存器
 */
static FUNC(uint32, CAN_CODE)
Can_ReadReg(uint32 BaseAddr, uint32 Offset)
{
    return *(volatile uint32 *)(BaseAddr + Offset);
}

/**
 * @brief 写 32-bit 寄存器
 */
static FUNC(void, CAN_CODE)
Can_WriteReg(uint32 BaseAddr, uint32 Offset, uint32 Value)
{
    *(volatile uint32 *)(BaseAddr + Offset) = Value;
}

/**
 * @brief 修改寄存器的指定 bits (读-改-写)
 */
static FUNC(void, CAN_CODE)
Can_ModifyReg(uint32 BaseAddr, uint32 Offset, uint32 Mask, uint32 Value)
{
    uint32 reg = Can_ReadReg(BaseAddr, Offset);
    reg &= ~Mask;
    reg |= (Value & Mask);
    Can_WriteReg(BaseAddr, Offset, reg);
}

/* ==================== 公开函数 ==================== */

/**
 * @brief 将 CAN 控制器置于初始化模式 (INRQ=1)
 *
 * 在修改 BTR、过滤器等配置前，需先进入初始化模式。
 * 轮询直到 INAK=1，超时返回错误。
 *
 * @param[in] Channel CAN 通道号
 * @return Std_ReturnType
 *   @retval STD_OK      成功进入初始化模式
 *   @retval STD_NOT_OK  超时
 */
FUNC(Std_ReturnType, CAN_CODE)
Can_EnterInitMode(uint8 Channel)
{
    uint32 base = Can_GetBaseAddr(Channel);
    uint32 timeout = 1000U;  /* 约 1ms 超时 */

    /* 置位 INRQ */
    Can_WriteReg(base, CAN_MCR_OFFSET, CAN_MCR_INRQ);

    /* 等待 INAK 确认 */
    while ((Can_ReadReg(base, CAN_MSR_OFFSET) & CAN_MSR_INAK) == 0U) {
        if (--timeout == 0U) {
            return STD_NOT_OK;
        }
    }

    return STD_OK;
}

/**
 * @brief 将 CAN 控制器退出初始化模式 (INRQ=0)
 *
 * 配置完成后调用，进入正常工作模式。
 *
 * @param[in] Channel CAN 通道号
 * @return Std_ReturnType
 *   @retval STD_OK      成功退出
 *   @retval STD_NOT_OK  超时
 */
FUNC(Std_ReturnType, CAN_CODE)
Can_ExitInitMode(uint8 Channel)
{
    uint32 base = Can_GetBaseAddr(Channel);
    uint32 timeout = 1000U;

    /* 清零 INRQ */
    Can_WriteReg(base, CAN_MCR_OFFSET, 0U);

    /* 等待 INAK 清除 */
    while ((Can_ReadReg(base, CAN_MSR_OFFSET) & CAN_MSR_INAK) != 0U) {
        if (--timeout == 0U) {
            return STD_NOT_OK;
        }
    }

    return STD_OK;
}

/**
 * @brief 手动配置位时序 (BTR 寄存器)
 *
 * 覆盖 CubeMX 初始化的波特率配置。
 * 通常在需要动态切换波特率时使用。
 *
 * @param[in] Channel  CAN 通道号
 * @param[in] brp      波特率分频器 (1-64)
 * @param[in] ts1      时间段1 (1-16)
 * @param[in] ts2      时间段2 (1-8)
 * @param[in] sjw      同步跳转宽度 (1-4)
 */
FUNC(void, CAN_CODE)
Can_SetBitTiming(uint8 Channel, uint8 brp, uint8 ts1, uint8 ts2, uint8 sjw)
{
    uint32 base = Can_GetBaseAddr(Channel);
    uint32 btr = 0U;

    btr |= ((uint32)(brp - 1U) & 0x3FU) << CAN_BTR_BRP_POS;
    btr |= ((uint32)(ts1 - 1U) & 0xFU)  << CAN_BTR_TS1_POS;
    btr |= ((uint32)(ts2 - 1U) & 0x7U)  << CAN_BTR_TS2_POS;
    btr |= ((uint32)(sjw - 1U) & 0x3U)  << CAN_BTR_SJW_POS;

    Can_WriteReg(base, CAN_BTR_OFFSET, btr);
}

/**
 * @brief 设置回环模式 (用于自测)
 *
 * @param[in] Channel CAN 通道号
 * @param[in] enable  TRUE = 回环模式, FALSE = 正常模式
 */
FUNC(void, CAN_CODE)
Can_SetLoopbackMode(uint8 Channel, boolean enable)
{
    uint32 base = Can_GetBaseAddr(Channel);

    if (enable) {
        Can_ModifyReg(base, CAN_BTR_OFFSET, CAN_BTR_LBKM, CAN_BTR_LBKM);
    } else {
        Can_ModifyReg(base, CAN_BTR_OFFSET, CAN_BTR_LBKM, 0U);
    }
}

/**
 * @brief 获取 Tx 错误计数 (TEC)
 */
FUNC(uint8, CAN_CODE)
Can_GetTxErrorCount(uint8 Channel)
{
    uint32 base = Can_GetBaseAddr(Channel);
    uint32 esr  = Can_ReadReg(base, CAN_ESR_OFFSET);
    return (uint8)((esr >> CAN_ESR_TEC_POS) & 0xFFU);
}

/**
 * @brief 获取 Rx 错误计数 (REC)
 */
FUNC(uint8, CAN_CODE)
Can_GetRxErrorCount(uint8 Channel)
{
    uint32 base = Can_GetBaseAddr(Channel);
    uint32 esr  = Can_ReadReg(base, CAN_ESR_OFFSET);
    return (uint8)((esr >> CAN_ESR_REC_POS) & 0xFFU);
}

/**
 * @brief 获取最后错误代码 (LEC)
 *
 * @return 0=无错误, 1=位填充, 2=格式, 3=ACK, 4=隐性位, 5=显性位, 6=CRC, 7=保留
 */
FUNC(uint8, CAN_CODE)
Can_GetLastErrorCode(uint8 Channel)
{
    uint32 base = Can_GetBaseAddr(Channel);
    uint32 esr  = Can_ReadReg(base, CAN_ESR_OFFSET);
    return (uint8)((esr & CAN_ESR_LEC_MASK) >> CAN_ESR_LEC_POS);
}

/**
 * @brief 清除 CAN 错误状态 (ESR 寄存器)
 *
 * 当检测到 BusOff 或错误被动时，调用此函数复位 ESR 的错误位。
 * 配合 HAL_CAN_ResetError + 重新 Start 使用。
 */
FUNC(void, CAN_CODE)
Can_ClearErrorStatus(uint8 Channel)
{
    uint32 base = Can_GetBaseAddr(Channel);

    /* 清零 LEC 位 (写 0 清除) */
    Can_ModifyReg(base, CAN_ESR_OFFSET, CAN_ESR_LEC_MASK, 0U);
}

/**
 * @brief 软件复位 CAN 控制器
 *
 * 将 CAN 控制器复位到默认状态，相当于重新上电。
 */
FUNC(void, CAN_CODE)
Can_SoftwareReset(uint8 Channel)
{
    uint32 base = Can_GetBaseAddr(Channel);

    /* 置位 RESET 位 */
    Can_WriteReg(base, CAN_MCR_OFFSET, CAN_MCR_RESET);
    /* 延迟几个周期等待复位完成 */
    volatile uint32 delay = 10U;
    while (delay-- > 0U) {
        __NOP();
    }
    /* 清除 RESET 位 */
    Can_WriteReg(base, CAN_MCR_OFFSET, 0U);
}

/**
 * @brief 检查 CAN 控制器是否处于 BusOff
 */
FUNC(boolean, CAN_CODE)
Can_IsBusOff(uint8 Channel)
{
    uint32 base = Can_GetBaseAddr(Channel);
    return (Can_ReadReg(base, CAN_ESR_OFFSET) & CAN_ESR_BOFF) ? TRUE : FALSE;
}

/**
 * @brief 检查 CAN 控制器是否处于错误被动 (Error Passive)
 */
FUNC(boolean, CAN_CODE)
Can_IsErrorPassive(uint8 Channel)
{
    uint32 base = Can_GetBaseAddr(Channel);
    return (Can_ReadReg(base, CAN_ESR_OFFSET) & CAN_ESR_EPVF) ? TRUE : FALSE;
}

/**
 * @brief 检查 CAN 控制器是否处于错误警告 (Error Warning)
 */
FUNC(boolean, CAN_CODE)
Can_IsErrorWarning(uint8 Channel)
{
    uint32 base = Can_GetBaseAddr(Channel);
    return (Can_ReadReg(base, CAN_ESR_OFFSET) & CAN_ESR_EWGF) ? TRUE : FALSE;
}
