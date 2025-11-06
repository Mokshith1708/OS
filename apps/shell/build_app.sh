#!/bin/bash

set -e

# Change to the script's directory to resolve relative paths correctly
cd $(dirname $0)

# The name of our application.
APP_NAME=shell_app

# This is the path to our cross-compiler toolchain.
TOOLCHAIN_PREFIX=arm-none-eabi

# Create a build directory.
mkdir -p build

# Compile the C sources for the app itself.
${TOOLCHAIN_PREFIX}-gcc -c -o build/$APP_NAME.o $APP_NAME.c -O2 -Wall -Wextra -nostdlib -mcpu=cortex-a9 -I../libuser/include
${TOOLCHAIN_PREFIX}-as -o build/vectors.o vectors.s -mcpu=cortex-a9

# Link the application using the GCC compiler driver.
# We now link against the libuser.a static library built by the main cmake build.
${TOOLCHAIN_PREFIX}-gcc -T $APP_NAME.ld -o build/$APP_NAME.elf build/$APP_NAME.o build/vectors.o ../../build/libuser.a -nostartfiles -nostdlib -lgcc -Wl,-Map=build/output.map

# Generate the final .bin and .proc files using our custom tool.
# First, convert the ELF to a raw binary.
${TOOLCHAIN_PREFIX}-objcopy -O binary build/$APP_NAME.elf build/$APP_NAME.bin

# Then, use pack_app to create the .proc file (now built by cmake).
../../build/pack_app build/$APP_NAME.bin build/$APP_NAME.proc

echo "Shell app built successfully!"
