# SPDX-License-Identifier: GPL-2.0
CONFIG_VIDEO_ROCKCHIP_CIF :=m
KERN_DIR = /home/zero/rk/linux_sdk/atk-rk3568_linux_release_v1.2_20240129/kernel
CROSS_COMPILE := /home/zero/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
ARCH := arm64
ccflags-y += -mtune=cortex-a55 -Wno-error -Wno-misleading-indentation

obj-$(CONFIG_VIDEO_ROCKCHIP_CIF) += video_rkcif.o
video_rkcif-objs += dev.o \
			capture.o \
			mipi-csi2.o \
			cif-luma.o \
			hw.o \
			subdev-itf.o \
			procfs.o

all:
	make V=1 -C $(KERN_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
##$(CROSS_COMPILE)gcc -o test_input_demo.elf test_input_demo.c
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf ./*.o ./*.elf
