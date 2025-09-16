// src/syscalls.c
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

// Include your own HAL header for character output
#include "hal/hal_console.h"

#undef errno
extern int errno;

// This function is called by printf, puts, etc. to write characters.
// We redirect its output to our console driver.
int _write(int file, char *ptr, int len) {
    // We only handle stdout (file descriptor 1) and stderr (2)
    if (file != 1 && file != 2) {
        errno = EBADF;
        return -1;
    }
    int i;
    for (i = 0; i < len; i++) {
        hal_console_putc(ptr[i]);
    }
    return len;
}

// _sbrk is used by malloc to request more memory.
// We need a simple heap implementation for the kernel itself.
void *_sbrk(int incr) {
    extern char _kernel_end; // Symbol from linker script marking end of kernel .bss
    static char *heap_end = &_kernel_end;
    extern char _estack; // Symbol for the top of our kernel stack

    char *prev_heap_end = heap_end;

    if (heap_end + incr > &_estack) {
        // Heap and stack collision
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end += incr;
    return (void *)prev_heap_end;
}

// _exit is called when a program (or the kernel) calls exit().
// For now, it just prints a message and halts the CPU.
void _exit(int status) {
    hal_console_puts("\n--- System Halted ---\n");
    (void)status;
    while (1) {
        // Infinite loop to halt execution
    }
}

// The rest of these are stubs for file and process operations.
// We return error codes because our OS does not yet support them.
int _close(int file) { (void)file; errno = EBADF; return -1; }
int _fstat(int file, struct stat *st) { (void)file; st->st_mode = S_IFCHR; return 0; }
int _isatty(int file) { (void)file; return 1; }
int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
int _read(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
int _open(const char *name, int flags, int mode) { (void)name; (void)flags; (void)mode; errno = ENOSYS; return -1; }
int _kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
int _getpid(void) { return 1; }

// Dummy implementations for other functions that might be required.
int _gettimeofday(struct timeval *tv, void *tz) { (void)tv; (void)tz; return 0; }