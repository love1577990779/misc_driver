#!/bin/sh
make clean all
sudo insmod ./framebuffer_dummy.ko
sudo ./framebuffer_test.elf
sudo rmmod framebuffer_dummy

