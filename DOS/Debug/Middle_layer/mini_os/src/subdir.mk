################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middle_layer/mini_os/src/minnOs_task.c \
../Middle_layer/mini_os/src/mutex.c \
../Middle_layer/mini_os/src/semaphore.c 

OBJS += \
./Middle_layer/mini_os/src/minnOs_task.o \
./Middle_layer/mini_os/src/mutex.o \
./Middle_layer/mini_os/src/semaphore.o 

C_DEPS += \
./Middle_layer/mini_os/src/minnOs_task.d \
./Middle_layer/mini_os/src/mutex.d \
./Middle_layer/mini_os/src/semaphore.d 


# Each subdirectory must supply rules for building sources it contributes
Middle_layer/mini_os/src/%.o Middle_layer/mini_os/src/%.su Middle_layer/mini_os/src/%.cyclo: ../Middle_layer/mini_os/src/%.c Middle_layer/mini_os/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/GitWork/DOS/DOS/App" -I"D:/GitWork/DOS/DOS/Middle_layer/mini_os/Irc" -I"D:/GitWork/DOS/DOS/Middle_layer/mini_os/List" -I"D:/GitWork/DOS/DOS/modules/uart" -I"D:/GitWork/DOS/DOS/modules/U_DriverUart" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middle_layer-2f-mini_os-2f-src

clean-Middle_layer-2f-mini_os-2f-src:
	-$(RM) ./Middle_layer/mini_os/src/minnOs_task.cyclo ./Middle_layer/mini_os/src/minnOs_task.d ./Middle_layer/mini_os/src/minnOs_task.o ./Middle_layer/mini_os/src/minnOs_task.su ./Middle_layer/mini_os/src/mutex.cyclo ./Middle_layer/mini_os/src/mutex.d ./Middle_layer/mini_os/src/mutex.o ./Middle_layer/mini_os/src/mutex.su ./Middle_layer/mini_os/src/semaphore.cyclo ./Middle_layer/mini_os/src/semaphore.d ./Middle_layer/mini_os/src/semaphore.o ./Middle_layer/mini_os/src/semaphore.su

.PHONY: clean-Middle_layer-2f-mini_os-2f-src

