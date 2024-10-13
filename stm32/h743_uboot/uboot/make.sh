#!/bin/bash
make ARCH=arm CROSS_COMPILE=arm-none-eabi- clean
make ARCH=arm CROSS_COMPILE=arm-none-eabi- V=1 stm32h743-eval_defconfig all 



