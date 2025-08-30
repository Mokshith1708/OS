#include "hal_console.h"

#define SYS_WRITE0 0x04

// More explicit semihosting call for Cortex-M0
static inline void semihosting_call(unsigned int op, void *arg) {
    register unsigned int r0 asm("r0") = op;
    register void *r1 asm("r1") = arg;
    
    __asm__ volatile (
        "bkpt 0xAB"
        : "+r" (r0)
        : "r" (r1)
        : "memory"
    );
}

void hal_console_puts(const char *s) {
    semihosting_call(SYS_WRITE0, (void *)s);
}