#!/bin/bash

set -e

# This is the path to our cross-compiler toolchain.
export PATH="/opt/mcb32-toolchain/bin:$PATH"

# The name of our application.
APP_NAME=shell_app

# This is the path to our cross-compiler toolchain.
TOOLCHAIN_PREFIX=arm-none-eabi

# Create a build directory.
mkdir -p build

# Compile the C and assembly sources.
${TOOLCHAIN_PREFIX}-gcc -c -o build/$APP_NAME.o $APP_NAME.c -O2 -Wall -Wextra -nostdlib -ffreestanding -mcpu=cortex-m0 -mthumb -I../libuser/include
${TOOLCHAIN_PREFIX}-gcc -c -o build/user_syscalls.o ../../src/libuser/user_syscalls.c -O2 -Wall -Wextra -nostdlib -ffreestanding -mcpu=cortex-m0 -mthumb -I../libuser/include
${TOOLCHAIN_PREFIX}-gcc -c -o build/stdio.o ../libuser/stdio.c -O2 -Wall -Wextra -nostdlib -ffreestanding -mcpu=cortex-m0 -mthumb -I../libuser/include
${TOOLCHAIN_PREFIX}-gcc -c -o build/string.o ../libuser/string.c -O2 -Wall -Wextra -nostdlib -ffreestanding -mcpu=cortex-m0 -mthumb -I../libuser/include
${TOOLCHAIN_PREFIX}-as -o build/user_startup.o ../../src/libuser/user_startup.S -mcpu=cortex-m0 -mthumb

# Link the application using the GCC compiler driver.
${TOOLCHAIN_PREFIX}-gcc -T $APP_NAME.ld -o build/$APP_NAME.elf build/$APP_NAME.o build/user_syscalls.o build/user_startup.o build/stdio.o build/string.o -nostartfiles -nostdlib -lgcc -Wl,-Map=build/output.map

# Generate the final .bin and .proc files using our custom tool.
# First, convert the ELF to a raw binary.
${TOOLCHAIN_PREFIX}-objcopy -O binary build/$APP_NAME.elf build/$APP_NAME.bin

# Then, use pack_app to create the .proc file.
../../tools/pack_app build/$APP_NAME.bin build/$APP_NAME.proc

echo "Shell app built successfully!"
