#!/bin/bash

# This script builds and runs the MonolithOS kernel in QEMU.

# Exit immediately if a command exits with a non-zero status.
set -e

# 1. Build the project
# Create a build directory if it doesn't exist
mkdir -p build
# Go into the build directory
cd build
# Configure the project with CMake
cmake ..
# Compile and link the project
make
cd ..

# 2. Run the kernel in QEMU
#    -M mps2-an385: Use a generic ARM Cortex-M0/M0+ reference machine model.
#      This model has a memory map that matches our linker script.
qemu-system-arm \
    -M microbit \
    -cpu cortex-m0 \
    -kernel build/OS.elf \
    -nographic \
    -semihosting


echo "QEMU has exited."