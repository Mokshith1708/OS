#include <stdint.h>

/*
 * This is a minimal semihosting implementation for the application.
 * It does NOT touch any hardware registers.
 */

// A function to make the special "bkpt" call
static inline int sh_call(int op, void *arg) {
    register int r0 __asm__("r0") = op;
    register void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xAB" : "+r"(r0) : "r"(r1) : "memory");
    return r0;
}

// Semihosting operation number for writing a null-terminated string
#define SH_WRITE0 0x04

// A simple function to print a string using semihosting
void sh_puts(const char *s) {
    sh_call(SH_WRITE0, (void *)s);
}


// The main function. It does NOT initialize any hardware.
int main(void) {
    sh_puts("Hello from QEMU!\n");

    // Loop forever so we know the app didn't crash
    for (;;) {}
    return 0;
}