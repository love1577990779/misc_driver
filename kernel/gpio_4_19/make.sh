#!/bin/bash
sudo rmmod gpiochip_driver.ko || ture
make clean all
sudo modprobe gpio-amdpt || ture
sudo modprobe gpio-generic || ture
sudo modprobe gpio-pcie-idio-24 || ture
sudo modprobe gpio-exar || ture
sudo modprobe gpio-ml-ioh || ture
sudo modprobe intel_int0002_vgpio || ture
sudo modprobe ad7879 || ture
sudo modprobe ad7879 || ture



sudo insmod gpiochip_driver.ko
