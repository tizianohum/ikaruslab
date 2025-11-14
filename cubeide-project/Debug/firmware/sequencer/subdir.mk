################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../firmware/sequencer/ikarus_sequencer.cpp 

OBJS += \
./firmware/sequencer/ikarus_sequencer.o 

CPP_DEPS += \
./firmware/sequencer/ikarus_sequencer.d 


# Each subdirectory must supply rules for building sources it contributes
firmware/sequencer/%.o firmware/sequencer/%.su firmware/sequencer/%.cyclo: ../firmware/sequencer/%.cpp firmware/sequencer/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/Users/tizianohumpert/Documents/IMES/Ikarus/ikaruslab/cubeide-project/firmware" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-firmware-2f-sequencer

clean-firmware-2f-sequencer:
	-$(RM) ./firmware/sequencer/ikarus_sequencer.cyclo ./firmware/sequencer/ikarus_sequencer.d ./firmware/sequencer/ikarus_sequencer.o ./firmware/sequencer/ikarus_sequencer.su

.PHONY: clean-firmware-2f-sequencer

