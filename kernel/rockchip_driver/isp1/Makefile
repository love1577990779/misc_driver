# SPDX-License-Identifier: GPL-2.0
KERN_DIR = /home/zero/samba/rk/linux_sdk/atk-rk3568_linux_release_v1.2_20240129/kernel
obj-m += video_rkisp1.o

video_rkisp1-objs += rkisp1.o \
			dev.o \
			regs.o \
			isp_stats.o \
			isp_params.o \
			capture.o \
			dmarx.o

CROSS_COMPILE := aarch64-linux-gnu-
ARCH := arm64
ccflags-y += -mtune=cortex-a53


all:
	make -C $(KERN_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf ./*.o ./*.elf
