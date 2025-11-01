# OS Features: Detailed Implementation

This document provides a detailed explanation of the features implemented in this operating system, including their architectural layout, design decisions, and execution flow.

---

## 1. System Call Interface

The System Call Interface (SCI) is the fundamental mechanism that allows unprivileged user-space applications to request services from the privileged kernel. It acts as a secure gateway, preventing direct access to hardware or critical kernel data structures, thereby maintaining system stability and security.

**Design Decisions:**

1.  **ARM `SVC` (Supervisor Call) Instruction:**
    *   **Why:** The ARM Cortex-M architecture provides the `SVC` instruction specifically for generating a Supervisor Call exception. This instruction is designed for transitioning from unprivileged Thread mode to privileged Handler mode.
    *   **Benefit:** When `SVC` is executed, the CPU automatically performs a context save (pushing `R0-R3`, `R12`, `LR`, `PC`, and `xPSR` onto the stack) and switches to Handler mode, ensuring that the kernel operates with full privileges while the user application remains restricted.
2.  **Syscall Numbers:**
    *   **Why:** To differentiate between various kernel services while using a single `SVC` instruction. Each service (e.g., `read`, `write`, `exit`) is assigned a unique integer identifier.
    *   **Benefit:** This simplifies the user-space interface (all syscalls look like `svc 0`) and allows the kernel to use a simple `switch` statement for dispatching to the correct handler.
3.  **Register-based Argument Passing:**
    *   **Why:** For efficiency. Passing arguments directly in CPU registers (`R0-R3`) is faster than pushing them onto the stack, especially for a small number of arguments typical of many syscalls. The return value is also passed back in `R0`.
    *   **Benefit:** Minimizes overhead during the frequent context switches associated with system calls.
4.  **Newlib Stubs:**
    *   **Why:** To provide compatibility with Newlib, a standard C library used in embedded systems. Newlib expects certain functions (like `_read`, `_write`, `sbrk`, `_exit`) to be available for its standard I/O and memory management routines.
    *   **Benefit:** Allows user applications to use standard C library functions (e.g., `printf`, `malloc`) without needing to know the underlying OS's specific syscall mechanism. The stubs translate these standard calls into the OS's custom `SVC` calls.

**Implementation Details and Execution Flow (Example: `write` syscall):**

Let's trace the journey of a `write` system call, from a user application to the kernel and back.

**1. User Application Initiates a Call (e.g., `printf`)**
    *   **File:** Any user application, for example, `/home/admin/Documents/OS/apps/hello/hello_app.c`.
    *   **Action:** A user application calls a standard C library function like `printf`. `printf` internally calls `write` to output data.

**2. User-Space Newlib Stub and Syscall Wrapper**
    *   **File:** `/home/admin/Documents/OS/apps/libuser/user_syscalls.c`
    *   **Action:**
        *   The `write` call from Newlib is routed to the `_write` stub in `user_syscalls.c`.
        *   `_write` then calls a generic `syscall` inline assembly function.
        *   Inside `syscall`, the syscall number (`SYS_WRITE`) is placed into CPU register `R0`, and arguments (`file` descriptor, buffer pointer, length) are placed into `R1`, `R2`, and `R3`.
        *   The `svc 0` instruction is executed, triggering a Supervisor Call exception.

**3. CPU Exception Handling and Vector Table**
    *   **File:** `/home/admin/Documents/OS/src/boot.s`
    *   **Action:**
        *   Upon executing `svc 0`, the ARM Cortex-M CPU automatically switches from unprivileged Thread mode to privileged Handler mode.
        *   It saves the current context (registers `R0-R3`, `R12`, `LR`, `PC`, and `xPSR`) onto the Process Stack Pointer (PSP) for user threads.
        *   It loads the address of the `SVC_Handler` from the exception vector table (defined in `boot.s`) into the Program Counter (PC).
        *   The Link Register (LR) is set to a special `EXC_RETURN` value, indicating how to return from the exception.

**4. Kernel-Space SVC Handler**
    *   **File:** `/home/admin/Documents/OS/src/syscalls.c` (specifically the `SVC_Handler` assembly and `svc_handler_c` C dispatcher).
    *   **Action:**
        *   The `SVC_Handler` assembly routine determines the user's stack pointer (PSP), where the hardware-saved registers (`R0-R3`, `R12`, `LR`, `PC`, `xPSR`) reside.
        *   It then branches to the `svc_handler_c` C function, passing the address of this saved stack frame as an argument.
        *   `svc_handler_c` extracts the syscall number (from `stack[0]`) and arguments (from `stack[1]` to `stack[3]`).

**5. Syscall Dispatcher**
    *   **File:** `/home/admin/Documents/OS/src/syscalls.c` (specifically `svc_handler_c`)
    *   **Action:**
        *   The `svc_handler_c` function, acting as the dispatcher, uses a `switch` statement to identify the requested service (e.g., `SYS_WRITE`) based on the extracted syscall number.
        *   It then calls the appropriate kernel-level service function, such as `sys_write`.
        *   The return value from the kernel service function is stored back into `stack[0]` of the saved stack frame.

