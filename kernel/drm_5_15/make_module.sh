#!/bin/bash
make clean all
sudo modprobe drm_dma_helper || true
sudo rmmod drm_dummy.ko || true
sudo insmod ./drm_dummy.ko