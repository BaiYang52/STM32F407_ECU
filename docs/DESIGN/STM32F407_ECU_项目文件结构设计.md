# STM32F407 AUTOSAR ECU 项目文件结构与命名规范

## 1. 项目整体架构

### 1.1 目录树结构

```
STM32F407_ECU_Project/
├── .vscode/                          # VSCode配置
│   ├── settings.json
│   ├── launch.json
│   └── tasks.json
├── docs/                             # 文档目录
│   ├── SRS/                          # 需求规格说明
│   ├── API/                          # API文档
│   └── DESIGN/                       # 设计文档
├── tools/                            # 工具脚本
│   ├── scripts/
│   ├── python/
│   └── cmakelists/
├── cubeide_project/                  # STM32CubeIDE工程
│   ├── stm32f407ve_cfg.ioc          # CubeMX配置文件
│   ├── Drivers/                     # MCAL驱动(由CubeMX生成)
│   ├── Core/                        # 核心驱动代码
│   └── .project
├── matlab_simulink/                 # Simulink模型
│   ├── models/
│   ├── codegen/                    # 自动生成的代码
│   └── configs/
├── src/                             # 主源代码目录
│   ├── mcal/                       # 微控制器抽象层
│   ├── bsw/                        # 基础软件层
│   ├── rte/                        # 运行时环境
│   ├── asw/                        # 应用软件层
│   ├── app/                        # 应用主程序
│   ├── fbl/                        # Bootloader代码
│   └── config/                     # 配置文件
├── inc/                            # 头文件目录(同src结构)
│   ├── mcal/
│   ├── bsw/
│   ├── rte/
│   ├── asw/
│   ├── app/
│   ├── fbl/
│   └── config/
├── test/                           # 单元测试
│   ├── unit/
│   ├── integration/
│   ├── mock/
│   └── fixtures/
├── build/                          # 编译输出(gitignore)
├── CMakeLists.txt                 # CMake构建配置
├── .gitignore
└── README.md
```

---

## 2. 详细文件结构和命名规范

### 2.1 MCAL 层 (微控制器抽象层)

**职责**: 封装STM32硬件驱动，通过BSW调用

**源文件目录**: `src/mcal/`
**头文件目录**: `inc/mcal/`

#### 文件列表与命名规则

```
src/mcal/
├── can/
│   ├── can_driver.c               # CAN驱动实现
│   ├── can_interrupt.c            # CAN中断处理
│   └── can_lowlevel.c             # CAN底层操作
├── adc/
│   ├── adc_driver.c               # ADC驱动实现
│   └── adc_interrupt.c            # ADC中断处理
├── timer/
│   ├── timer_driver.c             # 通用定时器驱动
│   ├── timer_systick.c            # SysTick驱动(系统时钟)
│   └── timer_iwdg.c               # 独立看门狗驱动
├── gpio/
│   ├── gpio_driver.c              # GPIO驱动实现
│   └── gpio_interrupt.c           # GPIO外部中断处理
├── uart/
│   ├── uart_driver.c              # UART通用驱动
│   ├── uart_debug.c               # 调试串口(USART1)
│   ├── uart_lin1.c                # LIN1驱动(USART2)
│   └── uart_lin2.c                # LIN2驱动(USART3)
├── spi/
│   ├── spi_driver.c               # SPI驱动实现
│   └── spi_flash.c                # SPI Flash(W25Q16)驱动
├── pwm/
│   ├── pwm_driver.c               # PWM驱动实现
│   ├── pwm_led.c                  # LED PWM(TIM4_CH1)
│   └── pwm_motor.c                # 马达PWM(TIM4_CH2)
├── nvm/
│   ├── eeprom_driver.c            # 外部EEPROM(W25Q16)驱动
│   ├── flash_driver.c             # 内部Flash驱动
│   └── nvm_abstraction.c          # NVM抽象接口
├── clock/
│   ├── clock_driver.c             # 时钟配置驱动
│   └── pll_config.c               # PLL配置
├── power/
│   ├── power_manager.c            # 电源管理驱动
│   └── lowpower_mode.c            # 低功耗模式管理
└── startup/
    └── startup_stm32f407ve.c      # 启动代码

inc/mcal/
├── can/
│   ├── can_driver.h
│   ├── can_types.h                # CAN类型定义
│   └── can_reg.h                  # CAN寄存器定义
├── adc/
│   ├── adc_driver.h
│   └── adc_types.h
├── timer/
│   ├── timer_driver.h
│   ├── timer_types.h
│   └── systick.h
├── gpio/
│   ├── gpio_driver.h
│   └── gpio_config.h              # GPIO引脚配置
├── uart/
│   ├── uart_driver.h
│   └── uart_types.h
├── spi/
│   ├── spi_driver.h
│   └── spi_types.h
├── pwm/
│   ├── pwm_driver.h
│   └── pwm_types.h
├── nvm/
│   ├── nvm_driver.h
│   ├── eeprom.h
│   └── flash.h
├── clock/
│   └── clock_driver.h
├── power/
│   └── power_manager.h
└── mcal_types.h                   # MCAL全局类型定义
```

