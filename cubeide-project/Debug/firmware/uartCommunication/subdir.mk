################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../firmware/uartCommunication/ikarus_communication.cpp \
../firmware/uartCommunication/uart_task.cpp 

OBJS += \
./firmware/uartCommunication/ikarus_communication.o \
./firmware/uartCommunication/uart_task.o 

CPP_DEPS += \
./firmware/uartCommunication/ikarus_communication.d \
./firmware/uartCommunication/uart_task.d 


# Each subdirectory must supply rules for building sources it contributes
firmware/uartCommunication/%.o firmware/uartCommunication/%.su firmware/uartCommunication/%.cyclo: ../firmware/uartCommunication/%.cpp firmware/uartCommunication/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/Users/tizianohumpert/Documents/IMES/Ikarus/ikaruslab/cubeide-project/firmware" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-firmware-2f-uartCommunication

clean-firmware-2f-uartCommunication:
	-$(RM) ./firmware/uartCommunication/ikarus_communication.cyclo ./firmware/uartCommunication/ikarus_communication.d ./firmware/uartCommunication/ikarus_communication.o ./firmware/uartCommunication/ikarus_communication.su ./firmware/uartCommunication/uart_task.cyclo ./firmware/uartCommunication/uart_task.d ./firmware/uartCommunication/uart_task.o ./firmware/uartCommunication/uart_task.su

.PHONY: clean-firmware-2f-uartCommunication

