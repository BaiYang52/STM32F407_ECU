/**
 * @file compiler.h
 * @brief AUTOSAR 编译器抽象层宏定义
 * @version 1.0.0
 *
 * 遵循 AUTOSAR_SWS_CompilerAbstraction 规范，提供：
 *   - FUNC / P2VAR / P2CONST / CONSTP2VAR / CONSTP2CONST
 *   - VAR / CONST
 *   - 各模块专用内存类 (MEMORY_CLASS)
 *
 * @note 当前实现映射到 PC-lint / GCC，不对齐到具体内存段。
 *       实际产品中需根据链接脚本调整内存类到指定段。
 */

#ifndef COMPILER_H
#define COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== AUTOSAR 编译器宏 ==================== */

/**
 * @def FUNC(rettype, memclass)
 * @brief 函数返回类型声明
 * @param rettype  返回值类型
 * @param memclass 函数所在内存段 (如 CAN_CODE, DCM_CODE)
 */
#define FUNC(rettype, memclass)  rettype

/**
 * @def P2VAR(ptrtype, memclass, ptrclass)
 * @brief 指向非常量数据的指针
 * @param ptrtype  指向的类型
 * @param memclass 指针变量所在内存段
 * @param ptrclass 被指数据所在内存段
 */
#define P2VAR(ptrtype, memclass, ptrclass)             ptrtype *

/**
 * @def P2CONST(ptrtype, memclass, ptrclass)
 * @brief 指向常量数据的指针（指针本身可写）
 */
#define P2CONST(ptrtype, memclass, ptrclass)           const ptrtype *

/**
 * @def CONSTP2VAR(ptrtype, memclass, ptrclass)
 * @brief 常量指针指向非常量数据
 */
#define CONSTP2VAR(ptrtype, memclass, ptrclass)        ptrtype *

/**
 * @def CONSTP2CONST(ptrtype, memclass, ptrclass)
 * @brief 常量指针指向常量数据
 */
#define CONSTP2CONST(ptrtype, memclass, ptrclass)      const ptrtype *

/**
 * @def VAR(vartype, memclass)
 * @brief 变量声明
 */
#define VAR(vartype, memclass)                         vartype

/**
 * @def CONST(consttype, memclass)
 * @brief 常量声明
 */
#define CONST(consttype, memclass)                     const consttype

/* ==================== 模块内存类定义 ==================== */

/* --- CAN 模块 --- */
#define CAN_CODE                     /**< CAN 模块代码段 */
#define CAN_APPL_CONST               /**< CAN 模块常量 */
#define CAN_APPL_DATA                /**< CAN 模块数据 */

/* --- DCM 模块 --- */
#define DCM_CODE                     /**< DCM 模块代码段 */
#define DCM_APPL_CONST               /**< DCM 模块常量 */
#define DCM_APPL_DATA                /**< DCM 模块数据 */

/* --- DEM 模块 --- */
#define DEM_CODE
#define DEM_APPL_CONST
#define DEM_APPL_DATA

/* --- NVM 模块 --- */
#define NVM_CODE
#define NVM_APPL_CONST
#define NVM_APPL_DATA

/* --- RTE 模块 --- */
#define RTE_CODE
#define RTE_APPL_DATA

/* --- OS 模块 --- */
#define OS_CODE
#define OS_APPL_DATA

/* --- MCAL 通用 --- */
#define MCAL_CODE
#define MCAL_APPL_CONST
#define MCAL_APPL_DATA

/* ==================== 常用 AUTOSAR 类型缩写 ==================== */

#define AUTOMATIC                  /**< 自动变量 (栈上) */
#define TYPEDEF                    /**< 类型定义 */

#ifdef __cplusplus
}
#endif

#endif /* COMPILER_H */