#### MCAL 文件命名规则

| 文件类型 | 命名格式 | 示例 | 说明 |
|--------|--------|------|------|
| 驱动实现 | `{模块}_driver.c` | `can_driver.c` | 驱动核心实现 |
| 驱动头文件 | `{模块}_driver.h` | `can_driver.h` | 驱动对外接口 |
| 中断处理 | `{模块}_interrupt.c` | `can_interrupt.c` | 中断服务例程 |
| 类型定义 | `{模块}_types.h` | `can_types.h` | 结构体、枚举定义 |
| 底层操作 | `{模块}_lowlevel.c` | `can_lowlevel.c` | 寄存器底层操作 |
| 寄存器定义 | `{模块}_reg.h` | `can_reg.h` | 寄存器宏定义 |

#### MCAL 关键函数命名规则

```c
// 初始化函数: Mcal_{Module}_{Feature}_Init
Std_ReturnType Mcal_Can_Driver_Init(const Can_ConfigType *Config);
Std_ReturnType Mcal_Adc_Channel_Init(uint8 ChannelId);

// 读写函数: Mcal_{Module}_{Operation}
Std_ReturnType Mcal_Can_Send(uint32 CanId, const uint8 *Data, uint8 Dlc);
Std_ReturnType Mcal_Adc_Read(uint8 ChannelId, uint16 *Value);

// 中断处理: Mcal_{Module}_ISR
void Mcal_Can_RxISR(void);
void Mcal_Timer_SystickISR(void);

// 使能/禁能: Mcal_{Module}_{Feature}_Enable/Disable
void Mcal_Can_Receiver_Enable(void);
void Mcal_Iwdg_Enable(void);
```

---

### 2.2 BSW 层 (基础软件层)

**职责**: 通信协议栈、诊断、网络管理、故障管理

**源文件目录**: `src/bsw/`
**头文件目录**: `inc/bsw/`

