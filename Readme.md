# OS: A Custom Kernel and Operating System

**Version:** 2.0 (Milestone M7)
**Status:** In Development (Core Features Implemented)

## Overview

OS is a from-scratch operating system kernel built to run on an emulated ARM platform using QEMU. The project's goal is to implement a complete operating system stack, including a custom bootloader, kernel, memory and process managers, hardware drivers, a system call interface, and an interactive shell. A key objective is to create a platform capable of running a virtual machine.

This document outlines the project's development chronologically across seven major modules.

## Features

*   **Custom Bootloader:** Low-level startup code to initialize the ARM environment.
*   **Physical & Virtual Memory Management:** Manages the system's memory map, isolates kernel/user space, and includes foundational paging support.
*   **Process Management:** A high-level scheduler and process manager capable of loading and executing user-space applications.
*   **System Call Interface:** A robust mechanism for user-space applications to request kernel services.
*   **Hardware Drivers:** Support for console I/O (Framebuffer and Semihosting), keyboard, and block devices (SD Card).
*   **Interactive Shell:** A command-line interface for user interaction and program execution.
*   **Advanced Debugging:** A custom fault handler to capture and diagnose system crashes.
*   **Integrated Build System:** Uses CMake and custom scripts for automated cross-compilation and execution in QEMU.

## Getting Started

### Prerequisites

You must have the following tools installed and available in your system's PATH:

*   **CMake:** For building the project.
*   **Make:** The build utility used by CMake.
*   **QEMU:** Specifically `qemu-system-arm` for emulating the target hardware.
*   **ARM GCC Toolchain:** `arm-none-eabi-gcc` and its related tools for cross-compiling.

### Build and Run

1.  **Clone the repository:**
    ```sh
    git clone <your-repository-url>
    cd os-project
    ```
2.  **Make the run script executable:**
    ```sh
    chmod +x run.sh
    ```
3.  **Execute the script:**
    ```sh
    ./run.sh
    ```
The script will compile the kernel and all related components, then launch it within the QEMU emulator.

---

## Development Modules & Work Distribution

This project was developed in sequential modules. This section documents the goals and work completed by each team member.

### Module 1: Foundational Setup and Kernel Boot

**Goal:** Establish the QEMU simulation environment, boot the system, set up a C environment, and begin work on process management.
**Status:** **COMPLETE**

| Team Member | Work Completed | Code/Files Contributed |
| :--- | :--- | :--- |
| **Mokshith Reddy** | Wrote the initial Cortex-M startup file (`boot.s`), including the vector table and memory initialization. Created the linker script (`linker.ld`) to define the memory map and set up the build system (`CMakeLists.txt`) and QEMU execution script (`run.sh`). | `src/boot.s`, `src/linker.ld`, `CMakeLists.txt`, `run.sh` |
| **Kalyan** | Wrote the initial kernel entry point (`kmain.c`). Implemented the foundational logic for the Physical Memory Manager (`pmm.c`) and designed the initial structures for process management (`proc.c`). | `src/kmain.c`, `src/pmm.c`, `src/include/proc.h` |

<br>

### Module 2: Memory Management and Console I/O

**Goal:** Implement a robust console driver and formalize the kernel's view of memory to prepare for user-space applications.
**Status:** **COMPLETE**

| Team Member | Work Completed | Code/Files Contributed |
| :--- | :--- | :--- |
| **Mokshith Reddy** | Implemented the hardware console driver (`hal_console.c`) to interface with the emulated PL011 UART, providing low-level print functions. Integrated the PMM into the boot sequence. | `src/hal/hal_console.c`, updates to `kmain.c` |
| **Kalyan** | Finalized the Physical Memory Manager, enabling the kernel to report the memory layout. Wrote test logic in `kmain.c` to verify that the PMM and console driver were fully functional. | `src/pmm.c` (final version), test logic in `kmain.c` |

<br>

### Module 3: Initial Driver and Shell Implementation

