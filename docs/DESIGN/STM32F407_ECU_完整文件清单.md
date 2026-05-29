# STM32F407 AUTOSAR ECU 项目 - 完整文件清单

## 1. 项目文件树完整版

```
STM32F407_ECU_Project/
│
├── 📁 src/                              # 源代码目录
│   ├── 📁 mcal/                        # MCAL层 (微控制器抽象层)
│   │   ├── 📁 can/
│   │   │   ├── can_driver.c            # CAN驱动核心实现
│   │   │   ├── can_interrupt.c         # CAN中断处理
│   │   │   ├── can_lowlevel.c          # CAN底层寄存器操作
│   │   │   └── can_buffer.c            # CAN环形缓冲区实现
│   │   │
│   │   ├── 📁 adc/
│   │   │   ├── adc_driver.c            # ADC驱动核心
│   │   │   ├── adc_interrupt.c         # ADC转换完成中断
│   │   │   └── adc_filter.c            # ADC数据滤波
│   │   │
│   │   ├── 📁 timer/
│   │   │   ├── timer_driver.c          # 通用定时器驱动
│   │   │   ├── timer_systick.c         # SysTick系统时钟 (1ms)
│   │   │   ├── timer_iwdg.c            # 独立看门狗驱动
│   │   │   └── timer_interrupt.c       # 定时器中断处理
│   │   │
│   │   ├── 📁 gpio/
│   │   │   ├── gpio_driver.c           # GPIO驱动实现
│   │   │   ├── gpio_interrupt.c        # GPIO外部中断处理
│   │   │   └── gpio_config.c           # GPIO引脚配置初始化
│   │   │
│   │   ├── 📁 uart/
│   │   │   ├── uart_driver.c           # UART通用驱动
│   │   │   ├── uart_debug.c            # 调试串口(USART1)
│   │   │   ├── uart_lin1.c             # LIN1主节点(USART2)
│   │   │   ├── uart_lin2.c             # LIN2备用(USART3)
│   │   │   └── uart_interrupt.c        # UART中断处理
│   │   │
│   │   ├── 📁 spi/
│   │   │   ├── spi_driver.c            # SPI驱动实现
│   │   │   ├── spi_flash.c             # W25Q16 Flash驱动
│   │   │   └── spi_interrupt.c         # SPI中断处理
│   │   │
│   │   ├── 📁 pwm/
│   │   │   ├── pwm_driver.c            # PWM驱动核心
│   │   │   ├── pwm_led.c               # LED PWM (TIM4_CH1)
│   │   │   └── pwm_motor.c             # 马达PWM (TIM4_CH2)
│   │   │
│   │   ├── 📁 nvm/
│   │   │   ├── eeprom_driver.c         # 外部EEPROM (W25Q16)
│   │   │   ├── flash_driver.c          # 内部Flash驱动
│   │   │   └── nvm_abstraction.c       # NVM抽象接口
│   │   │
│   │   ├── 📁 clock/
│   │   │   ├── clock_driver.c          # 时钟配置驱动
│   │   │   └── pll_config.c            # PLL参数配置
│   │   │
│   │   ├── 📁 power/
│   │   │   ├── power_manager.c         # 电源管理驱动
│   │   │   └── lowpower_mode.c         # 低功耗模式管理
│   │   │
│   │   └── 📁 startup/
│   │       └── startup_stm32f407ve.c  # STM32F407启动代码
│   │
│   ├── 📁 bsw/                         # BSW层 (基础软件层)
│   │   ├── 📁 com/
│   │   │   ├── com_manager.c           # 通信管理器 (信号打包/解包)
│   │   │   ├── com_signal_encode.c     # 信号编码
│   │   │   ├── com_signal_decode.c     # 信号解码
│   │   │   ├── com_e2e.c               # E2E保护 (滚动计数器+Checksum)
│   │   │   └── com_timeout.c           # 报文超时监测
│   │   │
│   │   ├── 📁 canif/
│   │   │   ├── canif_driver.c          # CAN接口驱动
│   │   │   ├── canif_handler.c         # CAN收发处理
│   │   │   └── canif_tx_buffer.c       # CAN发送缓冲管理
│   │   │
│   │   ├── 📁 pdur/
│   │   │   ├── pdur_router.c           # PDU路由核心
│   │   │   └── pdur_config.c           # PDU路由配置
│   │   │
│   │   ├── 📁 dcm/                     # UDS诊断服务模块
│   │   │   ├── dcm_main.c              # Dcm主处理逻辑
│   │   │   ├── dcm_service_0x10.c      # 0x10 会话控制
│   │   │   ├── dcm_service_0x11.c      # 0x11 ECU复位
│   │   │   ├── dcm_service_0x14.c      # 0x14 清除DTC
│   │   │   ├── dcm_service_0x19.c      # 0x19 读取DTC
│   │   │   ├── dcm_service_0x22.c      # 0x22 读DID (数据标识)
│   │   │   ├── dcm_service_0x2E.c      # 0x2E 写DID
│   │   │   ├── dcm_service_0x27.c      # 0x27 安全访问 (AES128)
│   │   │   ├── dcm_service_0x28.c      # 0x28 通信控制
│   │   │   ├── dcm_service_0x31.c      # 0x31 例行程序控制 (RID)
│   │   │   ├── dcm_service_0x34.c      # 0x34 请求下载 [FBL]
│   │   │   ├── dcm_service_0x36.c      # 0x36 传输数据 [FBL]
│   │   │   ├── dcm_service_0x37.c      # 0x37 请求传输退出 [FBL]
│   │   │   ├── dcm_service_0x85.c      # 0x85 DTC控制
│   │   │   ├── dcm_security.c          # AES128加密/验证算法
│   │   │   ├── dcm_did_manager.c       # DID (数据标识) 管理
│   │   │   ├── dcm_rid_manager.c       # RID (例行程序ID) 管理
│   │   │   ├── dcm_session_control.c   # 会话管理与状态机
│   │   │   └── dcm_timing.c            # 诊断定时管理 (P2/S3/S4)
│   │   │
│   │   ├── 📁 dem/                     # DTC故障检测与管理
│   │   │   ├── dem_main.c              # Dem主处理逻辑
│   │   │   ├── dem_dtc_manager.c       # DTC管理与存储
│   │   │   ├── dem_monitor.c           # 故障监控逻辑框架
│   │   │   ├── dem_button_stuck.c      # 0x010001 按键卡滞故障
│   │   │   ├── dem_busoff_monitor.c    # 0x050005 CAN总线关闭
│   │   │   ├── dem_overtemp_monitor.c  # 0x030003 温度过高故障
│   │   │   ├── dem_msg_loss_monitor.c  # 0x020002 报文丢失故障
│   │   │   ├── dem_crc_error_monitor.c # 0x060006 CRC错误故障
│   │   │   ├── dem_s2g_monitor.c       # 0x040004 按键短路故障
│   │   │   └── dem_storage.c           # DTC持久化存储到NVM
│   │   │
│   │   ├── 📁 cannm/                   # AUTOSAR网络管理
│   │   │   ├── cannm_main.c            # CanNm主处理逻辑
│   │   │   ├── cannm_sleep.c           # 休眠/唤醒逻辑
│   │   │   ├── cannm_wakeup.c          # 唤醒机制 (NM/IGN/Button)
│   │   │   ├── cannm_tx.c              # NM报文发送
│   │   │   ├── cannm_rx.c              # NM报文接收处理
│   │   │   └── cannm_timer.c           # CanNm定时管理
│   │   │
│   │   ├── 📁 nvm/
│   │   │   ├── nvm_manager.c           # NVM管理器 (异步调度)
│   │   │   ├── nvm_scheduler.c         # NVM异步调度逻辑
│   │   │   ├── nvm_did_storage.c       # DID存储管理
│   │   │   ├── nvm_dtc_storage.c       # DTC存储管理
│   │   │   └── nvm_config.c            # NVM配置与内存分配
│   │   │
│   │   ├── 📁 nm/
│   │   │   ├── nm_manager.c            # 通用网络管理
│   │   │   └── nm_state.c              # 网络状态机
│   │   │
│   │   └── bsw_config.c                # BSW全局配置
│   │
│   ├── 📁 rte/                         # RTE层 (运行时环境)
│   │   ├── rte_main.c                  # RTE主处理函数
│   │   ├── rte_signal_buffer.c         # 信号缓冲管理
│   │   ├── rte_read_interface.c        # Rte_Read_xxx实现
│   │   ├── rte_write_interface.c       # Rte_Write_xxx实现
│   │   ├── rte_scheduler.c             # RTE任务调度
│   │   └── rte_config.c                # RTE配置
│   │
│   ├── 📁 asw/                         # ASW层 (应用软件层)
│   │   ├── 📁 app_models/              # Simulink自动生成
│   │   │   ├── app_model_step_10ms.c   # 10ms模型步进
│   │   │   ├── app_model_step_100ms.c  # 100ms模型步进
│   │   │   ├── app_model_step_1s.c     # 1s模型步进
│   │   │   └── app_model_init.c        # 模型初始化
│   │   │
│   │   ├── app_motor_control.c         # 马达控制算法
│   │   ├── app_led_control.c           # LED控制算法
│   │   ├── app_overspeed_monitor.c     # 超速监测算法
│   │   └── app_thermal_manage.c        # 热管理算法
│   │
│   ├── 📁 app/                         # 应用主程序层
│   │   ├── main.c                      # 主程序入口点
│   │   ├── app_init.c                  # 系统初始化模块
│   │   ├── app_task_scheduler.c        # 10ms轮询任务调度器
│   │   ├── app_systick_handler.c       # SysTick 1ms中断处理
│   │   ├── app_can_handler.c           # CAN接收中断处理
│   │   ├── app_gpio_handler.c          # GPIO外部中断处理 (按键/唤醒)
│   │   ├── app_uart_handler.c          # UART接收中断处理
│   │   ├── app_adc_handler.c           # ADC转换完成中断
│   │   ├── app_hardfault_handler.c     # HardFault异常处理
│   │   ├── app_lowpower.c              # 低功耗/唤醒管理
│   │   └── app_watchdog.c              # 看门狗 (喂狗)
│   │
│   ├── 📁 fbl/                         # FBL层 (Flash Bootloader)
│   │   ├── fbl_main.c                  # FBL主程序
│   │   ├── fbl_init.c                  # FBL初始化
│   │   ├── fbl_flash_driver.c          # Flash擦写驱动
│   │   ├── fbl_download.c              # 0x34/0x36/0x37服务处理
│   │   ├── fbl_security.c              # FBL安全 (AES、密钥)
│   │   ├── fbl_dcm_service.c           # FBL诊断服务实现
│   │   ├── fbl_crc_check.c             # CRC32校验算法
│   │   ├── fbl_counter_manage.c        # 刷写次数计数器管理 (F501)
│   │   ├── fbl_app_validate.c          # APP有效性检查
│   │   ├── fbl_app_jump.c              # 跳转到APP执行
│   │   ├── fbl_recovery.c              # 掉电恢复逻辑
│   │   └── fbl_watchdog.c              # FBL看门狗管理
│   │
│   └── 📁 config/                      # 配置文件
│       ├── can_matrix.c                # CAN矩阵 (从DBC生成)
│       ├── com_config.c                # Com信号配置
│       ├── dcm_config.c                # Dcm诊断配置
│       ├── dem_config.c                # Dem故障配置
│       ├── cannm_config.c              # CanNm网络配置
│       ├── nvm_config.c                # NVM内存分配配置
│       ├── pin_config.c                # GPIO引脚配置
│       ├── system_config.c             # 系统全局配置
│       └── version.h                   # 版本信息
│
├── 📁 inc/                              # 头文件目录 (结构同src)
│   ├── 📁 mcal/
│   │   ├── 📁 can/
│   │   │   ├── can_driver.h            # CAN驱动头文件
│   │   │   ├── can_types.h             # CAN类型定义
│   │   │   └── can_reg.h               # CAN寄存器定义
│   │   ├── 📁 adc/
│   │   │   ├── adc_driver.h
│   │   │   └── adc_types.h
│   │   ├── 📁 timer/
│   │   │   ├── timer_driver.h
│   │   │   └── systick.h
│   │   ├── ... (省略其他MCAL头文件)
│   │   └── mcal_types.h                # MCAL全局类型定义
│   │
│   ├── 📁 bsw/
│   │   ├── 📁 com/
│   │   │   ├── com_manager.h
│   │   │   ├── com_types.h
│   │   │   ├── com_signal.h
│   │   │   └── com_e2e.h
│   │   ├── 📁 canif/
│   │   │   ├── canif_driver.h
│   │   │   ├── canif_types.h
│   │   │   └── can_matrix.h            # CAN矩阵定义
│   │   ├── 📁 dcm/
│   │   │   ├── dcm_main.h
│   │   │   ├── dcm_service.h           # 诊断服务接口
│   │   │   ├── dcm_types.h
│   │   │   ├── dcm_did.h               # DID定义表
│   │   │   ├── dcm_rid.h               # RID定义表
│   │   │   ├── dcm_security.h
│   │   │   └── dcm_timing.h
│   │   ├── 📁 dem/
│   │   │   ├── dem_main.h
│   │   │   ├── dem_types.h
│   │   │   ├── dem_dtc.h               # DTC编号定义 (0x010001~0x060006)
│   │   │   └── dem_config.h
│   │   ├── 📁 cannm/
│   │   │   ├── cannm_main.h
│   │   │   ├── cannm_types.h
│   │   │   └── cannm_config.h
│   │   ├── 📁 nvm/
│   │   │   ├── nvm_manager.h
│   │   │   └── nvm_types.h
│   │   └── bsw_types.h                 # BSW全局类型定义
│   │
│   ├── 📁 rte/
│   │   ├── rte.h                       # RTE主头文件
│   │   ├── rte_types.h
│   │   ├── rte_interface.h             # Rte_Read/Write接口
│   │   ├── rte_signal_map.h            # 信号映射表
│   │   └── rte_config.h
│   │
│   ├── 📁 asw/
│   │   ├── app_models.h                # Simulink模型声明
│   │   ├── app_types.h
│   │   ├── app_config.h
│   │   └── app_interface.h             # 应用接口
│   │
│   ├── 📁 app/
│   │   ├── main.h
│   │   ├── app_config.h
│   │   ├── app_common.h                # 全局宏定义
│   │   ├── app_task.h                  # 任务定义
│   │   ├── app_systick.h
│   │   ├── app_interrupt.h
│   │   └── app_lowpower.h
│   │
│   ├── 📁 fbl/
│   │   ├── fbl.h
│   │   ├── fbl_types.h
│   │   ├── fbl_config.h
│   │   ├── fbl_memory.h                # 内存布局定义
│   │   ├── fbl_security.h
│   │   ├── fbl_dcm.h
│   │   └── fbl_error.h                 # FBL错误码定义
│   │
│   ├── 📁 config/
│   │   ├── can_matrix.h
│   │   ├── com_config.h
│   │   ├── dcm_config.h
│   │   ├── dem_config.h
│   │   ├── system_config.h
│   │   └── version.h
│   │
│   ├── types.h                         # 全局标准类型定义
│   ├── common.h                        # 通用宏和函数
│   ├── compiler.h                      # 编译器相关定义
│   ├── os.h                            # OS调度相关
│   └── error_codes.h                   # 统一错误码定义
│
├── 📁 test/                             # 单元测试目录
│   ├── 📁 unit/
│   │   ├── 📁 test_mcal/
│   │   │   ├── test_can_driver.c       # CAN驱动单元测试
│   │   │   ├── test_can_send.c
│   │   │   ├── test_can_receive.c
│   │   │   ├── test_adc_driver.c
│   │   │   ├── test_timer_driver.c
│   │   │   └── test_gpio_driver.c
│   │   │
│   │   ├── 📁 test_bsw/
│   │   │   ├── test_dcm_service_0x22.c  # 读DID测试
│   │   │   ├── test_dcm_service_0x27.c  # 安全访问测试
│   │   │   ├── test_dem_dtc.c           # DTC故障管理测试
│   │   │   ├── test_dem_button_stuck.c  # 按键卡滞故障测试
│   │   │   ├── test_dem_overtemp.c      # 过温故障测试
│   │   │   ├── test_com_signal.c        # 通信信号测试
│   │   │   └── test_cannm_sleep.c       # 网络管理休眠测试
│   │   │
│   │   ├── 📁 test_asw/
│   │   │   ├── test_motor_control.c     # 马达控制单元测试
│   │   │   ├── test_overspeed_monitor.c # 超速监测单元测试
│   │   │   └── test_led_control.c
│   │   │
│   │   └── 📁 test_app/
│   │       ├── test_task_scheduler.c    # 任务调度器测试
│   │       ├── test_lowpower.c          # 低功耗模式测试
│   │       └── test_watchdog.c
│   │
│   ├── 📁 integration/
│   │   ├── test_can_communication.c     # CAN通信集成测试
│   │   ├── test_uds_diagnostic.c        # UDS诊断集成测试
│   │   ├── test_dtc_trigger_recovery.c  # DTC触发与恢复集成测试
│   │   └── test_end_to_end.c            # 端到端集成测试
│   │
│   ├── 📁 mock/                        # Mock对象库
│   │   ├── mock_can_driver.h
│   │   ├── mock_can_driver.c
│   │   ├── mock_adc_driver.h
│   │   ├── mock_adc_driver.c
│   │   ├── mock_nvm_driver.h
│   │   ├── mock_nvm_driver.c
│   │   └── mock_setup.h                # 通用Mock设置
│   │
│   ├── 📁 fixtures/                    # 测试数据
│   │   ├── can_test_frames.h           # CAN测试报文数据
│   │   ├── dtc_test_data.h             # DTC测试数据
│   │   ├── uds_test_packets.h          # UDS诊断测试包
│   │   └── did_test_values.h           # DID测试值表
│   │
│   ├── CMakeLists.txt                  # 测试编译配置
│   └── README.md                       # 测试说明文档
│
├── 📁 cubeide_project/                  # STM32CubeIDE工程
│   ├── stm32f407ve_cfg.ioc             # CubeMX配置文件
│   ├── .project                        # IDE项目文件
│   ├── 📁 Core/
│   │   ├── 📁 Inc/
│   │   │   ├── main.h
│   │   │   ├── stm32f4xx_it.h          # 中断处理头文件
│   │   │   └── stm32f4xx_hal_conf.h
│   │   └── 📁 Src/
│   │       ├── main.c
│   │       └── stm32f4xx_it.c
│   └── 📁 Drivers/
│       ├── 📁 STM32F4xx_HAL_Driver/
│       └── 📁 CMSIS/
│
├── 📁 matlab_simulink/                  # Simulink模型工程
│   ├── 📁 models/
│   │   ├── MotorControl_Model.slx      # 马达控制模型
│   │   ├── OverspeedMonitor_Model.slx  # 超速监测模型
│   │   └── MainModel.slx               # 主模型
│   │
│   ├── 📁 codegen/                     # 自动生成的代码
│   │   └── stm32f407/
│   │
│   └── 📁 configs/                     # Simulink配置
│       ├── coder_config.m              # 代码生成配置
│       └── simulink_settings.m
│
├── 📁 tools/                            # 工具脚本
│   ├── 📁 scripts/
│   │   ├── init_project.sh             # 项目初始化脚本
│   │   ├── build.sh                    # 编译脚本
│   │   ├── clean.sh                    # 清理脚本
│   │   ├── run_tests.sh                # 测试运行脚本
│   │   └── flash.sh                    # 固件烧写脚本
│   │
│   ├── 📁 python/
│   │   ├── can_matrix_gen.py           # CAN矩阵代码生成工具
│   │   ├── dbc_parser.py               # DBC文件解析器
│   │   ├── dcm_did_gen.py              # DID定义生成工具
│   │   └── test_data_gen.py            # 测试数据生成器
│   │
│   └── 📁 cmakelists/
│       ├── arm_toolchain.cmake
│       └── testing.cmake
│
├── 📁 docs/                             # 文档目录
│   ├── 📁 SRS/                         # 需求规格说明
│   │   ├── 汽车电子原型ECU项目需求规格.md
│   │   ├── Functional_Requirements.md
│   │   └── Non_Functional_Requirements.md
│   │
│   ├── 📁 API/                         # API文档
│   │   ├── MCAL_API.md                 # MCAL层API
│   │   ├── BSW_API.md                  # BSW层API
│   │   ├── RTE_API.md                  # RTE层API
│   │   └── ASW_API.md                  # ASW层API
│   │
│   ├── 📁 DESIGN/                      # 设计文档
│   │   ├── Architecture.md             # 整体架构设计
│   │   ├── AUTOSAR_Mapping.md          # AUTOSAR映射说明
│   │   ├── CAN_Matrix.md               # CAN矩阵设计
│   │   ├── DTC_Design.md               # DTC故障代码设计
│   │   ├── DID_Design.md               # DID数据标识设计
│   │   ├── UDS_Service_Design.md       # UDS诊断服务设计
│   │   ├── FBL_Design.md               # Bootloader设计
│   │   └── Task_Schedule.md            # 任务调度设计
│   │
│   ├── 📁 TESTING/
│   │   ├── Unit_Test_Plan.md           # 单元测试计划
│   │   ├── Integration_Test_Plan.md    # 集成测试计划
│   │   ├── Test_Coverage.md            # 测试覆盖率要求
│   │   └── Test_Report_Template.md     # 测试报告模板
│   │
│   └── 📁 TOOLS/
│       ├── Build_Guide.md              # 编译构建指南
│       ├── Debugging_Guide.md          # 调试指南
│       ├── CAN_DBC_Guide.md            # DBC文件处理指南
│       └── Development_Environment.md  # 开发环境配置
│
├── 📁 build/                            # 编译产物目录 (gitignore)
│   ├── firmware_app.elf
│   ├── firmware_app.hex
│   ├── firmware_app.bin
│   ├── firmware_app.map
│   └── ... (其他编译文件)
│
├── 📁 .vscode/                         # VSCode配置
│   ├── settings.json                   # IDE设置
│   ├── launch.json                     # 调试配置
│   ├── tasks.json                      # 任务配置
│   └── extensions.json                 # 推荐扩展
│
├── 📁 .github/                         # GitHub配置
│   └── 📁 workflows/
│       ├── build.yml                   # CI/CD构建流程
│       └── test.yml                    # CI/CD测试流程
│
├── CMakeLists.txt                      # CMake配置文件
├── .gitignore                          # Git忽略文件
├── .clang-format                       # 代码格式化配置
├── .editorconfig                       # 编辑器配置
├── Doxyfile                            # Doxygen配置
├── README.md                           # 项目说明
└── LICENSE                             # 许可证
```