```
src/bsw/
├── com/                           # 通信模块 (ISO 14229)
│   ├── com_manager.c              # Com管理器 (信号打包/解包)
│   ├── com_signal_encode.c        # 信号编码
│   ├── com_signal_decode.c        # 信号解码
│   ├── com_e2e.c                  # E2E保护(滚动计数器+校验)
│   └── com_timeout.c              # 报文超时监测
├── canif/                         # CAN接口层
│   ├── canif_driver.c             # CAN接口驱动
│   ├── canif_handler.c            # CAN收发处理
│   └── canif_tx_buffer.c          # CAN发送缓冲
├── pdur/                          # PDU路由器
│   ├── pdur_router.c              # PDU路由核心
│   └── pdur_config.c              # PDU路由配置
├── dcm/                           # UDS诊断服务
│   ├── dcm_main.c                 # Dcm主处理
│   ├── dcm_service_0x10.c         # 会话控制(0x10)
│   ├── dcm_service_0x11.c         # ECU复位(0x11)
│   ├── dcm_service_0x14.c         # 清除DTC(0x14)
│   ├── dcm_service_0x19.c         # 读DTC(0x19)
│   ├── dcm_service_0x22.c         # 读DID(0x22)
│   ├── dcm_service_0x2E.c         # 写DID(0x2E)
│   ├── dcm_service_0x27.c         # 安全访问(0x27)
│   ├── dcm_service_0x28.c         # 通信控制(0x28)
│   ├── dcm_service_0x31.c         # 例行程序控制(0x31)
│   ├── dcm_service_0x34.c         # 请求下载(0x34) [FBL]
│   ├── dcm_service_0x36.c         # 传输数据(0x36) [FBL]
│   ├── dcm_service_0x37.c         # 请求传输退出(0x37) [FBL]
│   ├── dcm_security.c             # AES128加密/验证
│   ├── dcm_did_manager.c          # DID管理
│   ├── dcm_rid_manager.c          # RID管理
│   ├── dcm_session_control.c      # 会话管理
│   └── dcm_timing.c               # 诊断定时管理
├── dem/                           # 故障管理
│   ├── dem_main.c                 # Dem主处理
│   ├── dem_dtc_manager.c          # DTC管理
│   ├── dem_monitor.c              # 故障监控逻辑
│   ├── dem_button_stuck.c         # 按键卡滞监控
│   ├── dem_busoff_monitor.c       # Busoff监控
│   ├── dem_overtemp_monitor.c     # 过温监控
│   ├── dem_msg_loss_monitor.c     # 报文丢失监控
│   ├── dem_crc_error_monitor.c    # CRC错误监控
│   ├── dem_s2g_monitor.c          # 短路到地监控
│   └── dem_storage.c              # DTC存储到NVM
├── cannm/                         # AUTOSAR网络管理
│   ├── cannm_main.c               # CanNm主处理
│   ├── cannm_sleep.c              # 休眠/唤醒逻辑
│   ├── cannm_wakeup.c             # 唤醒机制
│   ├── cannm_tx.c                 # NM报文发送
│   ├── cannm_rx.c                 # NM报文接收
│   └── cannm_timer.c              # CanNm定时管理
├── nvm/                           # 非易失性存储管理
│   ├── nvm_manager.c              # NVM管理器
│   ├── nvm_scheduler.c            # NVM异步调度
│   ├── nvm_did_storage.c          # DID存储管理
│   ├── nvm_dtc_storage.c          # DTC存储管理
│   └── nvm_config.c               # NVM配置
├── nm/                            # 通用网络管理
│   ├── nm_manager.c               # 网络管理主模块
│   └── nm_state.c                 # 网络状态机
└── bsw_config.c                   # BSW全局配置文件

inc/bsw/
├── com/
│   ├── com_manager.h
│   ├── com_types.h
│   ├── com_signal.h
│   └── com_e2e.h
├── canif/
│   ├── canif_driver.h
│   ├── canif_types.h
│   └── can_matrix.h               # CAN矩阵定义(从DBC生成)
├── pdur/
│   ├── pdur_router.h
│   └── pdur_types.h
├── dcm/
│   ├── dcm_main.h
│   ├── dcm_service.h
│   ├── dcm_types.h
│   ├── dcm_did.h                  # DID定义
│   ├── dcm_rid.h                  # RID定义
│   ├── dcm_security.h
│   └── dcm_timing.h
├── dem/
│   ├── dem_main.h
│   ├── dem_types.h
│   ├── dem_dtc.h                  # DTC编号定义
│   └── dem_config.h
├── cannm/
│   ├── cannm_main.h
│   ├── cannm_types.h
│   └── cannm_config.h
├── nvm/
│   ├── nvm_manager.h
│   └── nvm_types.h
├── nm/
│   └── nm_types.h
└── bsw_types.h                    # BSW全局类型定义
```

#### BSW 关键函数命名规则

```c
// 初始化: Bsw_{Module}_Init
Std_ReturnType Bsw_Dcm_Init(void);
Std_ReturnType Bsw_Dem_Init(void);

// 主处理(周期任务): Bsw_{Module}_MainFunction
void Bsw_Dcm_MainFunction(void);
void Bsw_Com_MainFunction(void);
void Bsw_CanNm_MainFunction(void);

// 接收回调: Bsw_{Module}_RxIndication
void Bsw_CanIf_RxIndication(uint32 CanId, const uint8 *Data, uint8 Dlc);
void Bsw_Com_RxIndication(const Com_SignalType *Signal);

// 发送确认: Bsw_{Module}_TxConfirmation
void Bsw_CanIf_TxConfirmation(uint32 CanId);
void Bsw_Com_TxConfirmation(const Com_SignalType *Signal);

// 操作: Bsw_{Module}_{Operation}
Std_ReturnType Bsw_Dem_SetEventStatus(uint16 EventId, Dem_EventStatusType Status);
Std_ReturnType Bsw_Nvm_WriteBlock(uint16 BlockId, const uint8 *Data);
```

---

### 2.3 RTE 层 (运行时环境)

**职责**: ASW与BSW的隔离桥梁，提供Rte_Read/Write接口

**源文件目录**: `src/rte/`
**头文件目录**: `inc/rte/`

```
src/rte/
├── rte_main.c                     # RTE主处理函数
├── rte_signal_buffer.c            # 信号缓冲管理
├── rte_read_interface.c           # Rte_Read_xxx实现
├── rte_write_interface.c          # Rte_Write_xxx实现
├── rte_scheduler.c                # RTE任务调度
└── rte_config.c                   # RTE配置

inc/rte/
├── rte.h                          # RTE主头文件
├── rte_types.h                    # RTE类型定义
├── rte_interface.h                # Rte_Read/Write接口定义
├── rte_signal_map.h               # 信号映射定义
└── rte_config.h                   # RTE配置头文件
```

#### RTE 关键函数命名规则

