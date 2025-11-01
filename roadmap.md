# OS Development Roadmap

This document outlines the step-by-step plan to build a simple, full-fledged operating system for a Cortex-M0 target.

## Phase 1: Core OS & Process Foundations (Complete)

- **Correct Architecture & Implement Privilege Separation**
- **Introduce Process Control Blocks (PCBs) & Basic Scheduler**
- **Implement System Timer (SysTick) for Preemption**
- **Implement Context Switching**
- **Refactor Kernel Boot Process**
- **Implement Blocking & Yielding Syscalls**

## Phase 2: System Features & User-space Enablement

- **Implement Command-Line Arguments (argc, argv):** **(DONE)** Added support for passing arguments from the shell to user applications via `exec` syscall. Shell and test applications updated.
- **Memory Management:** **(DONE)** Implemented a user-space heap allocator (`malloc`/`free`) via an `sbrk` syscall. Kernel `sys_sbrk` implemented, `heap_brk` added to PCB.
- **Proper Process Termination:** **(DONE)** Enhanced `_exit` to free a process's PCB, close all open files, and reload the shell.
- **File System:** **(DONE)** Implement a simple, in-memory file system (ramdisk) to manage application binaries and data.
    - Sub-task: Design filesystem structures (`fs.h`). **(DONE)**
    - Sub-task: Create `mkfs` host tool to build ramdisk image. **(DONE)**
    - Sub-task: Load ramdisk into kernel at boot. **(DONE)**
    - Sub-task: Implement `open`, `read`, `write`, `close`, `lseek`, `fstat` syscalls. **(DONE)**
    - Sub-task: Integrate `sys_exec` with new filesystem. **(DONE)**
- **Advanced Scheduler:** **(DONE)** Enhance the scheduler to handle multiple processes in various states (e.g., Ready, Running, Blocked, Sleeping).
    - Sub-task: Implement `sleep()` syscall. **(DONE)**
- **Expanded Syscall Interface:** **(DONE)** Add new system calls for features like inter-process communication (IPC) and timing.
    - Sub-task: Implement IPC mechanisms. **(DONE)**
    - Sub-task: Implement timing syscalls. **(DONE)** **(DONE)**
- **Device Drivers:** **(PARTIALLY DONE)** Develop drivers for specific hardware peripherals.
    - Sub-task: Implement basic UART console driver. **(DONE)**
    - Sub-task: Implement other device drivers. **(PENDING)**

## Known Issues / Next Steps

- **Build Error**: Kernel linker error: `undefined reference to 'sys_write'` (and other `sys_` functions) in `src/syscalls.c`. This is due to missing forward declarations for these kernel-internal syscall implementations within `src/syscalls.c` itself.