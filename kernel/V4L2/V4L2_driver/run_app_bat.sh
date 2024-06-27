#!/bin/sh
set -e
make app
sudo ./v4l2_app.elf