```c
// 初始化: Rte_Init
Std_ReturnType Rte_Init(void);

// 主处理: Rte_MainFunction
void Rte_MainFunction(void);

// 读取信号: Rte_Read_{ComponentName}_{PortName}_{DataElementName}
Std_ReturnType Rte_Read_VehicleCtrl_Port_Vehicle_Speed(float32 *Data);
Std_ReturnType Rte_Read_VehicleCtrl_Port_IGN_Status(uint8 *Data);

// 写入信号: Rte_Write_{ComponentName}_{PortName}_{DataElementName}
Std_ReturnType Rte_Write_EcuStatus_Port_Motor_Switch(uint8 Value);
Std_ReturnType Rte_Write_EcuStatus_Port_LED_Brightness(uint8 Value);

// 缓冲管理: Rte_{Operation}
void Rte_BufferWrite(uint16 SignalId, const uint8 *Data, uint8 Length);
Std_ReturnType Rte_BufferRead(uint16 SignalId, uint8 *Data, uint8 Length);
```

---

### 2.4 ASW 层 (应用软件层)

**职责**: Simulink自动生成的应用算法

**源文件目录**: `src/asw/`
**头文件目录**: `inc/asw/`

```
src/asw/
├── app_models/                    # Simulink生成的模型代码
│   ├── app_model_step_10ms.c      # 10ms模型步进
│   ├── app_model_step_100ms.c     # 100ms模型步进
│   ├── app_model_step_1s.c        # 1s模型步进
│   └── app_model_init.c           # 模型初始化
├── app_motor_control.c            # 马达控制应用
├── app_led_control.c              # LED控制应用
├── app_overspeed_monitor.c        # 超速监测应用
└── app_thermal_manage.c           # 热管理应用

inc/asw/
├── app_models.h                   # Simulink模型声明
├── app_types.h                    # 应用类型定义
├── app_config.h                   # 应用配置
└── app_interface.h                # 应用接口(与RTE交互)
```

#### ASW 关键函数命名规则

```c
// 模型步进: App_{Feature}_Step_{Period}
void App_MotorControl_Step_10ms(void);
void App_OverspeedMonitor_Step_10ms(void);

// 应用初始化: App_{Feature}_Init
Std_ReturnType App_MotorControl_Init(void);

// 应用处理: App_{Feature}_Process
void App_OverspeedMonitor_Process(float32 Speed, uint8 *OverSpeedFlag);
```

---

### 2.5 应用主程序层

**职责**: 主任务调度、初始化、中断管理

**源文件目录**: `src/app/`
**头文件目录**: `inc/app/`

```
src/app/
├── main.c                         # 主程序入口
├── app_init.c                     # 应用初始化模块
├── app_task_scheduler.c           # 任务调度器(10ms轮询)
├── app_systick_handler.c          # SysTick中断处理(1ms)
├── app_can_handler.c              # CAN中断处理
├── app_gpio_handler.c             # GPIO中断处理(按键/唤醒)
├── app_uart_handler.c             # UART中断处理(调试)
├── app_adc_handler.c              # ADC中断处理
├── app_hardfault_handler.c        # HardFault异常处理
├── app_lowpower.c                 # 低功耗/唤醒管理
└── app_watchdog.c                 # 看门狗喂狗管理

inc/app/
├── main.h
├── app_config.h                   # 应用全局配置
├── app_common.h                   # 应用全局宏定义
├── app_task.h                     # 任务定义与接口
├── app_systick.h
├── app_interrupt.h
└── app_lowpower.h
```

#### 应用主程序关键函数命名规则

```c
// 初始化: App_System_Init / App_{Module}_Init
void App_System_Init(void);
Std_ReturnType App_PowerManagement_Init(void);

// 主任务: App_Task_{Period}ms
void App_Task_10ms(void);      // 10ms周期任务
void App_Task_100ms(void);     // 100ms周期任务
void App_Task_1000ms(void);    // 1s周期任务

// 中断处理: App_{Source}_ISR / App_{Source}_Handler
void App_SysTick_ISR(void);
void App_CAN_RxISR(void);
void SysTick_Handler(void);    // ARM标准中断处理器
void CAN1_RX0_IRQHandler(void);

// 看门狗: App_Watchdog_{Operation}
void App_Watchdog_Feed(void);
void App_Watchdog_Init(void);
```

---

### 2.6 Bootloader (FBL) 层

**职责**: Flash刷写、安全校验、应用启动

**源文件目录**: `src/fbl/`
**头文件目录**: `inc/fbl/`

