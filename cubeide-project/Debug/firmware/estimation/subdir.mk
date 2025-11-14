################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../firmware/estimation/basicvqf.cpp \
../firmware/estimation/estimation.cpp \
../firmware/estimation/vqf.cpp 

OBJS += \
./firmware/estimation/basicvqf.o \
./firmware/estimation/estimation.o \
./firmware/estimation/vqf.o 

CPP_DEPS += \
./firmware/estimation/basicvqf.d \
./firmware/estimation/estimation.d \
./firmware/estimation/vqf.d 


# Each subdirectory must supply rules for building sources it contributes
firmware/estimation/%.o firmware/estimation/%.su firmware/estimation/%.cyclo: ../firmware/estimation/%.cpp firmware/estimation/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/Users/tizianohumpert/Documents/IMES/Ikarus/ikaruslab/cubeide-project/firmware" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-firmware-2f-estimation

clean-firmware-2f-estimation:
	-$(RM) ./firmware/estimation/basicvqf.cyclo ./firmware/estimation/basicvqf.d ./firmware/estimation/basicvqf.o ./firmware/estimation/basicvqf.su ./firmware/estimation/estimation.cyclo ./firmware/estimation/estimation.d ./firmware/estimation/estimation.o ./firmware/estimation/estimation.su ./firmware/estimation/vqf.cyclo ./firmware/estimation/vqf.d ./firmware/estimation/vqf.o ./firmware/estimation/vqf.su

.PHONY: clean-firmware-2f-estimation

