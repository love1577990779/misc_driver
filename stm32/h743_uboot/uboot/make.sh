#!/bin/bash
make ARCH=arm CROSS_COMPILE=/home/zero/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi- clean
make ARCH=arm CROSS_COMPILE=/home/zero/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi- V=1 stm32h743-eval_defconfig all 
dtc -O dts -I dtb -o ./uboot.dts ./u-boot.dtb