```
src/fbl/
├── fbl_main.c                     # FBL主程序
├── fbl_init.c                     # FBL初始化
├── fbl_flash_driver.c             # Flash擦写驱动
├── fbl_download.c                 # 0x34/0x36/0x37服务处理
├── fbl_security.c                 # FBL安全(AES、密钥)
├── fbl_dcm_service.c              # FBL诊断服务(0x10/0x34等)
├── fbl_crc_check.c                # CRC32校验
├── fbl_counter_manage.c           # 刷写次数计数器管理
├── fbl_app_validate.c             # APP有效性检查
├── fbl_app_jump.c                 # 跳转到APP
├── fbl_recovery.c                 # 掉电恢复逻辑
└── fbl_watchdog.c                 # FBL看门狗

inc/fbl/
├── fbl.h
├── fbl_types.h
├── fbl_config.h
├── fbl_memory.h                   # 内存布局定义
├── fbl_security.h
├── fbl_dcm.h
└── fbl_error.h                    # FBL错误码定义
```

#### FBL 关键函数命名规则

```c
// 初始化: Fbl_Init
void Fbl_Init(void);

// 主处理: Fbl_MainFunction
void Fbl_MainFunction(void);

// Flash操作: Fbl_Flash_{Operation}
Std_ReturnType Fbl_Flash_Erase(uint32 Address, uint32 Size);
Std_ReturnType Fbl_Flash_Write(uint32 Address, const uint8 *Data, uint32 Size);
Std_ReturnType Fbl_Flash_Read(uint32 Address, uint8 *Data, uint32 Size);

// 安全: Fbl_Security_{Operation}
Std_ReturnType Fbl_Security_GenerateSeed(uint8 *Seed, uint8 *Length);
Std_ReturnType Fbl_Security_VerifyKey(const uint8 *Key, uint8 Length);

// CRC校验: Fbl_Crc32_{Operation}
uint32 Fbl_Crc32_Calculate(const uint8 *Data, uint32 Length);
Std_ReturnType Fbl_Crc32_Verify(uint32 Address, uint32 Size, uint32 CrcValue);

// APP验证和启动
Std_ReturnType Fbl_App_Validate(void);
void Fbl_App_Jump(void);

// 计数器管理
Std_ReturnType Fbl_Counter_Read(uint16 *Counter);
Std_ReturnType Fbl_Counter_Increment(void);
Std_ReturnType Fbl_Counter_Reset(void);  // 带密码保护
```

---

### 2.7 配置文件

**源文件目录**: `src/config/`
**头文件目录**: `inc/config/`

```
src/config/
├── can_matrix.c                   # CAN矩阵(从DBC生成)
├── com_config.c                   # Com配置
├── dcm_config.c                   # Dcm配置
├── dem_config.c                   # Dem配置
├── cannm_config.c                 # CanNm配置
├── nvm_config.c                   # NVM配置
├── pin_config.c                   # GPIO引脚配置
└── system_config.c                # 系统全局配置

inc/config/
├── can_matrix.h
├── com_config.h
├── dcm_config.h
├── dem_config.h
├── cannm_config.h
├── nvm_config.h
├── pin_config.h
├── system_config.h
└── version.h                      # 版本信息
```

---

### 2.8 类型定义和公共头文件

```
src/
└── common.c                       # 通用函数实现(如CRC等)

inc/
├── types.h                        # 全局标准类型定义
│   ├── uint8, uint16, uint32
│   ├── float32, float64
│   ├── Std_ReturnType, Std_OkType等
│   └── 标准类型别名
├── common.h                       # 通用宏定义和函数
│   ├── 数学运算宏
│   ├── 位操作宏
│   ├── 通用数据结构
│   └── 调试宏(如LOG/ASSERT)
├── compiler.h                     # 编译器相关定义
├── os.h                           # OS调度相关定义
└── error_codes.h                  # 统一错误码定义
```

---

## 3. 关键文件命名规范总结

### 3.1 源文件命名规则

| 层级 | 前缀 | 命名格式 | 示例 |
|------|------|--------|------|
| MCAL | `mcal_` | `mcal_{module}_{function}.c` | `mcal_can_driver.c` |
| BSW | `bsw_` | `bsw_{module}_{feature}.c` | `bsw_dcm_service_0x22.c` |
| RTE | `rte_` | `rte_{feature}.c` | `rte_interface.c` |
| ASW | `app_` | `app_{feature}.c` | `app_motor_control.c` |
| APP主程序 | 无 | `{function}.c` | `main.c`, `task_scheduler.c` |
| FBL | `fbl_` | `fbl_{feature}.c` | `fbl_flash_driver.c` |
| Config | 无 | `{module}_config.c` | `can_matrix.c` |

### 3.2 头文件命名规则