**6. Kernel Service Execution (e.g., `sys_write`)**
    *   **Files:** `/home/admin/Documents/OS/src/syscalls.c` (for `sys_write` implementation) and `/home/admin/Documents/OS/src/hal/hal_console.c` (for `hal_console_write` implementation).
    *   **Action:**
        *   `sys_write` receives the arguments. It performs necessary checks (e.g., validating the file descriptor, checking permissions, translating user buffer addresses).
        *   For `STDOUT` (file descriptor 1), it calls `hal_console_write` (from `/home/admin/Documents/OS/src/hal/hal_console.c`).
        *   `hal_console_write` directly interacts with the UART peripheral (e.g., by writing characters to the PL011 UART's data register) to display the output).
        *   `sys_write` returns the number of bytes written (or an error code).

**7. Return to User-Space**
    *   **File:** `/home/admin/Documents/OS/src/syscalls.c` (return from `svc_handler_c`), `/home/admin/Documents/OS/src/interrupt.c` (return from `SVC_Handler`),
and `/home/admin/Documents/OS/apps/libuser/user_syscalls.c` (syscall wrapper).
    *   **Action:**
        *   After `svc_handler_c` returns, the `SVC_Handler` assembly function completes its execution.
        *   The `bx lr` instruction (with `LR` containing the `EXC_RETURN` value) causes the CPU to: switch back to unprivileged Thread mode, automatically restore the saved registers (`R0-R3`, `R12`, `LR`, `PC`, `xPSR`) from the stack (including the return value now in `R0`), and resume execution of the user application immediately following the `svc 0` instruction.
        *   The `syscall` function in `user_syscalls.c` retrieves the return value from `R0` and returns it to the calling user-space function (`_write`).
        *   `_write` then returns the result to Newlib, which finally returns it to the user application's `printf` function.

---

## 2. Memory Management: Physical Memory Manager (PMM)

The Physical Memory Manager (PMM) is responsible for managing the system's physical RAM. In this OS, the PMM provides a simple mechanism for allocating memory from a designated kernel heap and defines the boundaries for user-space memory.

**Architectural Layout:**

*   **Physical RAM:** The system has a defined RAM region, typically starting at `0x20000000` with a length of `16KB` (as defined in `/home/admin/Documents/OS/src/linker.ld`).
*   **Kernel Space vs. User Space:** The RAM is logically divided into two main areas:
    *   **Kernel RAM:** A portion of RAM (e.g., `4KB`) is reserved for the OS kernel's code, data, stack, and heap.
    *   **Application RAM:** The remaining RAM is designated for user applications, including their code, data, stack, and heap.
*   **Kernel Heap:** A contiguous block of memory within the kernel RAM, managed by the PMM for dynamic kernel allocations.
*   **User Heap:** A contiguous block of memory within the application RAM, intended for user-space dynamic allocations (though managed by user-space `sbrk` syscall, its boundaries are defined by the PMM).

**Design Decisions:**

1.  **Simple Bump Allocator for Kernel Heap:**
    *   **Why:** For simplicity and minimal overhead in a small embedded OS. A bump allocator is the easiest to implement and fastest for allocation, as it merely increments a pointer.
    *   **Benefit:** Fast allocation. Suitable for kernel components that allocate memory once and never free it, or for systems where memory fragmentation is not a primary concern.
    *   **Limitation:** No `free` functionality. Memory cannot be deallocated and reused, leading to potential memory exhaustion if many allocations occur without a system reset.
2.  **Linker-Defined Memory Regions:**
    *   **Why:** To precisely control the memory layout of the kernel and user applications at compile/link time.
    *   **Benefit:** Ensures that kernel and user memory regions do not overlap and that critical OS components are placed in known memory locations.
3.  **Separate Kernel and User Heaps:**
    *   **Why:** To enforce privilege separation and prevent user applications from corrupting kernel memory through heap operations.
    *   **Benefit:** Enhances system stability and security.

**Implementation Details and Execution Flow:**

**1. Memory Region Definition (`src/linker.ld`)**
    *   **File:** `/home/admin/Documents/OS/src/linker.ld`
    *   **Action:** This script defines the overall memory map:
        *   `RAM` is defined with its `ORIGIN` (start address) and `LENGTH`.
        *   `_os_ram_size` (e.g., `4KB`) is reserved for the OS.
        *   `_app_ram_start` is calculated as `ORIGIN(RAM) + _os_ram_size`.
        *   The kernel's `.text`, `.data`, and `.bss` sections are placed within the initial part of `RAM`.
        *   The `_end` symbol marks the end of the kernel's static memory, which implicitly becomes the start of the kernel heap.
        *   `__uheap_start` is set to `_app_ram_start`, and `__uheap_end` is set to `_ram_end` (the end of total RAM), defining the user-space heap region.
        *   Symbols like `__kheap_start` and `__kheap_end` are implicitly derived from `_end` and `_app_ram_start` for the kernel heap.

**2. PMM Initialization (`src/kmain.c` calls `pmm_init`)**
    *   **File:** `/home/admin/Documents/OS/src/pmm.c` and `/home/admin/Documents/OS/src/kmain.c`
    *   **Action:**
        *   During kernel startup, `kmain` (in `/home/admin/Documents/OS/src/kmain.c`) calls `pmm_init()`.
        *   `pmm_init()` (in `/home/admin/Documents/OS/src/pmm.c`) initializes the kernel heap:
            *   It sets `kernel_heap` to the address of `__kheap_start` (which is `_end` from the linker script).
            *   It calculates `kernel_heap_size` as the difference between `__kheap_end` and `__kheap_start`.
            *   `kernel_heap_used` is initialized to `0`.

**3. Kernel Memory Allocation (`pmm_alloc`)**
    *   **File:** `/home/admin/Documents/OS/src/pmm.c`
    *   **Action:**
        *   When a kernel component needs to allocate memory (e.g., for a Process Control Block), it calls `pmm_alloc(size)`.
        *   `pmm_alloc` checks if there is enough space remaining in the kernel heap (`kernel_heap_used + size <= kernel_heap_size`).
        *   If space is available, it returns a pointer to the current `kernel_heap + kernel_heap_used` address.
        *   It then increments `kernel_heap_used` by `size`.
        *   If no space is available, it returns `NULL` (or `0`).

**4. Kernel Memory Deallocation (`pmm_free`)**
    *   **File:** `/home/admin/Documents/OS/src/pmm.c`
    *   **Action:**
        *   `pmm_free(void *ptr)` is a no-op. It does not actually deallocate any memory. This means once memory is allocated from the kernel heap, it remains allocated for the lifetime of the kernel.

**5. User Space Memory Boundaries (`pmm_get_user_space_base`, `pmm_get_user_space_size`)**
    *   **File:** `/home/admin/Documents/OS/src/pmm.c`
    *   **Action:**
        *   These functions provide the base address (`__uheap_start`) and size (`__uheap_end - __uheap_start`) of the memory region designated for user-space applications. This information is crucial for the Memory Management Unit (MMU) and for user-space heap allocators (like `sbrk`).

---

## 3. Memory Management: Software-Enforced Memory Boundaries and User-Space Heap Management

This OS, targeting the Cortex-M0, does **not** implement a hardware-backed Memory Management Unit (MMU) or Memory Protection Unit (MPU). Instead, memory isolation and user-space memory management are achieved through software-enforced boundaries and a user-space heap managed via a system call.

**Architectural Layout:**

*   **Shared Physical Memory:** All user applications run within a single, contiguous physical memory region defined by the linker script (from `__app_ram_start__` to `__app_ram_end__`). This region is distinct from the kernel's memory space.
*   **User Application Layout:** Within this shared application RAM region, each user process has its own:
    *   **Code and Data Segment:** Loaded directly into the lower part of the application RAM.
    *   **Heap:** Grows upwards from immediately after the loaded code/data segment.
    *   **Stack:** Grows downwards from the top of the application RAM region.

**Design Decisions:**

1.  **Software-Enforced Boundaries:**
    *   **Why:** The Cortex-M0 lacks a hardware MPU, necessitating software checks to prevent user processes from accessing memory outside their designated areas or colliding with their own stack.
    *   **Benefit:** Provides a basic level of memory isolation and prevents common programming errors from corrupting other parts of the system (within the limits of software checks).
    *   **Limitation:** Less robust than hardware-backed protection. A malicious or buggy user application could potentially bypass these checks if not implemented perfectly, or if it directly accesses memory without using the `sbrk` syscall.
2.  **Linker-Defined Application RAM Region:**
    *   **Why:** To clearly delineate the memory available for user applications, separate from the kernel.
    *   **Benefit:** Simplifies memory management by providing fixed, known boundaries for user processes.
3.  **`sbrk` System Call for User Heap Management:**
    *   **Why:** To allow user applications to dynamically allocate memory (e.g., for `malloc`/`free` implementations) in a controlled manner, with kernel oversight.
    *   **Benefit:** Enables standard C library memory allocation functions in user space while allowing the kernel to enforce memory boundaries and prevent heap/stack collisions.

**Implementation Details and Execution Flow:**

**1. Application Memory Region Definition (`src/linker.ld`)**
    *   **File:** `/home/admin/Documents/OS/src/linker.ld`
    *   **Action:** The linker script defines the symbols `__app_ram_start__` and `__app_ram_end__`, which mark the beginning and end of the physical RAM region allocated for user applications. This region is distinct from the kernel's memory.

**2. Process Loading (`sys_exec` in `src/proc.c`)**
    *   **File:** `/home/admin/Documents/OS/src/proc.c`
    *   **Action:**
        *   When a new user process is created via `sys_exec`, its binary image (code and initial data) is loaded directly into the physical memory starting at `APP_BASE` (which corresponds to `__app_ram_start__`).
        *   The `pcb->heap_brk` for the new process is initialized to the address immediately following the loaded application image, ensuring 8-byte alignment. This marks the initial boundary of the user-space heap.
        *   The initial user stack pointer (`pcb->sp`) is set to `APP_END` (which corresponds to `__app_ram_end__`), and the stack grows downwards from there.

**3. User-Space Heap Allocation (`sbrk` syscall)**
    *   **Files:** `/home/admin/Documents/OS/apps/libuser/user_syscalls.c` (user-side wrapper) and `/home/admin/Documents/OS/src/proc.c` (kernel-side handler `_sbrk`).
    *   **Action:**
        *   A user application (or its `malloc` implementation) calls `sbrk(increment)` to request more heap memory.
        *   This triggers a `SYS_SBRK` system call (as described in the System Call Interface section).
        *   The kernel-side `_sbrk` function in `src/proc.c` is invoked:
            *   It retrieves the `current_process->heap_brk` (the current end of the heap).
            *   It calculates `new_brk = old_brk + increment`.
            *   **Collision Check 1 (Heap vs. Stack):** It checks if `new_brk` would overlap with the current process's stack pointer (`current_process->sp`). If `new_brk >= current_process->sp`, a heap/stack collision is detected, and `_sbrk` returns an error.
            *   **Collision Check 2 (Boundary Check):** It checks if `new_brk` would exceed the total application memory region (`APP_END`). If `new_brk >= APP_END`, it indicates out-of-memory, and `_sbrk` returns an error.
            *   If both checks pass, `current_process->heap_brk` is updated to `new_brk`, and the `old_brk` address is returned to the user application, effectively extending its heap.

**4. Memory Isolation:**
    *   **Mechanism:** The primary isolation mechanism relies on the fact that user applications are loaded into a specific physical memory range (`APP_BASE` to `APP_END`). The kernel's memory is located outside this range.
    *   **Protection:** The `_sbrk` syscall handler prevents user processes from growing their heap beyond their allocated region or into their stack. The `SVC` mechanism prevents user processes from directly accessing kernel memory or executing privileged instructions. However, without a hardware MPU, a user process could theoretically write to any address within the `APP_BASE` to `APP_END` range, potentially corrupting other user processes if multiple are loaded concurrently in this shared space without further software segmentation.

---

## 4. Process Management: PCBs and Process Creation

Process Management is a core function of the OS, enabling the execution of multiple applications concurrently and managing their lifecycle. In this OS, process management revolves around Process Control Blocks (PCBs) and a mechanism to create and load user applications.

**Architectural Layout:**

*   **Process Control Block (PCB) Table:** The OS maintains a fixed-size array of `pcb_t` structures (named `pcb_table` in `/home/admin/Documents/OS/src/proc.c`), which serves as a repository for all process-related information. Each entry in this table represents a potential process slot.
*   **Current Process Pointer:** A global pointer, `current_process` (in `/home/admin/Documents/OS/src/proc.c`), tracks the PCB of the process currently executing on the CPU.
*   **Application RAM Region:** User processes are loaded into a dedicated physical memory region (defined by `__app_ram_start__` and `__app_ram_end__` in `/home/admin/Documents/OS/src/linker.ld`) distinct from the kernel's memory. This is the `APP_BASE` to `APP_END` region.
*   **Ramdisk:** User application binaries are stored within an in-memory ramdisk, which the kernel consults during process creation.

**Design Decisions:**

1.  **Static PCB Table:**
    *   **Why:** A fixed-size array (`pcb_table`) for PCBs simplifies memory management in a small embedded OS as it avoids dynamic allocation of PCBs.
    *   **Benefit:** Predictable memory usage and simpler implementation.
    *   **Limitation:** A hard limit on the maximum number of processes (`MAX_PROCESSES`).
2.  **Explicit Process States:**
    *   **Why:** To clearly define the lifecycle stages of a process (e.g., `UNUSED`, `READY`, `RUNNING`, `BLOCKED`, `SLEEPING`).
    *   **Benefit:** Essential for a functional scheduler and managing process synchronization.
3.  **Unified `sys_exec` for Creation and Overlay:**
    *   **Why:** Using a single kernel function (`sys_exec`) to either start a new process or replace the current process's image (similar to the Unix `exec` family of calls) streamlines the process loading logic.
    *   **Benefit:** Reduces code duplication and provides a flexible mechanism for process execution.
4.  **In-Memory Ramdisk for Binaries:**
    *   **Why:** Simplifies file access by eliminating the need for complex block device drivers and on-disk filesystem structures during boot.
    *   **Benefit:** Faster boot times and simpler filesystem implementation for application binaries.
5.  **"Fake Exception Frame" for Initial Context:**
    *   **Why:** To allow a newly created process to start execution in user mode as if it just returned from an exception (e.g., an SVC call).
    *   **Benefit:** Conveniently uses the same context switching mechanism (`proc_switch_to_user`) for both new processes and processes resuming from a real exception.

**Implementation Details and Execution Flow (Process Creation: `start_process` and `sys_exec`):**

The process of creating and loading a user application into memory involves several steps orchestrated by the kernel.

**1. Kernel Initialization (`proc_init` in `src/kmain.c`)**
    *   **File:** `/home/admin/Documents/OS/src/proc.c` and `/home/admin/Documents/OS/src/kmain.c`
    *   **Action:**
        *   During kernel startup, `kmain` calls `proc_init()`.
        *   `proc_init()` iterates through the `pcb_table` and initializes each PCB slot to `PROC_STATE_UNUSED`.
        *   It also sets up default values for file descriptor tables, current working directories (`cwd_inode`), and message queues for each potential process.

**2. Initiating the First User Process (`start_process` in `src/kmain.c`)**
    *   **File:** `/home/admin/Documents/OS/src/kmain.c` and `/home/admin/Documents/OS/src/proc.c`
    *   **Action:**
        *   After other kernel subsystems are initialized, `kmain` calls `start_process("shell_app.proc")` to launch the initial shell application.
        *   `start_process` is a simple wrapper that prepares `argc` and `argv` for the initial shell and then calls `sys_exec`.

**3. Core Process Loading and Setup (`sys_exec` in `src/proc.c`)**
    *   **File:** `/home/admin/Documents/OS/src/proc.c`, `/home/admin/Documents/OS/src/fs.c`, `/home/admin/Documents/OS/src/swap.c`
    *   **Action:**
        *   **PCB Acquisition:**
            *   If `sys_exec` is called without an existing `current_process` (i.e., kernel starting a new process), it calls `proc_create()` to find an unused PCB slot from `pcb_table`. This `pcb_t` is the target for the new process. If no free PCB is found, it returns an error.
            *   If `sys_exec` is called from an existing process (e.g., a shell `exec` command), the current process's PCB (`current_process`) is reused.
        *   **File Lookup:** `sys_exec` uses `fs_find_file(path)` (implemented in `/home/admin/Documents/OS/src/fs.c`) to locate the application binary (`shell_app.proc` in our example) within the in-memory ramdisk. This function returns an `inode_t` if found.
        *   **Image Loading (`swap_in`):**
            *   `sys_exec` then calls `swap_in(inode, &entry, &initial_sp_from_file, &img_size)` (implemented in `/home/admin/Documents/OS/src/swap.c`).
            *   `swap_in` reads the process image header (`proc_img_hdr_t`) from the ramdisk. This header contains the application's `ram_size`, `entry_pc` (entry point address), and `initial_sp` (initial stack pointer hint).
            *   It then copies the application's code and data from the ramdisk into the designated physical memory region for applications, starting at `APP_BASE` (defined by `__app_ram_start__` from `/home/admin/Documents/OS/src/linker.ld`).
            *   The `entry`, `initial_sp_from_file`, and `img_size` values are returned to `sys_exec`.
        *   **Heap Break Initialization:** The process's heap break (`pcb->heap_brk`) is set to the address immediately following the loaded application image, ensuring 8-byte alignment.
        *   **Argument and Stack Setup:** This is a crucial step to prepare the user process's initial execution context:
            *   The user stack pointer (`sp`) is initially set to `APP_END` (the top of the application RAM region).
            *   Argument strings (`argv`) are copied onto this stack region, growing downwards.
            *   An `argv` pointer array is created on the stack, containing pointers to the copied argument strings, and is null-terminated.
            *   **"Fake Exception Frame" Creation:** A stack frame is meticulously crafted onto the user stack (growing downwards from `sp`):
                *   `xPSR` (Program Status Register) is pushed, with the Thumb bit set to indicate Thumb mode execution.
                *   `PC` (Program Counter) is pushed, set to the application's `entry_pc` (retrieved from the application header).
                *   `LR` (Link Register) is pushed (typically 0, as `main` doesn't return to a specific address in this context).
                *   `R12`, `R3`, `R2` are pushed (initialized to 0).
                *   `R1` is pushed, set to the address of the `argv` array on the stack.
                *   `R0` is pushed, set to `argc`.
            *   The process's `sp` in its PCB (`pcb->sp`) is updated to point to the top of this newly created stack frame.
        *   **Final State:** The PCB's state is set to `PROC_STATE_READY`, and the kernel prints a message indicating successful process creation.

**4. Initial Scheduler Start (`scheduler_start` in `src/kmain.c`)**
    *   **File:** `/home/admin/Documents/OS/src/kmain.c` and `/home/admin/Documents/OS/src/proc.c`
    *   **Action:**
        *   `kmain` calls `scheduler_start()`, which then finds the first `PROC_STATE_READY` process (which will be the `shell_app.proc`).
        *   This process is set as `current_process`, its state changes to `PROC_STATE_RUNNING`.
        *   `proc_switch_to_user(current_process->sp)` is called. This low-level assembly function (defined elsewhere, but conceptually it restores the CPU registers from the `current_process->sp` and uses the `EXC_RETURN` mechanism) transitions the CPU from kernel mode to user mode, starting the execution of the `shell_app.proc` at its entry point.

---

## 5. Process Management: Scheduler and Context Switching

The OS implements a preemptive, round-robin scheduler that manages the execution of multiple processes. Context switching is the mechanism by which the CPU's state is saved for the currently running process and restored for the next process to be executed.

**Architectural Layout:**

*   **SysTick Timer:** A hardware timer (SysTick) is configured to generate periodic interrupts, serving as the heartbeat for the scheduler.
*   **Interrupt Vector Table:** The `SysTick_Handler` and `PendSV_Handler` are registered in the CPU's interrupt vector table, ensuring they are called when their respective exceptions occur.
*   **Process Control Blocks (PCBs):** Each `pcb_t` stores the saved context (specifically the stack pointer `sp`) of its associated process, allowing the kernel to save and restore its execution state.
*   **Interrupt Control and State Register (ICSR):** A special CPU register used to pend the PendSV exception.

**Design Decisions:**

1.  **Preemptive Scheduling via SysTick and PendSV:**
    *   **Why:** To ensure fairness among processes and prevent a single process from monopolizing the CPU. SysTick provides the periodic timer, and PendSV allows the actual context switch to occur at a low priority, minimizing disruption to higher-priority interrupts.
    *   **Benefit:** Guarantees that all ready processes get a slice of CPU time, improving responsiveness and multitasking.
2.  **Round-Robin Scheduling Policy:**
    *   **Why:** A simple and fair scheduling algorithm for a basic OS. Each ready process gets an equal time slice in a cyclic manner.
    *   **Benefit:** Easy to implement and provides a reasonable distribution of CPU time among processes.
3.  **Assembly-Level Context Switching (`PendSV_Handler`):**
    *   **Why:** Direct manipulation of CPU registers and stack pointers is required for efficient and correct context switching. C code cannot achieve this level of control.
    *   **Benefit:** High performance and precise control over the CPU state during a switch.
4.  **Separation of Scheduler Logic (C) and Context Switch (Assembly):**
    *   **Why:** The `SysTick_Handler` pends PendSV, which then calls the C-level `schedule()` function. This separates the policy (which process to run next) from the mechanism (how to switch contexts).
    *   **Benefit:** Makes the scheduler logic easier to understand, modify, and debug in C, while keeping the performance-critical context switch in optimized assembly.

**Implementation Details and Execution Flow:**

**1. SysTick Initialization (`systick_init` in `src/systick.c`)**
    *   **File:** `/home/admin/Documents/OS/src/systick.c`
    *   **Action:**
        *   `systick_init(frequency)` is called during kernel startup (from `kmain`).
        *   It configures the ARM Cortex-M SysTick timer:
            *   Disables SysTick initially.
            *   Calculates a `reload_value` based on the `SYSTEM_CLOCK_HZ` and desired `frequency` (e.g., 100Hz).
            *   Sets the `SYSTICK_LOAD` register with this value and clears `SYSTICK_VAL`.
            *   Enables SysTick, its interrupt, and sets it to use the processor clock by writing to `SYSTICK_CTRL`.

**2. SysTick Interrupt (`SysTick_Handler` in `src/systick.c`)**
    *   **File:** `/home/admin/Documents/OS/src/systick.c`
    *   **Action:**
        *   When the SysTick timer counts down to zero, a `SysTick` exception occurs, and the CPU jumps to `SysTick_Handler` (as defined in the vector table in `/home/admin/Documents/OS/src/boot.s`).
        *   `SysTick_Handler` increments a global `tick_count`.
        *   It iterates through the `pcb_table`: for any process in the `PROC_STATE_SLEEPING` state, it decrements `sleep_ticks`. If `sleep_ticks` reaches zero, the process's state is changed to `PROC_STATE_READY`, making it eligible for scheduling.
        *   Crucially, it sets the `PENDSVSET` bit in the `ICSR` register. This *pends* a PendSV exception. Because PendSV has a lower priority than SysTick, the PendSV handler will only execute *after* the `SysTick_Handler` completes and any other higher-priority interrupts have been serviced. This ensures the context switch happens at a safe, opportune moment.

**3. PendSV Interrupt and Context Switch (`PendSV_Handler` in `src/interrupt.c`)**
    *   **File:** `/home/admin/Documents/OS/src/interrupt.c`
    *   **Action:**
        *   Once the `SysTick_Handler` finishes and no higher-priority interrupts are active, the pending `PendSV` exception is taken, and the CPU jumps to `PendSV_Handler` (as defined in the vector table in `/home/admin/Documents/OS/src/boot.s`).
        *   `PendSV_Handler` is an assembly function (`__attribute__((naked))`) for precise control:
            *   **Save Current Process Context:**
                *   It checks if `current_process` is valid (i.e., a process was running).
                *   It reads the current Process Stack Pointer (PSP) using `mrs r0, psp`.
                *   It saves the general-purpose registers `R4-R11` onto the current process's stack (pointed to by PSP). The ARM hardware automatically saved `R0-R3`, `R12`, `LR`, `PC`, and `xPSR` when the exception occurred.
                *   The updated stack pointer (after saving `R4-R11`) is then stored in `current_process->sp`.
            *   **Call C-level Scheduler:**
                *   Interrupts are temporarily enabled (`cpsie i`) to allow the C code to run without being immediately interrupted by other low-priority events.
                *   The `schedule()` C function (from `/home/admin/Documents/OS/src/proc.c`) is called. This function determines which process should run next and updates the global `current_process` pointer accordingly.
                *   Interrupts are then disabled (`cpsid i`) before returning to assembly.
            *   **Restore Next Process Context:**
                *   It loads the stack pointer (`sp`) from the newly selected `current_process->sp` into `r0`.
                *   It restores the general-purpose registers `R4-R11` from the stack pointed to by `r0`.
                *   It writes the new stack pointer (`r0`) to the PSP using `msr psp, r0`.
            *   **Return from Exception:**
                *   It loads `r0` with the `EXC_RETURN` value (`0xFFFFFFF9`).
                *   `bx r0` is executed. This special return instruction causes the CPU to:
                    *   Switch back to Thread mode (unprivileged).
                    *   Restore the automatically saved registers (`R0-R3`, `R12`, `LR`, `PC`, `xPSR`) from the stack.
                    *   Resume execution of the newly scheduled process at the instruction pointed to by its restored PC.

**4. C-Level Scheduler Logic (`schedule` in `src/proc.c`)**
    *   **File:** `/home/admin/Documents/OS/src/proc.c`
    *   **Action:**
        *   `schedule()` is called by `PendSV_Handler`.
        *   If a `current_process` was running and its state was `PROC_STATE_RUNNING`, its state is changed to `PROC_STATE_READY` (unless it was blocked or sleeping by a syscall).
        *   It then performs a round-robin search through the `pcb_table` (starting from the next slot after the previous `current_process`) to find a process in the `PROC_STATE_READY` state.
        *   Once a `READY` process is found, `current_process` is updated to point to its PCB, and its state is set to `PROC_STATE_RUNNING`.
        *   If no `READY` process is found, and the original process is still `READY`, it remains the `current_process`. If no processes are ready at all, the system effectively idles (though a real OS would typically switch to an explicit idle task).

**5. Initial Process Start (`proc_switch_to_user` from `scheduler_start`)**
    *   **File:** `/home/admin/Documents/OS/src/proc.c` (calls `proc_switch_to_user`) and likely `/home/admin/Documents/OS/kernel.s` (or similar assembly file) for `proc_switch_to_user` implementation.
    *   **Action:**
        *   The `scheduler_start()` function, called from `kmain` to launch the very first user process, directly calls `proc_switch_to_user(current_process->sp)`.
        *   This assembly function is similar to the "restore context" part of `PendSV_Handler`. It takes the prepared stack pointer of the first user process, restores its registers, sets the PSP, and uses `EXC_RETURN` to transition to user mode and begin execution of the first application.

---

## 6. File System: Ramdisk Structure and Operations

This OS implements a simple, in-memory filesystem (ramdisk) to store and manage user application binaries and other data. The ramdisk is static, meaning its content is determined at build time by a host-side tool and embedded directly into the kernel image.

**Architectural Layout:**

*   **Ramdisk in Kernel Memory:** The entire ramdisk image is linked directly into the kernel's RAM space, specifically within the `.ramdisk` section defined in `/home/admin/Documents/OS/src/linker.ld`. The kernel accesses it directly via `__ramdisk_start` and `__ramdisk_end` symbols.
*   **Superblock:** Located at the very beginning of the ramdisk (Block 0), the `superblock_t` structure (defined in `/home/admin/Documents/OS/src/include/fs.h`) contains global filesystem metadata like a magic number, total blocks, and the starting block for data. 
*   **Inode Table:** Immediately following the superblock, the inode table is an array of `inode_t` structures (defined in `/home/admin/Documents/OS/src/include/fs.h`). Each inode describes a file or directory, holding metadata such as type, size, and pointers to its data blocks.
*   **Data Blocks:** The rest of the ramdisk is composed of data blocks, which store the actual file contents and directory entries.
*   **Directory Entries:** Directories are special files whose data blocks contain an array of `dirent_t` structures (defined in `/home/admin/Documents/OS/src/include/fs.h`). Each `dirent_t` links a filename to an inode number.
*   **Per-Process File Descriptor Table:** Each `pcb_t` (Process Control Block, defined in `/home/admin/Documents/OS/src/include/proc.h`) contains an array of `file_t` pointers (`fd_table`). A `file_t` structure (defined in `/home/admin/Documents/OS/src/include/fs.h`) represents an *open* file, tracking its associated inode, current read/write offset, and open flags.

**Design Decisions:**

1.  **Static, In-Memory Ramdisk:**
    *   **Why:** Simplifies filesystem implementation significantly by avoiding complex block device drivers, caching, and persistent storage concerns. The entire filesystem is available immediately upon kernel boot.
    *   **Benefit:** Fast access to files, simpler kernel code, and predictable behavior. Ideal for embedded systems where the set of available applications is known at compile time.
    *   **Limitation:** Filesystem content cannot be changed at runtime (no dynamic file creation/deletion or modification of file sizes beyond pre-allocated blocks). All content is lost on reboot.
2.  **Host-Side `mkfs` Tool:**
    *   **Why:** To construct the ramdisk image on the development machine, populating it with application binaries and other necessary files. This separates the complex task of filesystem creation from the kernel.
    *   **Benefit:** Simplifies kernel code, as the kernel only needs to *read* the pre-built filesystem.
3.  **Direct Block Addressing:**
    *   **Why:** The `inode_t` uses an array of `direct_blocks` to point directly to data blocks. This is the simplest way to map file data to storage.
    *   **Benefit:** Easy to implement and efficient for small files.
    *   **Limitation:** Limits the maximum file size (e.g., 12 direct blocks * BLOCK_SIZE). Larger files would require indirect block schemes, which are not implemented.
4.  **Unix-like File Descriptors:**
    *   **Why:** Provides a familiar and robust interface for user applications to interact with files and devices.
    *   **Benefit:** Standardizes I/O operations and simplifies user-space library development.

**Implementation Details and Execution Flow:**

**1. Ramdisk Image Creation (`tools/mkfs.c`)**
    *   **File:** `/home/admin/Documents/OS/tools/mkfs.c`
    *   **Action:**
        *   The `mkfs` utility is run on the host machine during the build process.
        *   It initializes a `ramdisk` array in host memory.
        *   It populates the `superblock_t` at the start of this array, including the `FS_MAGIC` number.
        *   It initializes the `inode_t` table, reserving Inode 0 as unused and Inode 1 as the root directory.
        *   It creates `.` and `..` directory entries within the root directory's data block.
        *   For each application binary or file specified on the command line:
            *   It reads the file's content from the host filesystem.
            *   It allocates a new `inode_t` in the ramdisk's inode table, setting its type (file), size, and assigning available data blocks.
            *   The file's content is copied into these data blocks within the ramdisk array.
            *   A `dirent_t` entry (containing the filename and inode number) is added to the root directory's data block.
        *   Finally, the entire `ramdisk` array is written as a binary image file (e.g., `ramdisk.img`). This image is then linked into the kernel.

**2. Filesystem Initialization (`fs_init` in `src/fs.c`)**
    *   **File:** `/home/admin/Documents/OS/src/fs.c` and `/home/admin/Documents/OS/src/kmain.c`
    *   **Action:**
        *   During kernel startup, `kmain` calls `fs_init()`.
        *   `fs_init()` checks if the ramdisk image was successfully linked (`__ramdisk_start` != `__ramdisk_end`).
        *   It then casts the `__ramdisk_start` address to a `superblock_t*` and validates its `magic` number.
        *   If the magic number is correct, `fs_superblock` is set, and the filesystem is considered mounted.

**3. File Lookup (`fs_lookup` in `src/fs.c`)**
    *   **File:** `/home/admin/Documents/OS/src/fs.c`
    *   **Action:**
        *   `fs_lookup(path)` is used by kernel functions (like `sys_exec` or `sys_open`) to resolve a given path to an `inode_t`.
        *   It starts by getting the inode for the root directory (Inode 1).
        *   It tokenizes the input `path` (e.g., "app/shell_app.proc" becomes "app", then "shell_app.proc").
        *   For each path component, it reads the directory entries (`dirent_t`s) from the current directory's data block.
        *   It compares the component name with the `dirent_t.name`. If a match is found, it retrieves the corresponding `inode_t` (using `dirent_t.inode_num`) and continues the search.
        *   If all components are resolved, it returns a pointer to the final `inode_t`. If any component is not found or a non-directory is traversed, it returns `NULL`.

**4. Opening a File (`sys_open` in `src/syscalls.c`)**
    *   **Files:** `/home/admin/Documents/OS/src/syscalls.c`, `/home/admin/Documents/OS/src/fs.c`, `/home/admin/Documents/OS/src/proc.c`
    *   **Action:**
        *   A user application calls `open(path, flags, mode)`, which triggers the `SYS_OPEN` syscall.
        *   The kernel-side `sys_open` function is invoked.
        *   It first uses `fs_lookup(path)` to find the `inode_t` for the requested file. If not found, it returns an error.
        *   It then searches the `current_process->fd_table` for an unused file descriptor slot.
        *   A `file_t` structure is allocated (using kernel `malloc`, implying `pmm_alloc` or a wrapper around it) and populated with the found `inode_t`, an initial `offset` of 0, and the provided `flags`.
        *   The pointer to this `file_t` is stored in the `fd_table` at the found slot, and the `fd` (index of the slot) is returned to the user application.

**5. Reading from a File (`sys_read` in `src/syscalls.c`)**
    *   **Files:** `/home/admin/Documents/OS/src/syscalls.c`, `/home/admin/Documents/OS/src/fs.c`, `/home/admin/Documents/OS/src/hal/hal_console.c`
    *   **Action:**
        *   A user application calls `read(fd, buffer, count)`, triggering the `SYS_READ` syscall.
        *   The kernel-side `sys_read` function is invoked.
        *   If `fd` is 0 (stdin), it reads characters from the console using `hal_console_try_getchar()`, blocking the process (`PROC_STATE_BLOCKED`) and calling `schedule()` if no input is available.
        *   For other `fd`s, it validates the file descriptor and checks if the file was opened with read permissions.
        *   It retrieves the `file_t` and its associated `inode_t` from the `current_process->fd_table`.
        *   It calculates how many bytes can actually be read, considering the requested `count`, the current `offset`, and the file's `size`.
        *   It then reads data block by block:
            *   It determines the current data block number from `inode->direct_blocks` based on the `offset`.
            *   It copies data from the ramdisk (starting at `__ramdisk_start + (block_num * BLOCK_SIZE) + block_offset`) into the user-provided `buffer`.
        *   The `file_t->offset` is updated, and the number of bytes read is returned.

**6. Writing to a File (`sys_write` in `src/syscalls.c`)**
    *   **Files:** `/home/admin/Documents/OS/src/syscalls.c`, `/home/admin/Documents/OS/src/fs.c`, `/home/admin/Documents/OS/src/hal/hal_console.c`
    *   **Action:**
        *   A user application calls `write(fd, buffer, count)`, triggering the `SYS_WRITE` syscall.
        *   The kernel-side `sys_write` function is invoked.
        *   If `fd` is 1 (stdout), it writes characters to the console using `hal_console_putc()`.
        *   For other `fd`s, it validates the file descriptor and checks if the file was opened with write permissions.
        *   It retrieves the `file_t` and its associated `inode_t`.
        *   It calculates how many bytes can be written, considering the requested `count`, the current `offset`, and the file's `size` (note: this implementation does not support extending files).
        *   It then writes data block by block:
            *   It determines the current data block number from `inode->direct_blocks`.
            *   It copies data from the user-provided `buffer` into the ramdisk (starting at `__ramdisk_start + (block_num * BLOCK_SIZE) + block_offset`).
        *   The `file_t->offset` is updated, and the number of bytes written is returned.

**7. Closing a File (`sys_close` in `src/syscalls.c`)**
    *   **Files:** `/home/admin/Documents/OS/src/syscalls.c`, `/home/admin/Documents/OS/src/proc.c`
    *   **Action:**
        *   A user application calls `close(fd)`, triggering the `SYS_CLOSE` syscall.
        *   The kernel-side `sys_close` function is invoked.
        *   It validates the `fd`.
        *   It frees the `file_t` structure associated with the `fd` (using kernel `malloc`/`free`, which in this OS is primitive and likely a no-op for `free`).
        *   The `fd_table` entry for that `fd` in the `current_process->fd_table` is set to `NULL`.

**8. Getting File Status (`sys_fstat` in `src/syscalls.c`)**
    *   **File:** `/home/admin/Documents/OS/src/syscalls.c`
    *   **Action:**
        *   A user application calls `fstat(fd, stat_buf)`, triggering the `SYS_FSTAT` syscall.
        *   The kernel-side `sys_fstat` function is invoked.
        *   It validates the `fd` and retrieves the associated `inode_t`.
        *   It populates the user-provided `stat_buf` with information from the `inode_t`, such as `st_mode` (file/directory type), `st_size`, and `st_ino` (inode number).

**9. Seeking in a File (`sys_lseek` in `src/syscalls.c`)**
    *   **File:** `/home/admin/Documents/OS/src/syscalls.c`
    *   **Action:**
        *   A user application calls `lseek(fd, offset, whence)`, triggering the `SYS_LSEEK` syscall.
        *   The kernel-side `sys_lseek` function is invoked.
        *   It validates the `fd` and retrieves the associated `file_t`.
        *   It calculates the `new_offset` based on `whence` (SEEK_SET, SEEK_CUR, SEEK_END) and the provided `offset`.
        *   It performs boundary checks to ensure `new_offset` is within the valid range of the file's size.
        *   If valid, it updates `file_t->offset` and returns the new offset.

## 7. Drivers: Console Driver

The Console Driver provides the fundamental input/output (I/O) capabilities for the operating system, allowing the kernel and user applications to print messages to a display and receive input from a keyboard
(or similar input device). This implementation uses a UART (Universal Asynchronous Receiver/Transmitter) for communication.

**Architectural Layout:**

*   **UART Hardware Registers:** The console driver directly interacts with memory-mapped registers of a UART peripheral. These registers include a Data Register (UART_DR) for sending/receiving data and a Flag
    Register (UART_FR) to check the status of the UART (e.g., if the transmit buffer is full or the receive buffer is empty).
*   **HAL (Hardware Abstraction Layer):** The console driver is part of the HAL, providing a set of generic functions (hal_console_putc, hal_console_getchar, etc.) that abstract away the specific details of the
    underlying UART hardware.
*   **System Calls:** User applications access console I/O indirectly through the sys_read and sys_write system calls, which then delegate to the HAL console driver.

**Design Decisions:**

1.  **Direct UART Register Access:**
    *   **Why:** For maximum performance and minimal overhead in an embedded system, the driver directly reads from and writes to the UART's memory-mapped registers.
    *   **Benefit:** Efficient I/O operations.
    *   **Limitation:** Requires precise knowledge of the target hardware's UART register map. The current implementation uses dummy addresses, which would need to be updated for real hardware.
2.  **Blocking and Non-Blocking Input:**
    *   **Why:** Provides flexibility for different use cases. hal_console_getchar() blocks until input is available, suitable for simple command-line interfaces. hal_console_try_getchar() and
        hal_console_input_available() allow for polling, which is useful when a process needs to do other work while waiting for input.
    *   **Benefit:** Supports both synchronous and asynchronous input handling.
3.  **Kernel-Level Output Functions:**
    *   **Why:** Provides basic debugging and informational output for the kernel itself (hal_console_puts, hal_console_put_int, hal_console_put_hex).
    *   **Benefit:** Essential for kernel development and debugging before a full user-space environment is stable.
4.  **Integration with Syscalls:**
    *   **Why:** To allow user applications to perform console I/O using standard file descriptors (stdin/stdout) through the system call interface.
    *   **Benefit:** Standard Unix-like I/O model for user applications.

**Implementation Details and Execution Flow:**

**1. Hardware Register Definitions (`src/hal/hal_console.c`)**
    *   **File:** `/home/admin/Documents/OS/src/hal/hal_console.c`
    *   **Action:** Defines symbolic constants for the base address of the UART peripheral and offsets for its Data Register (UART_DR) and Flag Register (UART_FR). It also defines flags for checking the transmit FIFO
        full (UART_FR_TXFF) and receive FIFO empty (UART_FR_RXFE) status. These are placeholders and must match the actual hardware.

**2. Console Initialization (`hal_console_init` in `src/hal/hal_console.c`)**
    *   **File:** `/home/admin/Documents/OS/src/hal/hal_console.c`
    *   **Action:** This function is currently a no-operation (no-op). In a real system, it would contain code to configure the UART (e.g., baud rate, data bits, stop bits, parity) to enable communication.

**3. Kernel Output (`hal_console_putc`, `hal_console_puts`, `hal_console_put_int`, `hal_console_put_hex` in `src/hal/hal_console.c`)**
    *   **File:** `/home/admin/Documents/OS/src/hal/hal_console.c`
    *   **Action:**
        *   When the kernel needs to output a character (e.g., for debugging messages in kmain or syscalls.c), it calls hal_console_putc(char c).
        *   hal_console_putc enters a loop, continuously checking the UART_FR_TXFF bit in the UART_FR register. This loop waits until the UART's transmit FIFO is not full, indicating it's ready to accept another
            character.
        *   Once the FIFO is ready, the character c is written to the UART_DR register, which sends it out via the UART.
        *   hal_console_puts iterates through a string, calling hal_console_putc for each character.
        *   hal_console_put_int and hal_console_put_hex convert numerical values to string representations and then use hal_console_puts.

**4. Kernel Input (`hal_console_getchar`, `hal_console_try_getchar`, `hal_console_input_available` in `src/hal/hal_console.c`)**
    *   **File:** `/home/admin/Documents/OS/src/hal/hal_console.c`
    *   **Action:**
        *   hal_console_getchar(): Enters a loop, checking the UART_FR_RXFE bit in UART_FR. It waits until the receive FIFO is not empty, indicating a character has been received. It then reads and returns the
            character from UART_DR.
        *   hal_console_try_getchar(): Checks UART_FR_RXFE once. If the FIFO is empty, it immediately returns -1. Otherwise, it reads and returns the character from UART_DR.
        *   hal_console_input_available(): Simply returns the state of the UART_FR_RXFE bit, indicating whether a character is waiting to be read.

**5. User-Space Console I/O via Syscalls (`sys_read`, `sys_write` in `src/syscalls.c`)**
    *   **Files:** `/home/admin/Documents/OS/src/syscalls.c`, `/home/admin/Documents/OS/src/hal/hal_console.c`, `/home/admin/Documents/OS/src/proc.c`
    *   **Action (Output - `sys_write`):**
        *   When a user application calls write(1, buffer, count) (writing to stdout), the SYS_WRITE syscall is triggered.
        *   The kernel-side sys_write function (in /home/admin/Documents/OS/src/syscalls.c) detects file == 1.
        *   It then iterates through the buffer, calling hal_console_putc() for each character, effectively sending the user's output to the UART.
    *   **Action (Input - `sys_read`):**
        *   When a user application calls read(0, buffer, count) (reading from stdin), the SYS_READ syscall is triggered.
        *   The kernel-side sys_read function (in /home/admin/Documents/OS/src/syscalls.c) detects file == 0.
        *   It enters a loop to read count characters:
            *   Inside the loop, it calls hal_console_input_available().
            *   If no input is available, the current_process's state is set to PROC_STATE_BLOCKED, and schedule() is called, causing the process to yield the CPU until input arrives.
            *   Once input is available, hal_console_try_getchar() reads the character, which is then placed into the user's buffer.
        *   The number of characters read is returned to the user application.

---

## 8. Bootloader

The bootloader is the very first piece of code that executes on the ARM Cortex-M0 processor after a reset. Its primary responsibilities are to initialize the system to a known good state, set up the stack, and transfer control to the main kernel entry point (`kmain`).

**Architectural Layout:**

*   **Vector Table (`.isr_vector` section):** This is a critical data structure located at a fixed memory address (typically `0x0` or an address specified by the microcontroller's boot configuration). It contains the initial stack pointer value and the entry addresses of various exception handlers, including the Reset Handler, NMI Handler, HardFault Handler, SVC Handler, PendSV Handler, and SysTick Handler.
*   **Reset Handler (`Reset_Handler`):** The entry point for the CPU after a reset event. It's responsible for initial system setup before jumping to the C-level kernel.
*   **Weak Aliases:** The bootloader defines "weak" aliases for most exception handlers. This allows the C code to provide its own implementations of these handlers, which will override the default (looping) handler provided in the assembly.

**Design Decisions:**

1.  **Assembly Language for Initial Boot:**
    *   **Why:** Assembly language is necessary for the very first stages of boot because the C runtime environment (stack, initialized data) is not yet set up. It provides direct control over CPU registers and memory.
    *   **Benefit:** Ensures precise control over the processor's state from the moment it powers on or resets.
2.  **Vector Table Placement:**
    *   **Why:** The ARM Cortex-M architecture mandates that the vector table be at a specific, known location (often `0x0` or configurable via `VTOR` register). The `.isr_vector` section ensures this placement.
    *   **Benefit:** Allows the CPU to correctly find the addresses of exception handlers.
3.  **`_estack` for Initial Stack Pointer:**
    *   **Why:** The first entry in the vector table must be the initial value of the Main Stack Pointer (MSP). `_estack` is a symbol typically provided by the linker script, marking the end of RAM, which is where the stack usually starts and grows downwards.
    *   **Benefit:** Ensures the stack is correctly initialized before any C code execution.
4.  **`+1` for Thumb Mode Entry Points:**
    *   **Why:** On ARM Cortex-M processors, the least significant bit (LSB) of an address in the vector table indicates the instruction set state (0 for ARM, 1 for Thumb). All Cortex-M processors execute in Thumb mode. Adding `+1` to the handler addresses ensures the CPU correctly enters Thumb mode.
    *   **Benefit:** Correct execution of Thumb instructions.
5.  **Weak Aliases for Handlers:**
    *   **Why:** By declaring handlers as `weak` in assembly and providing a `Default_Handler` that simply loops, the bootloader offers a fallback. C code can then implement specific handlers (e.g., `NMI_Handler`, `HardFault_Handler`) without needing to modify the assembly. If a C implementation is present, it overrides the weak assembly version.
    *   **Benefit:** Modularity and ease of development. C developers can focus on their logic without worrying about the low-level assembly boilerplate unless necessary.

**Implementation Details and Execution Flow:**

**1. Processor Reset and Vector Table Fetch**
    *   **Action:** Upon a reset event, the ARM Cortex-M0 processor automatically performs two actions:
        *   It loads the value at memory address `0x0` into the Main Stack Pointer (MSP). This value is `_estack` from the vector table.
        *   It loads the value at memory address `0x4` into the Program Counter (PC). This value is `Reset_Handler + 1` from the vector table.
    *   **File:** Implicitly handled by the CPU hardware, with values provided by the `.isr_vector` section in `/home/admin/Documents/OS/src/boot.s`.

**2. `Reset_Handler` Execution (`src/boot.s`)**
    *   **File:** `/home/admin/Documents/OS/src/boot.s`
    *   **Action:**
        *   The `Reset_Handler` is executed.
        *   **Call `kmain`:** The instruction `bl kmain` (Branch with Link to `kmain`) is executed. This transfers control to the `kmain` function, which is the main entry point of the kernel written in C. The `bl` instruction saves the return address in the Link Register (LR).
        *   **Infinite Loop (`hang`):** If `kmain` ever returns (which it should not in a typical embedded OS), the `b hang` instruction creates an infinite loop, effectively halting the system to prevent unpredictable behavior.

**3. C Runtime Initialization (Implicit)**
    *   **Action:** Before `kmain` can fully execute, the C runtime environment needs to be set up. While not explicitly shown in `boot.s`, a typical embedded boot process would include:
        *   **Copying Initialized Data (`.data` section):** Copying values from Flash (ROM) to RAM for global and static variables that have initial values.
        *   **Zeroing Uninitialized Data (`.bss` section):** Setting global and static variables without explicit initializers to zero in RAM.
    *   **File:** These steps are usually handled by a C runtime startup file (often generated by the toolchain or a small assembly routine before `bl kmain`). In this specific `boot.s`, it directly branches to `kmain`, implying that either `kmain` itself handles these or they are implicitly handled by the build system's default startup code.

**4. Transfer to `kmain` (`src/kmain.c`)**
    *   **File:** `/home/admin/Documents/OS/src/kmain.c`
    *   **Action:** The `kmain` function, written in C, takes over. It initializes various OS subsystems (process management, filesystem, SysTick timer) and then starts the first user process.

**5. Default Exception Handling (`Default_Handler`)**
    *   **File:** `/home/admin/Documents/OS/src/boot.s`
    *   **Action:** For any exception handler that is not explicitly implemented in C (e.g., if `NMI_Handler` is not defined in a C file), the `Default_Handler` is used. This handler is an infinite loop (`b .`), which effectively halts the system if an unhandled exception occurs, preventing further execution and aiding in debugging.

---

## 9. SysTick Timer (General Purpose)

The SysTick timer is a 24-bit down-counting timer integrated into the ARM Cortex-M processor. While its primary role in this OS is to provide the periodic interrupts necessary for preemptive scheduling (as detailed in the "Scheduler and Context Switching" section), it also serves as a general-purpose system timer for other OS functionalities, such as providing system time and managing process sleep states.

**Architectural Layout:**

*   **SysTick Registers:** The SysTick timer is controlled by a set of memory-mapped registers: `SYSTICK_CTRL` (Control and Status Register), `SYSTICK_LOAD` (Reload Value Register), and `SYSTICK_VAL` (Current Value Register). These registers allow configuration of the timer's behavior, including its enable state, interrupt generation, and clock source.
*   **System Clock:** The SysTick timer operates based on the processor's system clock, defined by `SYSTEM_CLOCK_HZ` (e.g., 100 MHz).
*   **Global Tick Counter:** A `volatile uint32_t` variable, `tick_count`, is maintained by the kernel to keep track of the total number of SysTick interrupts that have occurred since boot. This provides a monotonic time base for the system.

**Design Decisions:**

1.  **Hardware-Assisted Timing:**
    *   **Why:** Utilizing the dedicated SysTick hardware timer provides a highly accurate and efficient mechanism for generating periodic events and tracking time, offloading this task from the main CPU. 
    *   **Benefit:** Reduces CPU overhead for timing-related tasks and ensures consistent timing accuracy.
2.  **Configurable Frequency:**
    *   **Why:** The `systick_init` function allows the kernel to configure the SysTick interrupt frequency (e.g., 100 Hz). This flexibility enables tuning the scheduler's preemption rate and the granularity of time-based services.
    *   **Benefit:** Adaptability to different system requirements and performance needs.
3.  **Global Monotonic Time Source:**
    *   **Why:** The `tick_count` variable provides a simple, incrementing counter that represents the passage of time in SysTick ticks. This is a fundamental building block for any time-dependent OS service.
    *   **Benefit:** Offers a reliable and easily accessible system-wide time reference.

**Implementation Details and Execution Flow:**

**1. SysTick Initialization (`systick_init` in `src/systick.c`)**
    *   **File:** `/home/admin/Documents/OS/src/systick.c`
    *   **Action:**
        *   Called early in the kernel's `kmain` function with a desired frequency (e.g., `systick_init(100)` for 100 Hz).
        *   The function first disables the SysTick timer to ensure a clean configuration.
        *   It calculates the `reload_value` for the `SYSTICK_LOAD` register using the formula `(SYSTEM_CLOCK_HZ / frequency) - 1`. This value determines the number of clock cycles before the timer counts down to zero and generates an interrupt.
        *   The `SYSTICK_LOAD` register is set with this calculated value, and `SYSTICK_VAL` (the current value) is cleared.
        *   Finally, the `SYSTICK_CTRL` register is configured to enable the SysTick timer, enable its interrupt, and select the processor clock as its clock source.

**2. SysTick Interrupt Handler (`SysTick_Handler` in `src/systick.c`)**
    *   **File:** `/home/admin/Documents/OS/src/systick.c`
    *   **Action:**
        *   This function is automatically invoked by the CPU whenever the SysTick timer counts down to zero (at the configured frequency).
        *   **Increment `tick_count`:** The global `tick_count` variable is incremented. This is the core mechanism for tracking system uptime and providing a time base.
        *   **Process Sleep Management:** The handler iterates through all Process Control Blocks (`pcb_table`). For any process in the `PROC_STATE_SLEEPING` state, it decrements its `sleep_ticks` counter. If `sleep_ticks` reaches zero, the process's state is changed to `PROC_STATE_READY`, making it eligible for scheduling.
        *   **Pend PendSV for Scheduling:** As described in the "Scheduler and Context Switching" section, the handler pends a PendSV exception. This ensures that a context switch can occur after the SysTick interrupt has completed, allowing the scheduler to run and potentially switch to a newly awakened or ready process.

**3. System Time (`sys_gettimeofday` in `src/syscalls.c`)**
    *   **File:** `/home/admin/Documents/OS/src/syscalls.c`
    *   **Action:**
        *   User applications can call the `sys_gettimeofday` system call to retrieve the current system time.
        *   This syscall reads the `tick_count` and `SYSTICK_HZ` values.
        *   It then calculates the number of seconds (`tv_sec`) and microseconds (`tv_usec`) that have elapsed since boot based on these values, providing a basic timestamp to the user application.

**4. Process Sleeping (`sys_sleep` in `src/syscalls.c`)**
    *   **File:** `/home/admin/Documents/OS/src/syscalls.c`
    *   **Action:**
        *   User applications can call `sys_sleep(milliseconds)` to pause their execution for a specified duration.
        *   The syscall converts the requested `milliseconds` into `sleep_ticks` (the number of SysTick interrupts that need to occur before the process wakes up).
        *   The `current_process`'s state is set to `PROC_STATE_SLEEPING`, and its `sleep_ticks` counter is initialized.
        *   Finally, `schedule()` is called, which causes the sleeping process to yield the CPU, allowing other ready processes to run. The `SysTick_Handler` will eventually decrement `sleep_ticks` and wake the process.

---

## 10. Fault Handling (`fault.c`)

Fault handling is a critical aspect of operating system stability, especially in embedded systems where unexpected hardware or software errors can occur. This OS provides a basic mechanism to catch and report `HardFault` exceptions, which are general-purpose fault conditions that can arise from various error sources.

**Architectural Layout:**

*   **HardFault Exception:** An ARM Cortex-M processor generates a `HardFault` exception when a fault condition occurs that is not handled by other specific fault handlers (e.g., BusFault, MemManage Fault) or when a fault escalates to a higher priority.
*   **Vector Table Integration:** The `HardFault_Handler` is registered in the CPU's vector table (defined in `boot.s`). When a `HardFault` occurs, the CPU automatically branches to this handler.
*   **Hardware-Stacked Frame:** Upon entry into any exception handler on Cortex-M, the CPU automatically saves a set of registers (`R0-R3`, `R12`, `LR`, `PC`, and `xPSR`) onto the stack of the exception. This "stacked frame" captures the state of the CPU immediately before the fault occurred.

**Design Decisions:**

1.  **Assembly and C Handler Separation:**
    *   **Why:** A small assembly wrapper (`HardFault_Handler`) is used to extract the correct stack pointer (MSP or PSP) before calling a C function (`HardFault_Handler_C`). This is necessary because C code cannot directly determine which stack was active. The C function then handles the more complex task of logging.
    *   **Benefit:** Combines the precise control of assembly for critical context extraction with the readability and maintainability of C for diagnostic reporting.
2.  **Detailed Register Dumping:**
    *   **Why:** When a `HardFault` occurs, understanding the CPU's state at the moment of the fault is crucial for debugging. Dumping the stacked registers provides immediate insight into what the processor was doing when the error happened.
    *   **Benefit:** Significantly aids in diagnosing the root cause of crashes and unexpected behavior.
3.  **System Halt on Fatal Error:**
    *   **Why:** In an embedded OS, many `HardFault` conditions are unrecoverable errors. Continuing execution could lead to unpredictable behavior, data corruption, or further system instability.
    *   **Benefit:** Prevents the system from entering an undefined state, making it safer and indicating a clear fault condition.
    *   **Limitation:** This approach provides no recovery mechanism; manual intervention (e.g., reset) is required after a fault.

**Implementation Details and Execution Flow:**

**1. `HardFault_Handler` (Assembly Wrapper in `src/fault.c`)**
    *   **File:** `/home/admin/Documents/OS/src/fault.c` (assembly part)
    *   **Action:**
        *   When a `HardFault` exception occurs, the CPU's hardware automatically saves the context of the interrupted task (registers R0-R3, R12, LR, PC, xPSR) onto the active stack (either MSP or PSP).
        *   The `HardFault_Handler` assembly function is invoked. Its first task is to determine whether the Main Stack Pointer (MSP) or Process Stack Pointer (PSP) was in use before the exception. It does this by reading the PSP and comparing it with the current stack pointer (which will be MSP in handler mode).
        *   Once the correct stack pointer for the *faulted context* is identified (which points to the hardware-stacked frame), its value is passed as an argument to the C function `HardFault_Handler_C`.
        *   Control is then transferred to `HardFault_Handler_C`.

**2. `HardFault_Handler_C` (C Implementation in `src/fault.c`)**
    *   **File:** `/home/admin/Documents/OS/src/fault.c` (C part)
    *   **Action:**
        *   This function receives `stacked_frame`, a pointer to the array of `uint32_t` representing the registers saved by hardware and the assembly wrapper.
        *   **Diagnostic Output:** It first prints a clear "--- HARD FAULT ---" header to the console.
        *   It then proceeds to print the individual values of the stacked registers (`R0`, `R1`, `R2`, `R3`, `R12`, `LR`, `PC`, `xPSR`) using `hal_console_put_hex`. This output provides vital information about the program state at the time of the fault, including the Program Counter (PC) which indicates where the fault occurred.
        *   **System Halt:** After printing the diagnostics, it prints "--- System Halted ---" and enters an infinite `while(1);` loop. This action freezes the system, preventing any further execution and allowing a developer to observe the fault information on the console without it being overwritten.

---

## 11. Message Passing (Inter-Process Communication)

Inter-Process Communication (IPC) is a fundamental mechanism that allows independent processes to exchange data and synchronize their actions. In this OS, a basic message passing system is implemented, enabling processes to send and receive messages from each other.

**Architectural Layout:**

*   **Per-Process Message Queue:** Each Process Control Block (`pcb_t`) contains a dedicated, fixed-size message queue (`msg_queue`). This queue acts as a mailbox for the process, storing pointers to incoming messages.
*   **Circular Buffer Implementation:** The `msg_queue` is managed as a circular buffer using `msg_read_idx` and `msg_write_idx` to track the next available slot for reading and writing, respectively.
*   **Message Count:** `msg_count` keeps track of the number of messages currently in the queue.
*   **Message Content (Pointers):** The queue stores `char*` pointers to messages, rather than copying the message content directly. This implies that the sender is responsible for ensuring the message data remains valid until the receiver has processed it.

**Design Decisions:**

1.  **Fixed-Size, Per-Process Queues:**
    *   **Why:** Simplifies memory management and avoids dynamic allocation for message queues, which can be complex in a small embedded kernel. Each process has its own dedicated queue.
    *   **Benefit:** Predictable memory usage and straightforward implementation.
    *   **Limitation:** A hard limit (`MAX_MESSAGES`) on the number of messages a process can queue, potentially leading to `EMSGSIZE` errors if the queue is full.
2.  **Blocking Send/Receive:**
    *   **Why:** The `sys_receive_msg` syscall blocks the calling process if no messages are available. The `sys_send_msg` syscall can unblock a waiting receiver.
    *   **Benefit:** Simplifies synchronization between sender and receiver; the receiver automatically waits for messages.
3.  **Message Pointers, Not Copies:**
    *   **Why:** Passing pointers to messages instead of copying the message content reduces overhead and memory usage, especially for larger messages.
    *   **Benefit:** Efficient message transfer.
    *   **Limitation:** Requires careful management of message buffer lifetimes by user applications to prevent use-after-free or invalid pointer issues. The sender must ensure the message content is stable until the receiver has consumed it.

**Implementation Details and Execution Flow:**

**1. Sending a Message (`sys_send_msg` in `src/syscalls.c`)**
    *   **File:** `/home/admin/Documents/OS/src/syscalls.c`
    *   **Action:**
        *   A user application calls `send_msg(pid, msg_ptr)`, which triggers the `SYS_SEND_MSG` syscall.
        *   **Parameter Validation:** The kernel-side `sys_send_msg` function first validates the target `pid` to ensure it's a valid and active process.
        *   **Queue Full Check:** It checks if the target process's message queue (`target_pcb->msg_queue`) is full by comparing `target_pcb->msg_count` with `MAX_MESSAGES`. If full, it sets `errno` to `EMSGSIZE` and returns -1.
        *   **Enqueue Message:** If space is available, the `msg_ptr` is stored in the `target_pcb->msg_queue` at the `msg_write_idx`.
        *   **Update Queue Pointers:** The `msg_write_idx` is incremented (and wrapped around using the modulo operator `% MAX_MESSAGES` to maintain circularity), and `target_pcb->msg_count` is incremented.
        *   **Wake Receiver (if blocked):** If the `target_pcb->state` is `PROC_STATE_BLOCKED` (indicating it was waiting for a message), its state is changed to `PROC_STATE_READY`, making it eligible for scheduling.
        *   Returns 0 on success.

**2. Receiving a Message (`sys_receive_msg` in `src/syscalls.c`)**
    *   **File:** `/home/admin/Documents/OS/src/syscalls.c`
    *   **Action:**
        *   A user application calls `receive_msg()`, which triggers the `SYS_RECEIVE_MSG` syscall.
        *   **Parameter Validation:** The kernel-side `sys_receive_msg` function first validates the `current_process`.
        *   **Queue Empty Check and Blocking:** It checks if the `current_process->msg_queue` is empty (`current_process->msg_count == 0`).
            *   If empty, the `current_process->state` is set to `PROC_STATE_BLOCKED`, and `schedule()` is called. This causes the process to yield the CPU and wait until a message is sent to it.
            *   After `schedule()` returns (meaning the process has been woken up, presumably by a `sys_send_msg` call), it rechecks `msg_count`. If it's still 0 (an unexpected scenario if the sender correctly woke it), it sets `errno` to `EAGAIN` and returns `NULL`.
        *   **Dequeue Message:** If messages are available, the `char* msg` is retrieved from `current_process->msg_queue` at the `msg_read_idx`.
        *   **Clear Slot and Update Pointers:** The slot in the queue is cleared (set to `NULL`), `msg_read_idx` is incremented (circularly), and `current_process->msg_count` is decremented.
        *   Returns the `msg` pointer on success.

---