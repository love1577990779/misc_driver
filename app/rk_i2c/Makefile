CROSS_COMPILE = $(shell pwd)/../../toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-

CFLAGS += -mtune=cortex-a55

SRC_FILE := rk_i2c.c
TARGET := rk_i2c.elf


all: 
	$(CROSS_COMPILE)gcc $(CFLAGS) $(SRC_FILE) -o $(TARGET)

clean:
	rm -rf ./*.o ./*.elf

	