################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../firmware/utils/core_bytes.cpp \
../firmware/utils/core_math.cpp \
../firmware/utils/dshot.cpp 

C_SRCS += \
../firmware/utils/core_utils.c 

C_DEPS += \
./firmware/utils/core_utils.d 

OBJS += \
./firmware/utils/core_bytes.o \
./firmware/utils/core_math.o \
./firmware/utils/core_utils.o \
./firmware/utils/dshot.o 

CPP_DEPS += \
./firmware/utils/core_bytes.d \
./firmware/utils/core_math.d \
./firmware/utils/dshot.d 


# Each subdirectory must supply rules for building sources it contributes
firmware/utils/%.o firmware/utils/%.su firmware/utils/%.cyclo: ../firmware/utils/%.cpp firmware/utils/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/Users/tizianohumpert/Documents/IMES/Ikarus/ikaruslab/cubeide-project/firmware" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
firmware/utils/%.o firmware/utils/%.su firmware/utils/%.cyclo: ../firmware/utils/%.c firmware/utils/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/Users/tizianohumpert/Documents/IMES/Ikarus/ikaruslab/cubeide-project/firmware" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-firmware-2f-utils

clean-firmware-2f-utils:
	-$(RM) ./firmware/utils/core_bytes.cyclo ./firmware/utils/core_bytes.d ./firmware/utils/core_bytes.o ./firmware/utils/core_bytes.su ./firmware/utils/core_math.cyclo ./firmware/utils/core_math.d ./firmware/utils/core_math.o ./firmware/utils/core_math.su ./firmware/utils/core_utils.cyclo ./firmware/utils/core_utils.d ./firmware/utils/core_utils.o ./firmware/utils/core_utils.su ./firmware/utils/dshot.cyclo ./firmware/utils/dshot.d ./firmware/utils/dshot.o ./firmware/utils/dshot.su

.PHONY: clean-firmware-2f-utils

