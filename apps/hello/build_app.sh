#!/bin/bash

set -e

# Change to the script's directory
cd $(dirname $0)

APP_NAME=hello_app
TOOLCHAIN_PREFIX=arm-none-eabi

mkdir -p build

# Compile app sources
${TOOLCHAIN_PREFIX}-gcc -c -o build/$APP_NAME.o $APP_NAME.c -O2 -Wall -Wextra -nostdlib -ffreestanding -mcpu=cortex-a9 -I../libuser/include
${TOOLCHAIN_PREFIX}-as -o build/vectors.o vectors.s -mcpu=cortex-a9

# Link against libuser.a
${TOOLCHAIN_PREFIX}-gcc -T $APP_NAME.ld -o build/$APP_NAME.elf build/$APP_NAME.o build/vectors.o ../../build/libuser.a -nostartfiles -nostdlib -lgcc -Wl,-Map=build/output.map

# Convert to .bin
${TOOLCHAIN_PREFIX}-objcopy -O binary build/$APP_NAME.elf build/$APP_NAME.bin

# Create .proc file
../../build/pack_app build/$APP_NAME.bin build/$APP_NAME.proc

echo "$APP_NAME built successfully!"
