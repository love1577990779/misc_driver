################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/port/port.c 

OBJS += \
./lwip/port/port.o 

C_DEPS += \
./lwip/port/port.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/port/%.o lwip/port/%.su lwip/port/%.cyclo: ../lwip/port/%.c lwip/port/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu18 -g3 -DDEBUG -DSTM32 -DSTM32H7SINGLE -DSTM32H743IITx -DSTM32H7 -c -I../Inc -I"/home/zero/Downloads/h7/stm32h743/lwip/src/include" -I"/home/zero/Downloads/h7/stm32h743/lwip/port" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-port

clean-lwip-2f-port:
	-$(RM) ./lwip/port/port.cyclo ./lwip/port/port.d ./lwip/port/port.o ./lwip/port/port.su

.PHONY: clean-lwip-2f-port

