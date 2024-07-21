#!/bin/bash
sudo modprobe input-polldev
sudo rmmod gpio_keys_polled.ko || true
make clean all
sudo insmod gpio_keys_polled.ko
