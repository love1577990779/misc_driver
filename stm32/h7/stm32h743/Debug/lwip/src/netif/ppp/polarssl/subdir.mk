################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/netif/ppp/polarssl/arc4.c \
../lwip/src/netif/ppp/polarssl/des.c \
../lwip/src/netif/ppp/polarssl/md4.c \
../lwip/src/netif/ppp/polarssl/md5.c \
../lwip/src/netif/ppp/polarssl/sha1.c 

OBJS += \
./lwip/src/netif/ppp/polarssl/arc4.o \
./lwip/src/netif/ppp/polarssl/des.o \
./lwip/src/netif/ppp/polarssl/md4.o \
./lwip/src/netif/ppp/polarssl/md5.o \
./lwip/src/netif/ppp/polarssl/sha1.o 

C_DEPS += \
./lwip/src/netif/ppp/polarssl/arc4.d \
./lwip/src/netif/ppp/polarssl/des.d \
./lwip/src/netif/ppp/polarssl/md4.d \
./lwip/src/netif/ppp/polarssl/md5.d \
./lwip/src/netif/ppp/polarssl/sha1.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/netif/ppp/polarssl/%.o lwip/src/netif/ppp/polarssl/%.su lwip/src/netif/ppp/polarssl/%.cyclo: ../lwip/src/netif/ppp/polarssl/%.c lwip/src/netif/ppp/polarssl/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu18 -g3 -DDEBUG -DSTM32 -DSTM32H7SINGLE -DSTM32H743IITx -DSTM32H7 -c -I../Inc -I"/home/zero/Downloads/h7/stm32h743/lwip/src/include" -I"/home/zero/Downloads/h7/stm32h743/lwip/port" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-netif-2f-ppp-2f-polarssl

clean-lwip-2f-src-2f-netif-2f-ppp-2f-polarssl:
	-$(RM) ./lwip/src/netif/ppp/polarssl/arc4.cyclo ./lwip/src/netif/ppp/polarssl/arc4.d ./lwip/src/netif/ppp/polarssl/arc4.o ./lwip/src/netif/ppp/polarssl/arc4.su ./lwip/src/netif/ppp/polarssl/des.cyclo ./lwip/src/netif/ppp/polarssl/des.d ./lwip/src/netif/ppp/polarssl/des.o ./lwip/src/netif/ppp/polarssl/des.su ./lwip/src/netif/ppp/polarssl/md4.cyclo ./lwip/src/netif/ppp/polarssl/md4.d ./lwip/src/netif/ppp/polarssl/md4.o ./lwip/src/netif/ppp/polarssl/md4.su ./lwip/src/netif/ppp/polarssl/md5.cyclo ./lwip/src/netif/ppp/polarssl/md5.d ./lwip/src/netif/ppp/polarssl/md5.o ./lwip/src/netif/ppp/polarssl/md5.su ./lwip/src/netif/ppp/polarssl/sha1.cyclo ./lwip/src/netif/ppp/polarssl/sha1.d ./lwip/src/netif/ppp/polarssl/sha1.o ./lwip/src/netif/ppp/polarssl/sha1.su

.PHONY: clean-lwip-2f-src-2f-netif-2f-ppp-2f-polarssl

