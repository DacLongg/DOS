################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../modules/U_DriverUart/U_DriverUart.c 

OBJS += \
./modules/U_DriverUart/U_DriverUart.o 

C_DEPS += \
./modules/U_DriverUart/U_DriverUart.d 


# Each subdirectory must supply rules for building sources it contributes
modules/U_DriverUart/%.o modules/U_DriverUart/%.su modules/U_DriverUart/%.cyclo: ../modules/U_DriverUart/%.c modules/U_DriverUart/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/GitWork/DOS/DOS/App" -I"D:/GitWork/DOS/DOS/Middle_layer/mini_os/Irc" -I"D:/GitWork/DOS/DOS/Middle_layer/mini_os/List" -I"D:/GitWork/DOS/DOS/modules/uart" -I"D:/GitWork/DOS/DOS/modules/U_DriverUart" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-modules-2f-U_DriverUart

clean-modules-2f-U_DriverUart:
	-$(RM) ./modules/U_DriverUart/U_DriverUart.cyclo ./modules/U_DriverUart/U_DriverUart.d ./modules/U_DriverUart/U_DriverUart.o ./modules/U_DriverUart/U_DriverUart.su

.PHONY: clean-modules-2f-U_DriverUart