**源文件头文件对应关系**:
- `src/mcal/can/can_driver.c` → `inc/mcal/can/can_driver.h`
- `src/bsw/dcm/dcm_service_0x22.c` → `inc/bsw/dcm/dcm_service.h`(统一头)
- `src/asw/app_motor_control.c` → `inc/asw/app_interface.h`(应用统一头)

### 3.3 函数命名规则

**基础格式**: `{Layer}_{Module}_{Operation}[_{Param}]`

```c
// 初始化函数
Std_ReturnType Mcal_Can_Init(void);
Std_ReturnType Bsw_Dcm_Init(void);
void App_System_Init(void);

// 主处理/步进函数
void Mcal_Systick_MainFunction(void);    // 周期中断处理
void Bsw_Com_MainFunction(void);         // 周期处理
void App_Task_10ms(void);                // 应用周期任务

// 操作函数
Std_ReturnType Mcal_Can_Send(uint32 CanId, const uint8 *Data, uint8 Dlc);
Std_ReturnType Bsw_Dem_SetEventStatus(uint16 EventId, Dem_EventStatusType Status);
Std_ReturnType Fbl_Flash_Write(uint32 Address, const uint8 *Data, uint32 Size);

// 回调函数
void Bsw_CanIf_RxIndication(uint32 CanId, const uint8 *Data, uint8 Dlc);
void Bsw_Com_TxConfirmation(uint16 SignalId);

// 中断处理
void App_CAN_RxISR(void);
void CAN1_RX0_IRQHandler(void);  // ARM标准名称
```

### 3.4 变量命名规则

```c
// 全局变量: g_{type}_{name} 或 {module}_{name}
uint32 g_CanRxBuffer[256];
Can_MessageType g_canMessage;
uint16 fbl_flashCounter;

// 静态变量: s_{type}_{name}
static uint8 s_currentSession = 0;
static uint16 s_dtcBuffer[10];

// 局部变量: 小驼峰命名法
uint8 dataLength;
uint32 canId;

// 结构体成员: 小驼峰或下划线分隔
typedef struct {
    uint32 canId;
    uint8  dlc;
    uint8  data[8];
} Can_FrameType;

// 宏定义: 大写+下划线
#define MCAL_CAN_MAX_BUFFER_SIZE    256
#define BSW_DEM_NUM_DTC             6
#define APP_TASK_PERIOD_10MS        10
```

---

## 4. 编译和构建配置

### 4.1 CMakeLists.txt 结构

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(STM32F407_ECU C ASM)

# 设置编译工具链
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_FLAGS "-mcpu=cortex-m4 -mthumb -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O2 -g0")

# 定义源文件目录
set(MCAL_SRC src/mcal/can src/mcal/adc src/mcal/timer ...)
set(BSW_SRC src/bsw/com src/bsw/canif src/bsw/dcm ...)
set(RTE_SRC src/rte)
set(ASW_SRC src/asw)
set(APP_SRC src/app)
set(FBL_SRC src/fbl)

# 包含路径
include_directories(
    inc
    inc/mcal
    inc/bsw
    inc/rte
    inc/asw
    inc/app
    inc/fbl
    inc/config
)

# 编译目标
add_executable(firmware_app.elf
    ${MCAL_SRC}
    ${BSW_SRC}
    ${RTE_SRC}
    ${ASW_SRC}
    ${APP_SRC}
)

add_executable(firmware_fbl.elf
    ${MCAL_SRC}
    ${BSW_SRC}
    ${FBL_SRC}
)
```

### 4.2 构建脚本

```bash
# tools/build.sh
#!/bin/bash

BUILD_TYPE=${1:-Debug}
BUILD_DIR="build"

mkdir -p $BUILD_DIR
cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
make -j4

# 生成hex和bin文件
arm-none-eabi-objcopy -O ihex firmware_app.elf firmware_app.hex
arm-none-eabi-objcopy -O binary firmware_app.elf firmware_app.bin
```

---

## 5. 单元测试结构

### 5.1 测试目录结构

```
test/
├── unit/                          # 单元测试
│   ├── test_mcal/
│   │   ├── test_can_driver.c
│   │   ├── test_adc_driver.c
│   │   └── test_timer_driver.c
│   ├── test_bsw/
│   │   ├── test_dcm_service.c
│   │   ├── test_dem_dtc.c
│   │   └── test_com_signal.c
│   ├── test_asw/
│   │   ├── test_motor_control.c
│   │   └── test_overspeed_monitor.c
│   └── test_app/
│       ├── test_task_scheduler.c
│       └── test_lowpower.c
├── integration/                   # 集成测试
│   ├── test_can_communication.c
│   ├── test_uds_diagnostic.c
│   └── test_end_to_end.c
├── mock/                          # Mock对象
│   ├── mock_can_driver.h
│   ├── mock_adc_driver.h
│   └── mock_nvm_driver.h
├── fixtures/                      # 测试数据
│   ├── can_test_frames.h
│   ├── dtc_test_data.h
│   └── uds_test_packets.h
├── CMakeLists.txt                # 测试编译配置
└── README.md                      # 测试说明
```

### 5.2 测试文件命名规范

```
test_{Module}_{Feature}.c          # 单元测试
例: test_dcm_service_0x22.c
    test_can_driver_send.c
    test_motor_control_init.c

