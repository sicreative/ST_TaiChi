################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include STWIN\ -\ Predictive_Maintenance/User/Patch/subdir.mk
-include STWIN\ -\ Predictive_Maintenance/User/subdir.mk
-include STWIN\ -\ Predictive_Maintenance/STM32CubeIDE/subdir.mk
-include Middlewares/STM32_USBD_Library/Core/subdir.mk
-include Middlewares/STM32_USBD_Library/Class/CDC/subdir.mk
-include Middlewares/STM32_MotionSP/subdir.mk
-include Middlewares/MetaDataManager/subdir.mk
-include Middlewares/BlueNRG-2/Utils/subdir.mk
-include Middlewares/BlueNRG-2/HCI/Controller/subdir.mk
-include Middlewares/BlueNRG-2/HCI/subdir.mk
-include Drivers/STM32L4xx_HAL_Driver/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Drivers/BSP/STWIN/subdir.mk
-include Drivers/BSP/Components/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
STM32L4R9ZI-STWIN_PredictiveMaintenance_Latest.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
STM32L4R9ZI-STWIN_PredictiveMaintenance_Latest.list \

OBJCOPY_BIN += \
STM32L4R9ZI-STWIN_PredictiveMaintenance_Latest.bin \


# All Target
all: STM32L4R9ZI-STWIN_PredictiveMaintenance_Latest.elf secondary-outputs

# Tool invocations
STM32L4R9ZI-STWIN_PredictiveMaintenance_Latest.elf: $(OBJS) $(USER_OBJS) ../STM32L4R9ZITx_FLASH.ld
	arm-none-eabi-gcc -o "STM32L4R9ZI-STWIN_PredictiveMaintenance_Latest.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"../STM32L4R9ZITx_FLASH.ld" --specs=nosys.specs -Wl,-Map="ST_TAICHI.map" -Wl,--gc-sections -static -L../../../../../../../Drivers/CMSIS/Lib/GCC --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

STM32L4R9ZI-STWIN_PredictiveMaintenance_Latest.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "STM32L4R9ZI-STWIN_PredictiveMaintenance_Latest.list"
	@echo 'Finished building: $@'
	@echo ' '

STM32L4R9ZI-STWIN_PredictiveMaintenance_Latest.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "STM32L4R9ZI-STWIN_PredictiveMaintenance_Latest.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