---

## 2. 关键文件大小估算

| 文件类别 | 数量 | 平均大小 | 合计 |
|--------|------|--------|------|
| MCAL源文件 | 25+ | 2-3KB | 60KB |
| BSW源文件 | 35+ | 2-5KB | 130KB |
| RTE源文件 | 5+ | 1-2KB | 8KB |
| ASW源文件 | 8+ | 1-2KB | 12KB |
| 应用主程序 | 10+ | 1-3KB | 20KB |
| FBL源文件 | 12+ | 2-4KB | 35KB |
| 头文件 | 80+ | 1-3KB | 150KB |
| 单元测试 | 20+ | 3-5KB | 80KB |
| 配置文件 | 15+ | 1-2KB | 20KB |
| **总计** | **215+** | - | **515KB+** |

---

## 3. 编译输出产物

```
build/
├── firmware_app.elf       # 应用固件 (ELF格式, ~100-150KB)
├── firmware_app.hex       # 应用固件 (Intel HEX格式, 可烧写)
├── firmware_app.bin       # 应用固件 (二进制格式)
├── firmware_app.map       # 符号地址映射表
├── firmware_fbl.elf       # Bootloader固件
├── firmware_fbl.hex
├── firmware_fbl.bin
└── CMakeFiles/            # CMake临时文件

Flash布局:
├── 0x0800_0000 ~ 0x0801_0000: FBL (64KB)
└── 0x0801_0000 ~ 0x0808_0000: APP (448KB)

RAM布局:
├── 0x2000_0000 ~ 0x2000_B000: 数据段 (44KB)
├── 0x2000_B000 ~ 0x2000_FFFF: 堆和栈 (20KB)
└── CCRAM (64KB): FBL/APP共享区域
```

