# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(STM32F407_ECU C ASM)

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

# ============= 公共源文件 (FBL和APP都需要) =============

set(COMMON_SOURCES
    src/mcal/can/can_driver.c
    src/mcal/can/can_interrupt.c
    src/mcal/adc/adc_driver.c
    src/mcal/timer/timer_driver.c
    src/mcal/timer/timer_systick.c
    src/mcal/timer/timer_iwdg.c
    src/mcal/gpio/gpio_driver.c
    src/mcal/uart/uart_driver.c
    src/mcal/spi/spi_driver.c
    src/mcal/spi/spi_flash.c
    src/mcal/pwm/pwm_driver.c
    src/mcal/nvm/eeprom_driver.c
    src/mcal/nvm/flash_driver.c
    src/mcal/clock/clock_driver.c
    src/mcal/power/power_manager.c
    src/config/pin_config.c
    src/config/system_config.c
    src/config/can_matrix.c
    src/common.c
)

# ============= APP 特定源文件 =============

set(APP_SOURCES
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
    src/rte/rte_main.c
    src/rte/rte_interface.c
    src/asw/app_model_step_10ms.c
    src/asw/app_motor_control.c
    src/app/main.c
    src/app/app_init.c
    src/app/app_task_scheduler.c
    src/app/app_systick_handler.c
    src/app/app_lowpower.c
    src/app/app_watchdog.c
)

# ============= FBL 特定源文件 =============

set(FBL_SOURCES
    src/fbl/fbl_main.c
    src/fbl/fbl_init.c
    src/fbl/fbl_flash_driver.c
    src/fbl/fbl_download.c
    src/fbl/fbl_dcm_service.c
    src/fbl/fbl_security.c
    src/fbl/fbl_crc_check.c
    src/fbl/fbl_counter_manage.c
    src/fbl/fbl_app_validate.c
    src/fbl/fbl_app_jump.c
    src/fbl/fbl_recovery.c
    src/fbl/fbl_watchdog.c
)

# ============= 编译目标 1: APP固件 =============

add_executable(firmware_app.elf
    ${COMMON_SOURCES}
    ${APP_SOURCES}
)

target_include_directories(firmware_app.elf PRIVATE
    inc inc/mcal inc/bsw inc/rte inc/asw inc/app inc/config
)

target_compile_definitions(firmware_app.elf PRIVATE
    STM32F407xx
    USE_HAL_DRIVER
    ARM_MATH_CM4
    # APP 特定定义
    BUILD_TARGET_APP
)

# APP 链接脚本 (0x0801_0000 开始)
set_target_properties(firmware_app.elf PROPERTIES
    LINK_FLAGS "-Wl,--script=${CMAKE_SOURCE_DIR}/linker/STM32F407VE_APP.ld"
)

# ============= 编译目标 2: FBL固件 =============

add_executable(firmware_fbl.elf
    ${COMMON_SOURCES}
    ${FBL_SOURCES}
)

target_include_directories(firmware_fbl.elf PRIVATE
    inc inc/mcal inc/fbl inc/config
)

target_compile_definitions(firmware_fbl.elf PRIVATE
    STM32F407xx
    USE_HAL_DRIVER
    ARM_MATH_CM4
    # FBL 特定定义
    BUILD_TARGET_FBL
)

# FBL 链接脚本 (0x0800_0000 开始)
set_target_properties(firmware_fbl.elf PROPERTIES
    LINK_FLAGS "-Wl,--script=${CMAKE_SOURCE_DIR}/linker/STM32F407VE_FBL.ld"
)

# ============= Post-build Commands =============

add_custom_command(TARGET firmware_app.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex firmware_app.elf firmware_app.hex
    COMMAND arm-none-eabi-objcopy -O binary firmware_app.elf firmware_app.bin
    COMMAND arm-none-eabi-objcopy -O srec firmware_app.elf firmware_app.s19
    COMMAND arm-none-eabi-size firmware_app.elf
)

add_custom_command(TARGET firmware_fbl.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex firmware_fbl.elf firmware_fbl.hex
    COMMAND arm-none-eabi-objcopy -O binary firmware_fbl.elf firmware_fbl.bin
    COMMAND arm-none-eabi-objcopy -O srec firmware_fbl.elf firmware_fbl.s19
    COMMAND arm-none-eabi-size firmware_fbl.elf
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