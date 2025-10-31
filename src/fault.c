#include <stdint.h>
#include "include/hal_console.h"

// This function is called from the HardFault_Handler in assembly
void HardFault_Handler_C(uint32_t *stacked_frame) {
    hal_console_puts("\n--- HARD FAULT ---\n");
    hal_console_puts("Stacked registers:\n");
    hal_console_puts("  R0:  0x"); hal_console_put_hex(stacked_frame[0]); hal_console_puts("\n");
    hal_console_puts("  R1:  0x"); hal_console_put_hex(stacked_frame[1]); hal_console_puts("\n");
    hal_console_puts("  R2:  0x"); hal_console_put_hex(stacked_frame[2]); hal_console_puts("\n");
    hal_console_puts("  R3:  0x"); hal_console_put_hex(stacked_frame[3]); hal_console_puts("\n");
    hal_console_puts("  R12: 0x"); hal_console_put_hex(stacked_frame[4]); hal_console_puts("\n");
    hal_console_puts("  LR:  0x"); hal_console_put_hex(stacked_frame[5]); hal_console_puts("\n");
    hal_console_puts("  PC:  0x"); hal_console_put_hex(stacked_frame[6]); hal_console_puts("\n");
    hal_console_puts("  xPSR:0x"); hal_console_put_hex(stacked_frame[7]); hal_console_puts("\n");

    hal_console_puts("\n--- System Halted ---\n");
    while(1);
}

// Naked function for the assembly part of the handler
__attribute__((naked))
void HardFault_Handler(void) {
    __asm__ volatile (
        // Determine which stack pointer (MSP or PSP) was active
        "mrs r0, psp\n" // Assume PSP is active
        "mov r1, sp\n"  // Get MSP
        "cmp r0, r1\n"  // Compare PSP and MSP
        "bne .L_psp_is_active_hf\n" // If not equal, PSP was active
        "mrs r0, msp\n" // If equal, MSP was active
    ".L_psp_is_active_hf:\n"
        "b HardFault_Handler_C\n"
    );
}