---

## 4. 文件命名速查表

### MCAL层
```
can_driver.c             CAN核心驱动实现
can_interrupt.c          CAN中断处理
can_types.h              CAN类型定义
can_driver.h             CAN驱动头文件
adc_driver.c             ADC驱动实现
timer_systick.c          SysTick系统时钟驱动
uart_debug.c             调试串口(USART1)
uart_lin1.c              LIN1驱动(USART2)
spi_flash.c              W25Q16 SPI Flash驱动
pwm_led.c                LED PWM驱动(TIM4_CH1)
pwm_motor.c              马达PWM驱动(TIM4_CH2)
```

### BSW层
```
com_manager.c            通信管理器
com_e2e.c                E2E保护(滚动计数+校验)
canif_driver.c           CAN接口驱动
dcm_main.c               诊断服务主处理
dcm_service_0x22.c       读DID服务
dcm_service_0x2E.c       写DID服务
dcm_service_0x27.c       安全访问(AES128)
dcm_did.h                DID定义表 (F190/F200等)
dem_main.c               故障检测与管理
dem_dtc.h                DTC编号定义
dem_button_stuck.c       按键卡滞监控
dem_overtemp_monitor.c   温度过高监控
cannm_main.c             AUTOSAR网络管理
cannm_sleep.c            休眠/唤醒逻辑
nvm_manager.c            非易失性存储管理
```

