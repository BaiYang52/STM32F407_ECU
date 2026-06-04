################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/rte/rte.c \
../Core/Src/rte/rte_interface.c 

OBJS += \
./Core/Src/rte/rte.o \
./Core/Src/rte/rte_interface.o 

C_DEPS += \
./Core/Src/rte/rte.d \
./Core/Src/rte/rte_interface.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/rte/%.o Core/Src/rte/%.su Core/Src/rte/%.cyclo: ../Core/Src/rte/%.c Core/Src/rte/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I"E:/Project/Github/STM32F407_ECU/cubeide_project/Core/Inc" -I"E:/Project/Github/STM32F407_ECU/cubeide_project/Core/Inc/mcal" -I"E:/Project/Github/STM32F407_ECU/cubeide_project/Core/Inc/mcal/can" -I"E:/Project/Github/STM32F407_ECU/cubeide_project/Core/Inc/mcal/pwm" -I"E:/Project/Github/STM32F407_ECU/cubeide_project/Core/Inc/mcal/spi" -I"E:/Project/Github/STM32F407_ECU/cubeide_project/Core/Inc/mcal/gpio" -I"E:/Project/Github/STM32F407_ECU/cubeide_project/Core/Inc/mcal/timer" -IE:/Project/Github/STM32F407_ECU/cubeide_project/Core/Inc -IE:/Project/Github/STM32F407_ECU/cubeide_project/Core/Inc/bsw -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -finput-charset=UTF-8 -fexec-charset=UTF-8 -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-rte

clean-Core-2f-Src-2f-rte:
	-$(RM) ./Core/Src/rte/rte.cyclo ./Core/Src/rte/rte.d ./Core/Src/rte/rte.o ./Core/Src/rte/rte.su ./Core/Src/rte/rte_interface.cyclo ./Core/Src/rte/rte_interface.d ./Core/Src/rte/rte_interface.o ./Core/Src/rte/rte_interface.su

.PHONY: clean-Core-2f-Src-2f-rte

