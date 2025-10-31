#include "include/unistd.h"
#include <errno.h>

// Syscall numbers
#define SYS_EXIT  1
#define SYS_YIELD 2
#define SYS_READ  3
#define SYS_WRITE 4
#define SYS_EXEC  5
#define SYS_SBRK  6

// Generic syscall function
static inline int syscall(int num, int arg1, int arg2, int arg3) {
    int ret;
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "mov r2, %3\n"
        "mov r3, %4\n"
        "svc 0\n"
        "mov %0, r0\n"
        : "=r" (ret)
        : "r" (num), "r" (arg1), "r" (arg2), "r" (arg3)
        : "r0", "r1", "r2", "r3");
    return ret;
}

// Newlib stubs - these are called by the C library functions
// and make the actual syscalls to the kernel.

void _exit(int status) {
    syscall(SYS_EXIT, status, 0, 0);
    while(1); // Should not return
}

int _read(int file, char *ptr, int len) {
    return syscall(SYS_READ, file, (int)ptr, len);
}

int _write(int file, char *ptr, int len) {
    return syscall(SYS_WRITE, file, (int)ptr, len);
}

void* _sbrk(intptr_t increment) {
    return (void*)syscall(SYS_SBRK, (int)increment, 0, 0);
}

// Dummy implementations for other functions that might be required by Newlib.
// These will eventually be replaced by proper syscalls.
int _close(int file) { (void)file; errno = EBADF; return -1; }
int _fstat(int file, struct stat *st) { (void)file; st->st_mode = S_IFCHR; return 0; }
int _isatty(int file) { (void)file; return 1; }
int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
int _open(const char *name, int flags, int mode) { (void)name; (void)flags; (void)mode; errno = ENOSYS; return -1; }
int _kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
int _getpid(void) { return 1; }
int _gettimeofday(struct timeval *tv, void *tz) { (void)tv; (void)tz; return 0; }

// User-facing syscall wrappers
int exec(const char *path, int argc, char *const argv[]) {
    return syscall(SYS_EXEC, (int)path, argc, (int)argv);
}

void yield(void) {
    syscall(SYS_YIELD, 0, 0, 0);
}

// The write function from unistd.h now calls _write
ssize_t write(int fd, const void *buf, size_t count) {
    return _write(fd, (char*)buf, count);
}