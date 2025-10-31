// src/syscalls.c
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

// Include your own HAL header for character output
#include "include/hal_console.h"
#include "include/proc.h" // For schedule()

extern pcb_t *current_process;

// Syscall numbers - MUST match apps/libuser/user_syscalls.c
#define SYS_EXIT  1
#define SYS_YIELD 2
#define SYS_READ  3
#define SYS_WRITE 4
#define SYS_EXEC  5
#define SYS_SBRK  6

#undef errno
extern int errno;

// Forward declarations for kernel-internal syscall implementations
int sys_write(int file, char *ptr, int len);
void* sys_sbrk(intptr_t incr);
void sys_exit(int status);

// sys_exit is called when a program (or the kernel) calls exit().
void sys_exit(int status) {
    hal_console_puts("\n--- Process exited. Cleaning up and reloading shell. ---\n");

    // 1. Free the current process's PCB
    if (current_process) {
        current_process->state = PROC_STATE_UNUSED;
        current_process = NULL;
    }

    // 2. Start a new shell process
    start_process("apps/shell/build/shell_app.proc");

    // 3. Schedule the new shell to run. The old context is abandoned.
    schedule();

    // This point should be unreachable
    while(1);
}

// The rest of these are stubs for file and process operations.
// We return error codes because our OS does not yet support them.
int sys_close(int file) { (void)file; errno = EBADF; return -1; }
int sys_fstat(int file, struct stat *st) { (void)file; st->st_mode = S_IFCHR; return 0; }
int sys_isatty(int file) { (void)file; return 1; }
int sys_lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }

int sys_read(int file, char *ptr, int len) {
    // We only handle stdin (file descriptor 0)
    if (file != 0) {
        errno = EBADF;
        return -1;
    }

    int i;
    for (i = 0; i < len; i++) {
        // This is a blocking, polling read.
        ptr[i] = hal_console_getchar();
    }
    return len;
}

int sys_open(const char *name, int flags, int mode) { (void)name; (void)flags; (void)mode; errno = ENOSYS; return -1; }
int sys_kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
int sys_getpid(void) { return 1; }

// Dummy implementations for other functions that might be required.
int sys_gettimeofday(struct timeval *tv, void *tz) { (void)tv; (void)tz; return 0; }


void svc_handler_c(uint32_t *stack) {
    // 1. Get syscall number from R0, which we set in the wrapper
    int svc_number = stack[0];

    // 2. Get arguments from the process stack
    int r1 = stack[1];
    int r2 = stack[2];
    int r3 = stack[3];

    int ret = -1; // Default return value

    switch (svc_number) {
        case SYS_EXIT: // _exit
            sys_exit(r1);
            break; // sys_exit never returns

        case SYS_YIELD: // yield
            schedule(); // Call the scheduler to switch processes
            ret = 0;
            break;

        case SYS_READ: // _read
            ret = sys_read(r1, (char *)r2, r3);
            break;

        case SYS_WRITE: // _write
            ret = sys_write(r1, (char *)r2, r3);
            break;

        case SYS_EXEC: // exec
            ret = sys_exec((const char *)r1, r2, (char *const *)r3);
            break;

        case SYS_SBRK: // sbrk
            ret = (int)sys_sbrk(r1);
            break;

        default:
            hal_console_puts("Unknown syscall number: ");
            hal_console_put_int(svc_number);
            hal_console_puts("\n");
            break;
    }

    // 4. Store return value in stack frame
    stack[0] = ret;
}

__attribute__((naked))
void SVC_Handler(void) {
    __asm__ volatile (
        // Determine which stack pointer (MSP or PSP) was active
        "mrs r0, psp\n" // Assume PSP is active
        "mov r1, sp\n"  // Get MSP
        "cmp r0, r1\n"  // Compare PSP and MSP
        "bne .L_psp_is_active\n" // If not equal, PSP was active
        "mrs r0, msp\n" // If equal, MSP was active
    ".L_psp_is_active:\n"
        "b svc_handler_c\n"
    );
}