### RTE层
```
rte_main.c               RTE主处理
rte_interface.c          Rte_Read/Write接口实现
rte_interface.h          Rte_Read/Write接口声明
rte_signal_map.h         信号映射表
```

### ASW层
```
app_model_step_10ms.c    Simulink 10ms模型步进
app_motor_control.c      马达控制算法
app_overspeed_monitor.c  超速监测算法
```

### 应用主程序
```
main.c                   主程序入口
app_init.c               系统初始化
app_task_scheduler.c     任务调度器(10ms轮询)
app_systick_handler.c    SysTick中断处理(1ms)
app_lowpower.c           低功耗/唤醒管理
app_watchdog.c           看门狗管理
```

### FBL层
```
fbl_main.c               FBL主程序
fbl_flash_driver.c       Flash擦写驱动
fbl_download.c           0x34/0x36/0x37处理
fbl_dcm_service.c        FBL诊断服务
fbl_crc_check.c          CRC32校验
fbl_counter_manage.c     刷写计数器管理
fbl_app_validate.c       APP有效性检查
fbl_security.c           AES128加密/验证
```

### 配置文件
```
can_matrix.c             CAN矩阵(从DBC生成)
can_matrix.h             CAN信号定义
system_config.c          系统全局配置
pin_config.c             GPIO引脚配置
version.h                版本信息
```

