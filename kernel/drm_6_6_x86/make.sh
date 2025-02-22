#!/bin/bash
set -e
make clean all
cp ./drm_dummy.ko -f /home/zero/busybox/busybox-1.36.1/_install/mod
cd /home/zero/busybox/busybox-1.36.1/_install
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz







