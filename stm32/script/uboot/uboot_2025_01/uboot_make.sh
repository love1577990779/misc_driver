#!/bin/sh
UBOOT_PATCH_PATH=/home/zero/stm32h743/uboot/uboot_patch
UBOOT_SRC_PATH=/home/zero/stm32h743/uboot/u-boot-2025.01
TOOLCHAIN_PATH=/home/zero/toolchain/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi/bin
CROSS_COMPILE=arm-none-eabi-


cp -rf ${UBOOT_PATCH_PATH}/* ${UBOOT_SRC_PATH}
cd ${UBOOT_SRC_PATH}
make distclean
make stm32h743-eval_defconfig ARCH=arm CROSS_COMPILE=${TOOLCHAIN_PATH}/${CROSS_COMPILE}
make -j8 ARCH=arm CROSS_COMPILE=${TOOLCHAIN_PATH}/${CROSS_COMPILE}