---

## 5. 单元测试文件命名

```
test_can_driver.c              CAN驱动单元测试
test_can_send.c                CAN发送功能测试
test_can_receive.c             CAN接收功能测试
test_dcm_service_0x22.c        读DID测试
test_dcm_service_0x27.c        安全访问测试
test_dem_button_stuck.c        按键卡滞故障测试
test_dem_overtemp.c            过温故障测试
test_motor_control.c           马达控制单元测试
test_task_scheduler.c          任务调度器测试
test_can_communication.c       CAN通信集成测试
test_uds_diagnostic.c          UDS诊断集成测试
test_dtc_trigger_recovery.c    DTC触发与恢复测试

mock_can_driver.h              CAN驱动Mock对象
mock_can_driver.c              CAN驱动Mock实现
mock_adc_driver.h              ADC驱动Mock对象
mock_nvm_driver.h              NVM驱动Mock对象
can_test_frames.h              CAN测试报文数据
dtc_test_data.h                DTC测试数据
uds_test_packets.h             UDS诊断测试包
```

---

## 6. 头文件包含关系

```
main.c
├── app_init.h
├── app_task_scheduler.h
│   ├── rte.h
│   ├── bsw_dcm.h
│   ├── bsw_dem.h
│   ├── bsw_cannm.h
│   └── bsw_nvm.h
├── app_lowpower.h
│   ├── mcal_power_manager.h
│   └── mcal_can_driver.h
└── app_watchdog.h
    └── mcal_iwdg.h

bsw_dcm.h (诊断服务)
├── dcm_service.h
├── dcm_did.h             # DID定义表
├── dcm_rid.h             # RID定义表
├── dcm_security.h        # AES128
├── dcm_timing.h
└── canif_driver.h

bsw_dem.h (故障管理)
├── dem_dtc.h             # DTC定义
├── dem_monitor.h
└── nvm_manager.h

rte_interface.h           # RTE接口
├── rte_types.h
├── rte_signal_map.h
└── com_manager.h         # 通信管理
```

---

## 7. 版本信息文件示例 (inc/config/version.h)

```c
/**
 * @file version.h
 * @brief 固件版本信息定义
 */

#ifndef VERSION_H
#define VERSION_H

#define APP_VERSION_MAJOR    1
#define APP_VERSION_MINOR    0
#define APP_VERSION_PATCH    0
#define APP_BUILD_NUMBER     1

#define APP_VERSION_STRING   "1.0.0-build001"
#define APP_BUILD_DATE       __DATE__
#define APP_BUILD_TIME       __TIME__

#define FBL_VERSION_MAJOR    1
#define FBL_VERSION_MINOR    0
#define FBL_VERSION_PATCH    0

#endif /* VERSION_H */
```

---

**本清单包含了STM32F407 AUTOSAR ECU项目的完整文件参考，共215+个源代码文件，总计515KB+代码量。**
