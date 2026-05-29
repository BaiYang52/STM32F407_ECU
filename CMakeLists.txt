# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(STM32F407_ECU C ASM CXX)

# ========== Toolchain Configuration ==========

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "arm-none-eabi-as")

set(MCU "STM32F407VE")
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS "${CPU_FLAGS} -Wall -Wextra -Wpedantic -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O2 -g0")

set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -Wl,--gc-sections -Wl,-Map=output.map")

# ========== Include Paths ==========

set(COMMON_INCLUDES
    inc
    inc/config
    inc/mcal
    inc/bsw
    inc/rte
    inc/asw
    inc/app
    inc/fbl
)

include_directories(${COMMON_INCLUDES})

# ========== Source Files ==========

# MCAL源文件
set(MCAL_SOURCES
    src/mcal/can/can_driver.c
    src/mcal/can/can_interrupt.c
    src/mcal/adc/adc_driver.c
    src/mcal/timer/timer_driver.c
    src/mcal/gpio/gpio_driver.c
    src/mcal/uart/uart_driver.c
    src/mcal/spi/spi_driver.c
    src/mcal/pwm/pwm_driver.c
    src/mcal/nvm/eeprom_driver.c
)

# BSW源文件
set(BSW_SOURCES
    src/bsw/com/com_manager.c
    src/bsw/canif/canif_driver.c
    src/bsw/dcm/dcm_main.c
    src/bsw/dcm/dcm_service_0x22.c
    src/bsw/dcm/dcm_service_0x2E.c
    src/bsw/dcm/dcm_service_0x27.c
    src/bsw/dem/dem_main.c
    src/bsw/dem/dem_monitor.c
    src/bsw/cannm/cannm_main.c
    src/bsw/nvm/nvm_manager.c
)

# RTE源文件
set(RTE_SOURCES
    src/rte/rte_main.c
    src/rte/rte_interface.c
)

# ASW源文件 (Simulink生成的模型)
set(ASW_SOURCES
    src/asw/app_model_step_10ms.c
    src/asw/app_motor_control.c
)

# 应用主程序源文件
set(APP_SOURCES
    src/app/main.c
    src/app/app_init.c
    src/app/app_task_scheduler.c
    src/app/app_systick_handler.c
    src/app/app_lowpower.c
    src/app/app_watchdog.c
)

# FBL源文件
set(FBL_SOURCES
    src/fbl/fbl_main.c
    src/fbl/fbl_flash_driver.c
    src/fbl/fbl_dcm_service.c
)

# 配置文件
set(CONFIG_SOURCES
    src/config/can_matrix.c
    src/config/system_config.c
)

# 通用源文件
set(COMMON_SOURCES
    src/common.c
)

# ========== Build Targets ==========

# APP固件
add_executable(firmware_app.elf
    ${MCAL_SOURCES}
    ${BSW_SOURCES}
    ${RTE_SOURCES}
    ${ASW_SOURCES}
    ${APP_SOURCES}
    ${CONFIG_SOURCES}
    ${COMMON_SOURCES}
)

# FBL固件
add_executable(firmware_fbl.elf
    ${MCAL_SOURCES}
    ${BSW_SOURCES}
    ${FBL_SOURCES}
    ${CONFIG_SOURCES}
    ${COMMON_SOURCES}
)

# ========== Post-build Commands ==========

add_custom_command(TARGET firmware_app.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex firmware_app.elf firmware_app.hex
    COMMAND arm-none-eabi-objcopy -O binary firmware_app.elf firmware_app.bin
    COMMAND arm-none-eabi-size firmware_app.elf
)

# ========== Unit Tests (Optional) ==========

if(ENABLE_TESTING)
    enable_testing()
    
    add_executable(test_can_driver
        test/unit/test_can_driver.c
        test/mock/mock_can_driver.c
        src/mcal/can/can_driver.c
    )
    
    target_include_directories(test_can_driver PRIVATE
        ${COMMON_INCLUDES}
        test/mock
    )
    
    target_link_libraries(test_can_driver cunit)
    
    add_test(NAME CanDriver COMMAND test_can_driver)
endif()