mock_{Module}.h / mock_{Module}.c # Mock实现
例: mock_can_driver.h
    mock_nvm_driver.c
```

### 5.3 测试用例命名规范

```c
// CMake + cunit/gtest 框架
void test_dcm_service_0x22_read_did_f190(void);
void test_dem_button_stuck_trigger_and_recover(void);
void test_motor_pwm_duty_boundary_10_percent(void);
void test_can_receive_timeout_detection_2s(void);

// 命名格式: test_{module}_{feature}_{condition_or_expectation}
```

---

## 6. 版本控制和文档

### 6.1 .gitignore 示例

```
# 编译输出
/build/
/dist/
*.o
*.a
*.elf
*.hex
*.bin
*.map

# IDE
.vscode/
.settings/
*.project
*.pydevproject
*.cproject

# 临时文件
*~
*.swp
*.swo
*.tmp

# 构建缓存
CMakeCache.txt
CMakeFiles/

# 生成代码
src/asw/app_models/  # Simulink生成的代码
```

### 6.2 文档结构

```
docs/
├── SRS/
│   └── 汽车电子原型ECU项目需求规格.md
├── API/
│   ├── MCAL_API.md
│   ├── BSW_API.md
│   ├── RTE_API.md
│   └── ASW_API.md
├── DESIGN/
│   ├── Architecture.md            # 整体架构设计
│   ├── AUTOSAR_Mapping.md        # AUTOSAR映射
│   ├── CAN_Matrix.md             # CAN矩阵设计
│   ├── DTC_Design.md             # DTC故障设计
│   ├── DID_Design.md             # DID参数设计
│   ├── UDS_Service_Design.md     # UDS诊断设计
│   ├── FBL_Design.md             # Bootloader设计
│   └── Task_Schedule.md          # 任务调度设计
├── TESTING/
│   ├── Unit_Test_Plan.md
│   ├── Integration_Test_Plan.md
│   └── Test_Coverage.md
└── TOOLS/
    ├── Build_Guide.md
    ├── CAN_DBC_Guide.md
    └── Debugging_Guide.md
```

---

## 7. 完整项目初始化命令

```bash
# 创建项目根目录
mkdir STM32F407_ECU_Project && cd STM32F407_ECU_Project

# 创建核心目录结构
mkdir -p src/{mcal,bsw,rte,asw,app,fbl,config} \
         inc/{mcal,bsw,rte,asw,app,fbl,config} \
         cubeide_project \
         matlab_simulink \
         test/{unit,integration,mock,fixtures} \
         build \
         docs/{SRS,API,DESIGN,TESTING,TOOLS} \
         tools/scripts

# 创建细分目录
mkdir -p src/mcal/{can,adc,timer,gpio,uart,spi,pwm,nvm,clock,power,startup}
mkdir -p inc/mcal/{can,adc,timer,gpio,uart,spi,pwm,nvm,clock,power}
mkdir -p src/bsw/{com,canif,pdur,dcm,dem,cannm,nvm,nm}
mkdir -p inc/bsw/{com,canif,pdur,dcm,dem,cannm,nvm,nm}

# 初始化git
git init
echo "build/" > .gitignore
echo "cubeide_project/.settings/" >> .gitignore
echo ".vscode/settings.json" >> .gitignore

# 创建README
echo "# STM32F407 AUTOSAR ECU Project" > README.md
```

---

## 8. 文件头注释模板

### 8.1 C源文件头

```c
/**
 * @file can_driver.c
 * @author Your Name <your.email@example.com>
 * @date 2024-01-01
 * @version 1.0.0
 * 
 * @brief CAN驱动程序实现
 *        提供CAN硬件初始化、收发、中断处理功能
 *        遵循AUTOSAR MCAL规范
 * 
 * @section dependencies
 *   - mcal/can/can_driver.h
 *   - mcal/can/can_types.h
 *   - stm32f4xx_hal.h
 * 
 * @section history
 *   - 2024-01-01: 初始版本
 *   - 2024-01-15: 添加超时检测
 * 
 * @copyright Copyright (c) 2024 Company Name
 */

