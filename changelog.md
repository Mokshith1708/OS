# Changelog

This log tracks all code modifications starting from the OS planning phase.

---

### 2025-10-30

*   **`src/mmu.c`**: Replaced the Cortex-M3 specific MPU implementation with stub functions. This corrects the architecture mismatch for the Cortex-M0 target, which lacks an MPU. The functions are now no-ops to maintain API compatibility.
*   **`src/jump_trampoline.s`**: Added a new assembly function `proc_switch_to_user`. This function uses the ARM exception return mechanism to switch the CPU from privileged mode to unprivileged Thread Mode, which is the correct way to start a user process.
*   **`src/proc.c`**: Modified `start_process` to correctly launch a process in unprivileged mode. It now prepares a fake exception frame on the user stack and calls `proc_switch_to_user` to perform the privilege drop. Removed Cortex-M3 specific code.
*   **`src/include/proc.h`**: Introduced the Process Control Block (`pcb_t`) struct and process state enum (`proc_state_t`). This is the core data structure for process management.
*   **`src/proc.c`**: Added core scheduler data structures (`pcb_table`, `current_process`) and an initialization function (`proc_init`).
*   **`src/include/proc.h`**: Added the prototype for `proc_init`.
*   **`src/proc.c`**: Refactored process creation and scheduling. `start_process` now only creates a process and sets it to READY. A new `schedule()` function is added to find a READY process and run it.
*   **`src/include/proc.h`**: Added the prototype for `schedule`.
*   **`src/kmain.c`**: Updated to be the main kernel initialization routine, calling `proc_init()` and `shell_run()`.
*   **`src/shell.c`**: The `run` command now calls `schedule()` after creating a process, allowing the scheduler to run the new process.
*   **`src/include/systick.h`**: Created new header for the SysTick timer initialization.
*   **`src/systick.c`**: Created new file with implementation for `systick_init()` and the `SysTick_Handler()`. For now, the handler only prints a debug message.
*   **`src/kmain.c`**: Added a call to `systick_init(100)` to enable a 100Hz kernel tick.
*   **`src/systick.c`**: `SysTick_Handler` now triggers a `PendSV` exception to request a context switch.
*   **`src/jump_trampoline.s`**: Added the `PendSV_Handler` assembly routine to perform the actual context switch (saving and restoring registers).
*   **`src/proc.c`**: Added `schedule_next()` function to implement a round-robin scheduling policy.
*   **`src/include/proc.h`**: Added the prototype for `schedule_next`.
*   **`apps/shell/`**: Created new directory for the user-space shell application.
*   **`apps/shell/build_app.sh`**: Created build script for the shell application.
*   **`apps/shell/shell_app.c`**: Created the user-space shell application code.
*   **`apps/shell/shell_app.ld`**: Copied linker script for the shell application.
*   **`apps/shell/vectors.s`**: Copied vector table for the shell application.
*   **`src/shell.c`**: Deleted old kernel-space shell source file.
*   **`src/include/shell.h`**: Deleted old kernel-space shell header file.
*   **`src/syscalls.c`**: Implemented polling `_read` syscall (SVC #3) and renumbered `_exit` (SVC #1) and `_write` (SVC #4).
*   **`src/libuser/user_syscalls.c`**: Updated user-space syscall wrappers to match new kernel syscall numbers.
*   **`src/libuser/user_startup.S`**: Updated `_exit` syscall number to SVC #1.
*   **`apps/libuser/include/string.h`**: Created user-space `string.h` header.
*   **`apps/libuser/string.c`**: Created user-space `strcmp` and `strlen` implementations.
*   **`apps/libuser/include/stdio.h`**: Created user-space `stdio.h` header.
*   **`apps/libuser/stdio.c`**: Created user-space `putchar`, `puts`, `getchar`, and simple `printf` implementations.
*   **`apps/shell/build_app.sh`**: Updated to correctly compile and link `libuser` files, added `-nostartfiles -nostdlib -lgcc` flags, and added `objcopy` step.
*   **`src/proc.h`**: Renamed `schedule()` to `scheduler_start()` and `schedule_next()` to `schedule()`. Added `void scheduler_start(void);` and `void schedule(void);`.
*   **`src/proc.c`**: Renamed `schedule()` to `scheduler_start()` and `schedule_next()` to `schedule()`. Updated `scheduler_start` to halt if no processes are ready.
*   **`src/jump_trampoline.s`**: Updated `PendSV_Handler` to call `schedule()`.
*   **`src/kmain.c`**: Removed `shell.h` include and modified `kmain` to create the shell process and call `scheduler_start()`.
*   **`CMakeLists.txt`**: Removed `src/shell.c` from kernel build sources.
