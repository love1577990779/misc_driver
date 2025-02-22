################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/core/altcp.c \
../lwip/src/core/altcp_alloc.c \
../lwip/src/core/altcp_tcp.c \
../lwip/src/core/def.c \
../lwip/src/core/dns.c \
../lwip/src/core/inet_chksum.c \
../lwip/src/core/init.c \
../lwip/src/core/ip.c \
../lwip/src/core/mem.c \
../lwip/src/core/memp.c \
../lwip/src/core/netif.c \
../lwip/src/core/pbuf.c \
../lwip/src/core/raw.c \
../lwip/src/core/stats.c \
../lwip/src/core/sys.c \
../lwip/src/core/tcp.c \
../lwip/src/core/tcp_in.c \
../lwip/src/core/tcp_out.c \
../lwip/src/core/timeouts.c \
../lwip/src/core/udp.c 

OBJS += \
./lwip/src/core/altcp.o \
./lwip/src/core/altcp_alloc.o \
./lwip/src/core/altcp_tcp.o \
./lwip/src/core/def.o \
./lwip/src/core/dns.o \
./lwip/src/core/inet_chksum.o \
./lwip/src/core/init.o \
./lwip/src/core/ip.o \
./lwip/src/core/mem.o \
./lwip/src/core/memp.o \
./lwip/src/core/netif.o \
./lwip/src/core/pbuf.o \
./lwip/src/core/raw.o \
./lwip/src/core/stats.o \
./lwip/src/core/sys.o \
./lwip/src/core/tcp.o \
./lwip/src/core/tcp_in.o \
./lwip/src/core/tcp_out.o \
./lwip/src/core/timeouts.o \
./lwip/src/core/udp.o 

C_DEPS += \
./lwip/src/core/altcp.d \
./lwip/src/core/altcp_alloc.d \
./lwip/src/core/altcp_tcp.d \
./lwip/src/core/def.d \
./lwip/src/core/dns.d \
./lwip/src/core/inet_chksum.d \
./lwip/src/core/init.d \
./lwip/src/core/ip.d \
./lwip/src/core/mem.d \
./lwip/src/core/memp.d \
./lwip/src/core/netif.d \
./lwip/src/core/pbuf.d \
./lwip/src/core/raw.d \
./lwip/src/core/stats.d \
./lwip/src/core/sys.d \
./lwip/src/core/tcp.d \
./lwip/src/core/tcp_in.d \
./lwip/src/core/tcp_out.d \
./lwip/src/core/timeouts.d \
./lwip/src/core/udp.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/core/%.o lwip/src/core/%.su lwip/src/core/%.cyclo: ../lwip/src/core/%.c lwip/src/core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu18 -g3 -DDEBUG -DSTM32 -DSTM32H7SINGLE -DSTM32H743IITx -DSTM32H7 -c -I../Inc -I"D:/stm32/h7_lcd_20250211_23_11/h7_lcd/stm32h743/lwip/src/include" -I"D:/stm32/h7_lcd_20250211_23_11/h7_lcd/stm32h743/lwip/port" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-core

clean-lwip-2f-src-2f-core:
	-$(RM) ./lwip/src/core/altcp.cyclo ./lwip/src/core/altcp.d ./lwip/src/core/altcp.o ./lwip/src/core/altcp.su ./lwip/src/core/altcp_alloc.cyclo ./lwip/src/core/altcp_alloc.d ./lwip/src/core/altcp_alloc.o ./lwip/src/core/altcp_alloc.su ./lwip/src/core/altcp_tcp.cyclo ./lwip/src/core/altcp_tcp.d ./lwip/src/core/altcp_tcp.o ./lwip/src/core/altcp_tcp.su ./lwip/src/core/def.cyclo ./lwip/src/core/def.d ./lwip/src/core/def.o ./lwip/src/core/def.su ./lwip/src/core/dns.cyclo ./lwip/src/core/dns.d ./lwip/src/core/dns.o ./lwip/src/core/dns.su ./lwip/src/core/inet_chksum.cyclo ./lwip/src/core/inet_chksum.d ./lwip/src/core/inet_chksum.o ./lwip/src/core/inet_chksum.su ./lwip/src/core/init.cyclo ./lwip/src/core/init.d ./lwip/src/core/init.o ./lwip/src/core/init.su ./lwip/src/core/ip.cyclo ./lwip/src/core/ip.d ./lwip/src/core/ip.o ./lwip/src/core/ip.su ./lwip/src/core/mem.cyclo ./lwip/src/core/mem.d ./lwip/src/core/mem.o ./lwip/src/core/mem.su ./lwip/src/core/memp.cyclo ./lwip/src/core/memp.d ./lwip/src/core/memp.o ./lwip/src/core/memp.su ./lwip/src/core/netif.cyclo ./lwip/src/core/netif.d ./lwip/src/core/netif.o ./lwip/src/core/netif.su ./lwip/src/core/pbuf.cyclo ./lwip/src/core/pbuf.d ./lwip/src/core/pbuf.o ./lwip/src/core/pbuf.su ./lwip/src/core/raw.cyclo ./lwip/src/core/raw.d ./lwip/src/core/raw.o ./lwip/src/core/raw.su ./lwip/src/core/stats.cyclo ./lwip/src/core/stats.d ./lwip/src/core/stats.o ./lwip/src/core/stats.su ./lwip/src/core/sys.cyclo ./lwip/src/core/sys.d ./lwip/src/core/sys.o ./lwip/src/core/sys.su ./lwip/src/core/tcp.cyclo ./lwip/src/core/tcp.d ./lwip/src/core/tcp.o ./lwip/src/core/tcp.su ./lwip/src/core/tcp_in.cyclo ./lwip/src/core/tcp_in.d ./lwip/src/core/tcp_in.o ./lwip/src/core/tcp_in.su ./lwip/src/core/tcp_out.cyclo ./lwip/src/core/tcp_out.d ./lwip/src/core/tcp_out.o ./lwip/src/core/tcp_out.su ./lwip/src/core/timeouts.cyclo ./lwip/src/core/timeouts.d ./lwip/src/core/timeouts.o ./lwip/src/core/timeouts.su ./lwip/src/core/udp.cyclo ./lwip/src/core/udp.d ./lwip/src/core/udp.o ./lwip/src/core/udp.su

.PHONY: clean-lwip-2f-src-2f-core

