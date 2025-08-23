
# Bare-Metal OS Simulation on QEMU

## 📌 Overview
This project demonstrates how to set up a **bare-metal operating system** for your custom processor using **QEMU**. It focuses on:
- Booting into a minimal kernel.
- Printing messages through UART.
- Simulating basic process management (round-robin loop).

---

## Prerequisites
- **QEMU** (with support for your target architecture)
- **Cross-Compiler Toolchain** for your architecture  
  (e.g., for ARM AArch64: `aarch64-linux-gnu-gcc`)
- **GNU Binutils** (`ld`, `objcopy`)
- **Make** or a build script

Install on Ubuntu:
```bash
sudo apt-get update
sudo apt-get install qemu-system build-essential gcc make
sudo apt-get install gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu
```

---

## ⚙️ Architecture-Specific Setup

Before building and running the OS, updated the following files with your processor-specific values:

- **`boot.S`** – Set the correct UART base address and any initialization required for your processor.
- **`kernel.cpp`** – Update the UART print functions (`uart_putc` / `uart_puts`) to use your hardware addresses.
- **`linker.ld`** – Adjust the memory layout according to your processor’s memory map (kernel load address, stack location, etc.).
- **QEMU command** – Replace `-machine` and `-cpu` options with the correct values for your CPU model.
---


## Project Structure
```
project/
├── boot.S           # Assembly entry (stack setup, jump to kmain)
├── kernel.cpp       # Kernel with process management
├── process.cpp      # Process logic
├── process.h        # Process class definitions
├── uart.cpp         # UART functions implementation
├── uart.h           # UART function declarations
├── linker.ld        # Linker script defining memory layout
├── myos.qcow2       # Virtual disk image
├── README.md        # Project documentation
├── kernel.elf       # ELF executable
└── Instructions.txt # logs of instructions used.

```

---

## Build Instructions
Compile boot and kernel:
```bash
aarch64-linux-gnu-g++ -ffreestanding -nostdlib -fno-exceptions -fno-rtti -fno-threadsafe-statics -O2 -c kernel.cpp -o kernel.o

aarch64-linux-gnu-g++ -ffreestanding -nostdlib -fno-exceptions -fno-rtti -fno-threadsafe-statics -O2 -c process.cpp -o process.o

aarch64-linux-gnu-g++ -ffreestanding -nostdlib -fno-exceptions -fno-rtti -fno-threadsafe-statics -O2 -c uart.cpp -o uart.o

aarch64-linux-gnu-gcc -ffreestanding -nostdlib -O2 -c boot.S -o boot.o

aarch64-linux-gnu-ld -T linker.ld -o kernel.elf boot.o kernel.o process.o uart.o

```

Convert ELF to raw binary (optional):
```bash
qemu-system-aarch64 -M virt -cpu cortex-a53 -kernel kernel.elf -nographic
```

---

## Running on QEMU
Create a **virtual disk** for simulation:
```bash
qemu-img create -f qcow2 myos.qcow2 2G
```

Run the OS:
```bash
qemu-system-aarch64 \
  -machine virt \
  -cpu cortex-a53 \
  -m 1024 \
  -nographic \
  -kernel kernel.elf \
  -drive file=myos.qcow2,if=none,format=qcow2,id=vd0 \
  -device virtio-blk-device,drive=vd0
```

### To exit QEMU:
- Press `Ctrl + A`, then `X`, **or**
- From another terminal:
```bash
pkill qemu-system-aarch64
```

---
# Code Overview

## `boot.S`
- Sets up stack and CPU.
- Disables MMU/caches.
- Jumps to `kmain()` in `kernel.cpp`.

## `kernel.cpp`
- Manages kernel initialization.
- Runs simple round-robin scheduler.
- Uses UART to print `"Running the OS"`.

## `process.cpp / process.h`
- Defines `Process` class and example processes.
- Placeholder for future scheduling logic.

## `uart.cpp / uart.h`
- Implements UART communication functions (`putc`, `puts`).

## `linker.ld`
- Maps `.text` and `.bss` sections at correct addresses:
---

## Next Steps
- Add **real scheduling** instead of infinite loop.
- Implement **memory management** and **page tables**.
- Add **system calls** and **drivers**.
- Boot a **minimal user program**.


