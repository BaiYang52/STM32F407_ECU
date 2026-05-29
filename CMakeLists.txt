cmake_minimum_required(VERSION 3.10)
project(STM32F407_ECU C ASM)

# Toolchain Configuration
set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_ASM_COMPILER "arm-none-eabi-as")

set(MCU "STM32F407VE")
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS "${CPU_FLAGS} -Wall -Wextra -Wpedantic -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O2 -g0")

# Include paths
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

# Source files
file(GLOB_RECURSE MCAL_SOURCES "src/mcal/*.c")
file(GLOB_RECURSE BSW_SOURCES "src/bsw/*.c")
file(GLOB_RECURSE RTE_SOURCES "src/rte/*.c")
file(GLOB_RECURSE ASW_SOURCES "src/asw/*.c")
file(GLOB_RECURSE APP_SOURCES "src/app/*.c")
file(GLOB_RECURSE CONFIG_SOURCES "src/config/*.c")

# Build target
add_executable(firmware_app.elf
    ${MCAL_SOURCES}
    ${BSW_SOURCES}
    ${RTE_SOURCES}
    ${ASW_SOURCES}
    ${APP_SOURCES}
    ${CONFIG_SOURCES}
)

# Post-build
add_custom_command(TARGET firmware_app.elf POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex firmware_app.elf firmware_app.hex
    COMMAND arm-none-eabi-objcopy -O binary firmware_app.elf firmware_app.bin
)
