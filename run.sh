#!/bin/bash
set -e
rm -rf build
mkdir -p build
cd build
cmake ..
make
cd ..
qemu-system-arm \
    -M lm3s6965evb \
    -cpu cortex-m3 \
    -nographic \
    -kernel build/OS.elf
echo "QEMU has exited."