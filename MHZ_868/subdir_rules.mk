################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
SensorNodeGSE.obj: ../SensorNodeGSE.c $(GEN_OPTS) $(GEN_SRCS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv5/tools/compiler/msp430_4.1.5/bin/cl430" -vmspx --abi=eabi -g --include_path="C:/ti/ccsv5/ccs_base/msp430/include" --include_path="C:/Users/ragarci2/workspace_v5_4/SensorNodeGSE/HAL" --include_path="C:/ti/ccsv5/tools/compiler/msp430_4.1.5/include" --define=MHZ_868 --define=__CC430F6147__ --diag_warning=225 --silicon_errata=CPU18 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="SensorNodeGSE.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


