################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"/Users/grippy/ti/gcc_arm_none_eabi_9_2_1/bin/arm-none-eabi-gcc-9.2.1" -c @"syscfg/device.opt"  -mcpu=cortex-m0plus -march=armv6-m -mthumb -mfloat-abi=soft -I"/Users/grippy/workspace_ccstheia/SD_Swicharoo_mspm0l1227" -I"/Users/grippy/workspace_ccstheia/SD_Swicharoo_mspm0l1227/Debug" -I"/Users/grippy/ti/mspm0_sdk_2_05_01_00/source/third_party/CMSIS/Core/Include" -I"/Users/grippy/ti/mspm0_sdk_2_05_01_00/source" -I"/Users/grippy/ti/gcc_arm_none_eabi_9_2_1/arm-none-eabi/include/newlib-nano" -I"/Users/grippy/ti/gcc_arm_none_eabi_9_2_1/arm-none-eabi/include" -O2 -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"/Users/grippy/workspace_ccstheia/SD_Swicharoo_mspm0l1227/Debug/syscfg" -std=c99 $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1595320671: ../SD_Swicharoo_mspm0l1227.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"/Applications/ti/ccs2020/ccs/utils/sysconfig_1.24.0/sysconfig_cli.sh" --script "/Users/grippy/workspace_ccstheia/SD_Swicharoo_mspm0l1227/SD_Swicharoo_mspm0l1227.syscfg" -o "syscfg" -s "/Users/grippy/ti/mspm0_sdk_2_05_01_00/.metadata/product.json" --compiler gcc
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/device_linker.lds: build-1595320671 ../SD_Swicharoo_mspm0l1227.syscfg
syscfg/device.opt: build-1595320671
syscfg/device.lds.genlibs: build-1595320671
syscfg/ti_msp_dl_config.c: build-1595320671
syscfg/ti_msp_dl_config.h: build-1595320671
syscfg/Event.dot: build-1595320671
syscfg: build-1595320671

syscfg/%.o: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"/Users/grippy/ti/gcc_arm_none_eabi_9_2_1/bin/arm-none-eabi-gcc-9.2.1" -c @"syscfg/device.opt"  -mcpu=cortex-m0plus -march=armv6-m -mthumb -mfloat-abi=soft -I"/Users/grippy/workspace_ccstheia/SD_Swicharoo_mspm0l1227" -I"/Users/grippy/workspace_ccstheia/SD_Swicharoo_mspm0l1227/Debug" -I"/Users/grippy/ti/mspm0_sdk_2_05_01_00/source/third_party/CMSIS/Core/Include" -I"/Users/grippy/ti/mspm0_sdk_2_05_01_00/source" -I"/Users/grippy/ti/gcc_arm_none_eabi_9_2_1/arm-none-eabi/include/newlib-nano" -I"/Users/grippy/ti/gcc_arm_none_eabi_9_2_1/arm-none-eabi/include" -O2 -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -MMD -MP -MF"syscfg/$(basename $(<F)).d_raw" -MT"$(@)" -I"/Users/grippy/workspace_ccstheia/SD_Swicharoo_mspm0l1227/Debug/syscfg" -std=c99 $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0l122x_gcc.o: /Users/grippy/ti/mspm0_sdk_2_05_01_00/source/ti/devices/msp/m0p/startup_system_files/gcc/startup_mspm0l122x_gcc.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"/Users/grippy/ti/gcc_arm_none_eabi_9_2_1/bin/arm-none-eabi-gcc-9.2.1" -c @"syscfg/device.opt"  -mcpu=cortex-m0plus -march=armv6-m -mthumb -mfloat-abi=soft -I"/Users/grippy/workspace_ccstheia/SD_Swicharoo_mspm0l1227" -I"/Users/grippy/workspace_ccstheia/SD_Swicharoo_mspm0l1227/Debug" -I"/Users/grippy/ti/mspm0_sdk_2_05_01_00/source/third_party/CMSIS/Core/Include" -I"/Users/grippy/ti/mspm0_sdk_2_05_01_00/source" -I"/Users/grippy/ti/gcc_arm_none_eabi_9_2_1/arm-none-eabi/include/newlib-nano" -I"/Users/grippy/ti/gcc_arm_none_eabi_9_2_1/arm-none-eabi/include" -O2 -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"/Users/grippy/workspace_ccstheia/SD_Swicharoo_mspm0l1227/Debug/syscfg" -std=c99 $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


