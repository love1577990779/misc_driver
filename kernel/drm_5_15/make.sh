#!/bin/bash
set -e
make clean all
sudo rmmod drm_dummy.ko || true
sudo insmod drm_dummy.ko