**Goal:** Develop initial hardware drivers for user interaction (keyboard/display) and create a basic command shell.
**Status:** **COMPLETE**

| Team Member | Work Completed | Code/Files Contributed |
| :--- | :--- | :--- |
| **Mokshith Reddy** | Researched hardware specifications for PS/2 keyboards and screen controllers. Implemented the low-level stubs and HAL APIs for the keyboard and framebuffer drivers. | `src/include/hal/keyboard.h`, `src/include/hal/display.h` |
| **Kalyan** | Implemented the core logic for an interactive command shell (`shell.c`). Developed the initial process management code to support launching a foreground task like the shell. | `src/shell.c`, `src/proc.c` |

<br>

### Module 4: System Integration and Cross-Compilation

**Goal:** Cross-compile the OS and a Virtual Machine to run together on the QEMU simulator, establishing a basic system call interface.
**Status:** **COMPLETE**

| Team Member | Work Completed | Code/Files Contributed |
| :--- | :--- | :--- |
| **Mokshith Reddy** | Implemented the low-level system call interface, including the assembly stubs for trapping into the kernel. Updated the build system to cross-compile the VM and package it for the OS to load. | `src/syscall.c`, `src/arch/arm/syscall_asm.s`, `CMakeLists.txt` |
| **Kalyan** | Enhanced the process manager to load and execute an external binary (the VM). Implemented the kernel-side handlers for the newly created system calls. | `src/proc.c` (loader logic), `src/syscall_handler.c` |

<br>

### Module 5: Process Management and Storage I/O

**Goal:** Mature the process manager to run the OS and VM side-by-side and implement a loader to read from an SD card.
**Status:** **COMPLETE**

| Team Member | Work Completed | Code/Files Contributed |
| :--- | :--- | :--- |
| **Mokshith Reddy** | Wrote a low-level loader routine to read data from a simulated SD card block device. Implemented a basic paging system with large pages to simplify memory mapping for loaded programs. | `src/hal/sd_card.c`, `src/mm/paging.c` |
| **Kalyan** | Improved the shell with additional commands. Advanced the process manager to correctly isolate memory between the kernel and the running VM process, ensuring they could run concurrently. | `src/shell.c`, `src/proc.c` (memory isolation logic) |

<br>

### Module 6: System Refinement and Debugging

**Goal:** Stabilize all implemented features, complete the dual-mode console drivers, and implement an advanced fault handler for debugging.
**Status:** **COMPLETE**

| Team Member | Work Completed | Code/Files Contributed |
| :--- | :--- | :--- |
| **Mokshith Reddy**| Implemented low-level interrupt handlers and the foundational logic for process state persistence, allowing processes to be swapped. Refined the memory management and loader for stability. | `src/arch/arm/interrupt.c`, `src/mm/` |
| **Kalyan** | Implemented a dual-mode console that can switch between a fast framebuffer and a simpler semihosting output for debugging. Wrote an advanced fault handler that provides detailed crash diagnostics. | `src/hal/console_dual.c`, `src/fault_handler.c` |

<br>

### Module 7: Final Testing and Hardware Target Validation

**Goal:** Complete and test all drivers, validate the full software stack (OS + VM) on QEMU, and prepare the OS binary to run on a processor testbench.
**Status:** **COMPLETE**

| Team Member | Work Completed | Code/Files Contributed |
| :--- | :--- | :--- |
| **Mokshith Reddy**| Generated the final OS binary and created scripts to format it for the processor testbench. Led the effort to boot the OS on the target processor, identifying and resolving low-level hardware compatibility issues. | `scripts/generate_binary.sh`, updates to `linker.ld` |
| **Kalyan** | Completed the final implementation of the keyboard and monitor drivers. Conducted comprehensive end-to-end testing of the entire software stack in QEMU to ensure all components worked together correctly before hardware testing. | `src/hal/keyboard.c`, `src/hal/display.c`, integration test suites |