################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/slee/Downloads/ST_TAICHI/Projects/STM32L4R9ZI-STWIN/Demonstrations/TaiChi/Src/system_stm32l4xx.c 

OBJS += \
./Drivers/CMSIS/system_stm32l4xx.o 

C_DEPS += \
./Drivers/CMSIS/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/system_stm32l4xx.o: /Users/slee/Downloads/ST_TAICHI/Projects/STM32L4R9ZI-STWIN/Demonstrations/TaiChi/Src/system_stm32l4xx.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L4R9xx -DUSE_HAL_DRIVER -DBLUENRG1_NWK_COPROC -DSPI_INTERFACE -DARM_MATH_CM4 -c -I../../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/CMSIS/DSP/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Drivers/BSP/Components/hts221 -I../../../../../../../Drivers/BSP/Components/lps22hh -I../../../../../../../Drivers/BSP/STWIN -I../../../../../../../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../../../../../../../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../../../../../../../Drivers/BSP/Components/ism330dhcx -I../../../../../../../Middlewares/ST/BlueNRG-2/includes -I../../../../../../../Middlewares/ST/BlueNRG-2/utils -I../../../../../../../Drivers/BSP/Components/iis2mdc -I../../../../../../../Middlewares/ST/STM32_MotionSP_Library/Inc -I../../../../../../../Middlewares/ST/STM32_MetaDataManager -I../../../Patch -I../../../../../../../Drivers/CMSIS/Include -Og -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"Drivers/CMSIS/system_stm32l4xx.d" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
