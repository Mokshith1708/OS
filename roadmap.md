# OS Development Roadmap

This document outlines the step-by-step plan to build a simple, full-fledged operating system for a Cortex-M0 target.

## Phase 1: Core OS & Process Foundations

### Step 1: Correct Architecture & Implement Privilege Separation
- **Action:** Remove incorrect Cortex-M3 MPU code from `src/mmu.c`. Modify `src/proc.c` to launch processes in the unprivileged "Thread Mode" using the correct ARMv6-M (Cortex-M0) mechanism.
- **Goal:** Establish the fundamental security boundary of the OS. This will enable true system calls from unprivileged user code to the privileged kernel, making the `test_write` application function as intended.

### Step 2: Introduce Process Control Blocks (PCBs) & Basic Scheduler
- **Action:** Define a `pcb_t` struct in a new `proc.h` to store process state (registers, status, ID). Create a simple scheduler to manage a `current_process` pointer.
- **Goal:** Formalize the concept of a "process" within the OS, a prerequisite for multitasking.

### Step 3: Implement System Timer (SysTick) for Preemption
- **Action:** Configure the Cortex-M0's SysTick timer to fire at a regular interval. Create an interrupt handler for it.
- **Goal:** Allow the kernel to periodically regain control from a running process, enabling pre-emptive multitasking.

### Step 4: Implement Context Switching
- **Action:** Using the `PendSV` exception, write assembly code to save the context of the current process to its PCB and restore the context of the next process. The SysTick handler will trigger this switch.
- **Goal:** Enable the OS to switch between different processes, the core mechanism of multitasking.

## Phase 2: Expansion & Features

Once the core is stable, development will proceed to these areas:
- **Refactor Kernel Boot Process:** (Completed as part of recent work)
- **Implement Command-Line Arguments (argc, argv):** Add support for passing arguments from the kernel to user applications via the process's stack.
- **Implement Blocking & Yielding Syscalls:** (Currently in progress)
- **Advanced Scheduler:** Enhance the scheduler to handle multiple processes in various states (e.g., Ready, Running, Blocked, Sleeping).
- **Memory Management:** Implement a more robust kernel heap allocator (`kalloc`) and potentially a user-space allocator.
- **Expanded Syscall Interface:** Add new system calls for features like inter-process communication (IPC), timing, and dynamic memory allocation for user processes.
- **Device Drivers:** Develop drivers for specific hardware peripherals on the target FPGA.
- **File System:** Implement a simple, in-memory file system (like a ramdisk) to manage application binaries and data.
