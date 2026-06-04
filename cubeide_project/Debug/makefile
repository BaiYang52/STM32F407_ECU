################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/rte/subdir.mk
-include Core/Src/mcal/timer/subdir.mk
-include Core/Src/mcal/spi/subdir.mk
-include Core/Src/mcal/pwm/subdir.mk
-include Core/Src/mcal/gpio/subdir.mk
-include Core/Src/mcal/can/subdir.mk
-include Core/Src/bsw/pdur/subdir.mk
-include Core/Src/bsw/dcm/subdir.mk
-include Core/Src/bsw/com/subdir.mk
-include Core/Src/bsw/cantp/subdir.mk
-include Core/Src/bsw/canif/subdir.mk
-include Core/Src/asw/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := firmware_app
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
firmware_app.elf \

MAP_FILES += \
firmware_app.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
firmware_app.list \

OBJCOPY_HEX += \
firmware_app.hex \

OBJCOPY_BIN += \
firmware_app.bin \


# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: firmware_app.elf secondary-outputs

# Tool invocations
firmware_app.elf firmware_app.map: $(OBJS) $(USER_OBJS) E:\Project\Github\STM32F407_ECU\cubeide_project\STM32F407VETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "firmware_app.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"E:\Project\Github\STM32F407_ECU\cubeide_project\STM32F407VETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="firmware_app.map" -Wl,--gc-sections -static -u _printf_float --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

firmware_app.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "firmware_app.list"
	@echo 'Finished building: $@'
	@echo ' '

firmware_app.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "firmware_app.hex"
	@echo 'Finished building: $@'
	@echo ' '

firmware_app.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "firmware_app.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) default.size.stdout firmware_app.bin firmware_app.elf firmware_app.hex firmware_app.list firmware_app.map
	-@echo ' '

post-build:
	arm-none-eabi-objcopy -O srec --srec-len=32 --srec-forceS3 "firmware_app.elf" "firmware_app.s19"
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified post-build

-include ../makefile.targets
