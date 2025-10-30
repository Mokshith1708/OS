// src/libuser/user_syscalls.c

// These are the user-space functions that applications will call.
// Their only job is to trigger a Supervisor Call (SVC) exception,
// which transfers control to the OS kernel.

void _exit(int code) {
    // SVC #1 is EXIT
    __asm__ volatile("mov r0, %0; svc #1" : : "r"(code));
    while(1); // Should not be reached
}

int _write(int file, char *ptr, int len) {
    // SVC #4 is WRITE
    register int r0 __asm__("r0") = file;
    register char *r1 __asm__("r1") = ptr;
    register int r2 __asm__("r2") = len;
    register int ret __asm__("r0");
    __asm__ volatile("svc #4" : "=r"(ret) : "r"(r0), "r"(r1), "r"(r2) : "memory");
    return ret;
}

int _read(int file, char *ptr, int len) {
    // SVC #3 is READ
    register int r0 __asm__("r0") = file;
    register char *r1 __asm__("r1") = ptr;
    register int r2 __asm__("r2") = len;
    register int ret __asm__("r0");
    __asm__ volatile("svc #3" : "=r"(ret) : "r"(r0), "r"(r1), "r"(r2) : "memory");
    return ret;
}

void yield(void) {
    // SVC #2 is YIELD
    __asm__ volatile("svc #2");
}

// --- Stubs for other functions to keep the linker happy ---
void *_sbrk(int incr) { return (void*)-1; }
int _close(int file) { return -1; }
int _fstat(int file, void *st) { return 0; }
int _isatty(int file) { return 1; }
int _lseek(int file, int ptr, int dir) { return 0; }
int _kill(int pid, int sig) { return -1; }
int _getpid(void) { return 1; }