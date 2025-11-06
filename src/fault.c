#include <stdint.h>
#include "include/hal_console.h"

void DataAbort_Handler_C(uint32_t *stacked_frame) {
    hal_console_puts("\n--- DATA ABORT ---\n");
    // Print out the stacked registers
    for (int i = 0; i < 16; i++) {
        hal_console_puts("  R");
        hal_console_put_int(i);
        hal_console_puts(":  0x");
        hal_console_put_hex(stacked_frame[i]);
        hal_console_puts("\n");
    }
    hal_console_puts("\n--- System Halted ---\n");
    while(1);
}

void PrefetchAbort_Handler_C(uint32_t *stacked_frame) {
    hal_console_puts("\n--- PREFETCH ABORT ---\n");
    // Print out the stacked registers
    for (int i = 0; i < 16; i++) {
        hal_console_puts("  R");
        hal_console_put_int(i);
        hal_console_puts(":  0x");
        hal_console_put_hex(stacked_frame[i]);
        hal_console_puts("\n");
    }
    hal_console_puts("\n--- System Halted ---\n");
    while(1);
}

__attribute__((naked))
void DataAbort_Handler(void) {
    __asm__ volatile (
        "push {r0-r12, lr}\n"
        "mov r0, sp\n"
        "bl DataAbort_Handler_C\n"
        "pop {r0-r12, pc}\n"
    );
}

__attribute__((naked))
void PrefetchAbort_Handler(void) {
    __asm__ volatile (
        "push {r0-r12, lr}\n"
        "mov r0, sp\n"
        "bl PrefetchAbort_Handler_C\n"
        "pop {r0-r12, pc}\n"
    );
}
