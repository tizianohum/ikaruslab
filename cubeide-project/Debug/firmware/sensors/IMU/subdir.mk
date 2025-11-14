################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../firmware/sensors/IMU/bmi160.cpp \
../firmware/sensors/IMU/core_imu_helpers.cpp 

OBJS += \
./firmware/sensors/IMU/bmi160.o \
./firmware/sensors/IMU/core_imu_helpers.o 

CPP_DEPS += \
./firmware/sensors/IMU/bmi160.d \
./firmware/sensors/IMU/core_imu_helpers.d 


# Each subdirectory must supply rules for building sources it contributes
firmware/sensors/IMU/%.o firmware/sensors/IMU/%.su firmware/sensors/IMU/%.cyclo: ../firmware/sensors/IMU/%.cpp firmware/sensors/IMU/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/Users/tizianohumpert/Documents/IMES/Ikarus/ikaruslab/cubeide-project/firmware" -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-firmware-2f-sensors-2f-IMU

clean-firmware-2f-sensors-2f-IMU:
	-$(RM) ./firmware/sensors/IMU/bmi160.cyclo ./firmware/sensors/IMU/bmi160.d ./firmware/sensors/IMU/bmi160.o ./firmware/sensors/IMU/bmi160.su ./firmware/sensors/IMU/core_imu_helpers.cyclo ./firmware/sensors/IMU/core_imu_helpers.d ./firmware/sensors/IMU/core_imu_helpers.o ./firmware/sensors/IMU/core_imu_helpers.su

.PHONY: clean-firmware-2f-sensors-2f-IMU

