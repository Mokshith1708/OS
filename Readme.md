# OS

**Version:** 1.0 (Milestone M2 Complete)

**Status:** In Development

## Current Status

This document reflects the successful completion of the first two development modules. The kernel now has a stable boot process, a robust hardware abstraction layer for console I/O, and a functional physical memory manager. The system boots from a simulated FLASH memory and correctly reports its own memory layout.

**Demonstrable Milestone Achieved (M2):**
Project OS Kernel Initialized.

--- Module 2 Test ---

User space available: 32KB at 0x20008000

---

## Development Modules & Work Distribution

This project is divided into sequential modules. This section documents the work completed and attributes it to the responsible team members.

### Module 1: Foundational Setup, Boot, and Kernel Entry

**Goal:** Get the system to boot, set up a C environment, and print a "Kernel Initialized" message.
**Status:** **COMPLETE**

| Role | Team Member | Work Completed | Code/Files Contributed |
| :--- | :--- | :--- | :--- |
| **A: HAL & Low-Level** | T Mokshith Reddy | Wrote the initial Cortex-M processor startup file (`boot.s`), which includes the vector table, memory initialization logic (copying `.data` and zeroing `.bss`), and the jump to the C kernel entry point. | `src/boot.s` |
| **B: Kernel Logic** | Sasaank | Wrote the initial kernel entry point (`kmain.c`) and implemented the logic to call the console driver to print the boot message, fulfilling the module's primary goal. | `src/kmain.c` (initial version) |
| **C: API & Integration** | Sonith | Created the project's build system (`CMakeLists.txt`) and the crucial linker script (`linker.ld`). The linker script defines the FLASH/RAM memory map and separates kernel space from the future user space. Also defined the initial HAL API for the console. | `CMakeLists.txt`, `src/linker.ld`, `src/hal/hal_console.h` (initial version) |
| **D: Test & Verification** | Kalyan | Set up the QEMU environment and created the `run.sh` script to automate the build and execution process. This script proved invaluable for rapid testing and debugging of the boot process. | `run.sh` |

<br>

### Module 2: Kernel Memory Manager and Console Driver

**Goal:** Formalize the kernel's view of memory and implement a robust console driver.
**Status:** **COMPLETE**

| Role | Team Member | Work Completed | Code/Files Contributed |
| :--- | :--- | :--- | :--- |
| **A: HAL & Low-Level** | T Mokshith Reddy | Implemented the real hardware console driver in `hal_console.c`. This driver interfaces directly with the memory-mapped registers of the emulated PL011 UART, fulfilling the API defined in Module 1 and adding helper functions for printing integers and hexadecimal values. | `src/hal/hal_console.c` |
| **B: Kernel Logic** | Sasaank | Implemented the kernel's physical memory manager (`pmm.c`). This module provides functions (`get_user_space_base`, `get_user_space_size`) that intelligently read memory layout symbols (`_USER_SPACE_START`, etc.) provided by the linker script. | `src/pmm.c` |
| **C: API & Integration** | Sonith | Defined the formal API for the Physical Memory Manager in `pmm.h`. Integrated the new PMM and the final console driver into `kmain.c`, orchestrating the calls required to achieve the final milestone. Also updated the build system to include the new PMM source files. | `src/pmm.h`, updates to `kmain.c` and `CMakeLists.txt` |
| **D: Test & Verification** | Kalyan | Wrote the test logic within `kmain.c` that serves as the "unit test" for this module. The code now actively calls the PMM, retrieves the memory map, and uses every function in the new console driver to print the results, thus verifying that all components of Module 2 are working correctly. | Final test logic within `src/kmain.c` |

---

## How to Build and Run

1.  **Prerequisites:** Ensure you have `cmake`, `make`, `qemu-system-arm`, and the `arm-none-eabi-gcc` toolchain installed and in your system's PATH.
2.  **Clone the repository.**
3.  **Navigate to the project root directory.**
4.  **Make the run script executable:** `chmod +x run.sh`
5.  **Execute the script:** `./run.sh`

The script will automatically compile the kernel and launch it in the QEMU emulator. The milestone output will be printed to your terminal.