#include "can_driver.h"
```

### 8.2 C头文件头

```c
/**
 * @file can_driver.h
 * @author Your Name <your.email@example.com>
 * @date 2024-01-01
 * @version 1.0.0
 * 
 * @brief CAN驱动程序头文件
 *        定义CAN驱动的公共接口和类型
 * 
 * @defgroup MCAL_CAN CAN驱动模块
 * @{
 * 
 * @par 功能列表:
 *   - CAN初始化 (Mcal_Can_Init)
 *   - 消息发送 (Mcal_Can_Send)
 *   - 消息接收 (Mcal_Can_Receive)
 *   - 中断处理 (Mcal_Can_ISR)
 * 
 * @}
 */

#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============= Includes ============= */
#include "types.h"
#include "can_types.h"

/* ============= Public Functions ============= */

/**
 * @brief CAN驱动初始化
 * @param[in] Config CAN配置指针
 * @return Std_ReturnType
 *   @retval STD_OK    初始化成功
 *   @retval STD_NOT_OK 初始化失败
 */
Std_ReturnType Mcal_Can_Init(const Can_ConfigType *Config);

/* ============= ISR ============= */

/**
 * @brief CAN接收中断处理
 */
void Mcal_Can_RxISR(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_DRIVER_H */
```

---

## 9. VSCode 配置示例

### 9.1 .vscode/settings.json

```json
{
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/inc",
        "${workspaceFolder}/inc/mcal",
        "${workspaceFolder}/inc/bsw",
        "${workspaceFolder}/inc/rte",
        "${workspaceFolder}/inc/asw",
        "${workspaceFolder}/inc/app",
        "${workspaceFolder}/inc/fbl",
        "${workspaceFolder}/inc/config"
    ],
    "C_Cpp.default.defines": [
        "STM32F407xx",
        "USE_HAL_DRIVER",
        "ARM_MATH_CM4"
    ],
    "editor.formatOnSave": true,
    "[c]": {
        "editor.defaultFormatter": "ms-vscode.cpptools",
        "editor.formatOnSave": true
    }
}
```

### 9.2 .vscode/tasks.json

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build Debug",
            "type": "shell",
            "command": "bash",
            "args": ["./tools/build.sh", "Debug"],
            "problemMatcher": ["$gcc"],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
        {
            "label": "Build Release",
            "type": "shell",
            "command": "bash",
            "args": ["./tools/build.sh", "Release"]
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "bash",
            "args": ["./tools/run_tests.sh"]
        }
    ]
}
```

---

## 10. 命名规范总结表

| 项目 | 规范 | 示例 |
|------|------|------|
| **文件** | | |
| MCAL源文件 | `{module}_*.c` | `can_driver.c`, `adc_interrupt.c` |
| BSW源文件 | `{module}_*.c` | `dcm_service_0x22.c`, `dem_dtc_manager.c` |
| RTE源文件 | `rte_*.c` | `rte_interface.c`, `rte_scheduler.c` |
| 头文件 | 与源文件对应 | `can_driver.h`, `dcm_service.h` |
| **函数** | | |
| 初始化 | `{Layer}_{Module}_Init` | `Mcal_Can_Init`, `Bsw_Dcm_Init` |
| 主处理 | `{Layer}_{Module}_MainFunction` | `Bsw_Com_MainFunction`, `App_Task_10ms` |
| 中断处理 | `{Layer}_{Module}_ISR` | `Mcal_Can_RxISR`, `App_SysTick_ISR` |
| 回调 | `{Layer}_{Module}_{Event}Indication` | `Bsw_CanIf_RxIndication` |
| **变量** | | |
| 全局 | `g_{name}` 或 `{module}_{name}` | `g_canRxBuffer`, `dcm_sessionState` |
| 静态 | `s_{name}` | `s_currentSession` |
| 局部 | 小驼峰 | `canId`, `dataLength` |
| **宏定义** | | |
| 常量宏 | `{MODULE}_{ITEM}` | `MCAL_CAN_MAX_BUFFER`, `BSW_DEM_NUM_DTC` |
| 函数宏 | 小驼峰或大写 | `GET_BYTE(x)`, `SET_BIT(x,n)` |

---

## 11. 相关文档参考

- AUTOSAR标准: https://www.autosar.org/
- STM32F407参考手册
- ISO 14229-1 (UDS诊断标准)
- MISRA C编码规范
- GCC ARM嵌入式编程指南

---

**文档版本**: 1.0.0  
**最后更新**: 2024年  
**维护者**: Project Team
