################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/api/api_lib.c \
../lwip/src/api/api_msg.c \
../lwip/src/api/err.c \
../lwip/src/api/if_api.c \
../lwip/src/api/netbuf.c \
../lwip/src/api/netdb.c \
../lwip/src/api/netifapi.c \
../lwip/src/api/sockets.c \
../lwip/src/api/tcpip.c 

OBJS += \
./lwip/src/api/api_lib.o \
./lwip/src/api/api_msg.o \
./lwip/src/api/err.o \
./lwip/src/api/if_api.o \
./lwip/src/api/netbuf.o \
./lwip/src/api/netdb.o \
./lwip/src/api/netifapi.o \
./lwip/src/api/sockets.o \
./lwip/src/api/tcpip.o 

C_DEPS += \
./lwip/src/api/api_lib.d \
./lwip/src/api/api_msg.d \
./lwip/src/api/err.d \
./lwip/src/api/if_api.d \
./lwip/src/api/netbuf.d \
./lwip/src/api/netdb.d \
./lwip/src/api/netifapi.d \
./lwip/src/api/sockets.d \
./lwip/src/api/tcpip.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/api/%.o lwip/src/api/%.su lwip/src/api/%.cyclo: ../lwip/src/api/%.c lwip/src/api/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu18 -g3 -DDEBUG -DSTM32 -DSTM32H7SINGLE -DSTM32H743IITx -DSTM32H7 -c -I../Inc -I"/home/zero/Downloads/h7/stm32h743/lwip/src/include" -I"/home/zero/Downloads/h7/stm32h743/lwip/port" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-api

clean-lwip-2f-src-2f-api:
	-$(RM) ./lwip/src/api/api_lib.cyclo ./lwip/src/api/api_lib.d ./lwip/src/api/api_lib.o ./lwip/src/api/api_lib.su ./lwip/src/api/api_msg.cyclo ./lwip/src/api/api_msg.d ./lwip/src/api/api_msg.o ./lwip/src/api/api_msg.su ./lwip/src/api/err.cyclo ./lwip/src/api/err.d ./lwip/src/api/err.o ./lwip/src/api/err.su ./lwip/src/api/if_api.cyclo ./lwip/src/api/if_api.d ./lwip/src/api/if_api.o ./lwip/src/api/if_api.su ./lwip/src/api/netbuf.cyclo ./lwip/src/api/netbuf.d ./lwip/src/api/netbuf.o ./lwip/src/api/netbuf.su ./lwip/src/api/netdb.cyclo ./lwip/src/api/netdb.d ./lwip/src/api/netdb.o ./lwip/src/api/netdb.su ./lwip/src/api/netifapi.cyclo ./lwip/src/api/netifapi.d ./lwip/src/api/netifapi.o ./lwip/src/api/netifapi.su ./lwip/src/api/sockets.cyclo ./lwip/src/api/sockets.d ./lwip/src/api/sockets.o ./lwip/src/api/sockets.su ./lwip/src/api/tcpip.cyclo ./lwip/src/api/tcpip.d ./lwip/src/api/tcpip.o ./lwip/src/api/tcpip.su

.PHONY: clean-lwip-2f-src-2f-api

