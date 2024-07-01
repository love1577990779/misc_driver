#!/bin/sh
set -e
make clean || true
make driver  
sudo modprobe vivid 
sudo rmmod vivid
sudo modprobe v4l2-async || true
sudo rmmod v4l2_drv.ko || true
sudo insmod v4l2_drv.ko
