################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../modules/uart/user_uart.c 

OBJS += \
./modules/uart/user_uart.o 

C_DEPS += \
./modules/uart/user_uart.d 


# Each subdirectory must supply rules for building sources it contributes
modules/uart/%.o modules/uart/%.su modules/uart/%.cyclo: ../modules/uart/%.c modules/uart/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/GitWork/DOS/DOS/App" -I"D:/GitWork/DOS/DOS/Middle_layer/mini_os/Irc" -I"D:/GitWork/DOS/DOS/Middle_layer/mini_os/List" -I"D:/GitWork/DOS/DOS/modules/uart" -I"D:/GitWork/DOS/DOS/modules/U_DriverUart" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-modules-2f-uart

clean-modules-2f-uart:
	-$(RM) ./modules/uart/user_uart.cyclo ./modules/uart/user_uart.d ./modules/uart/user_uart.o ./modules/uart/user_uart.su

.PHONY: clean-modules-2f-uart

