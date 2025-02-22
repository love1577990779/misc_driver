################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/lcd.c \
../Src/mac.c \
../Src/main.c \
../Src/rcc.c \
../Src/sdram.c \
../Src/sys.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/udp_app.c \
../Src/usart.c 

OBJS += \
./Src/lcd.o \
./Src/mac.o \
./Src/main.o \
./Src/rcc.o \
./Src/sdram.o \
./Src/sys.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/udp_app.o \
./Src/usart.o 

C_DEPS += \
./Src/lcd.d \
./Src/mac.d \
./Src/main.d \
./Src/rcc.d \
./Src/sdram.d \
./Src/sys.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/udp_app.d \
./Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu18 -g3 -DDEBUG -DSTM32 -DSTM32H7SINGLE -DSTM32H743IITx -DSTM32H7 -c -I../Inc -I"D:/stm32/h7_lcd_20250211_23_11/h7_lcd/stm32h743/lwip/src/include" -I"D:/stm32/h7_lcd_20250211_23_11/h7_lcd/stm32h743/lwip/port" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/lcd.cyclo ./Src/lcd.d ./Src/lcd.o ./Src/lcd.su ./Src/mac.cyclo ./Src/mac.d ./Src/mac.o ./Src/mac.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/rcc.cyclo ./Src/rcc.d ./Src/rcc.o ./Src/rcc.su ./Src/sdram.cyclo ./Src/sdram.d ./Src/sdram.o ./Src/sdram.su ./Src/sys.cyclo ./Src/sys.d ./Src/sys.o ./Src/sys.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/udp_app.cyclo ./Src/udp_app.d ./Src/udp_app.o ./Src/udp_app.su ./Src/usart.cyclo ./Src/usart.d ./Src/usart.o ./Src/usart.su

.PHONY: clean-Src

