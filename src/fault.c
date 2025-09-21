#include <stdint.h>
#include "hal/hal_console.h"

/* SCB system registers (memory-mapped, Cortex-M standard) */
#define SCB_CFSR   (*(volatile uint32_t *)0xE000ED28)
#define SCB_HFSR   (*(volatile uint32_t *)0xE000ED2C)
#define SCB_BFAR   (*(volatile uint32_t *)0xE000ED38)
#define SCB_MMFAR  (*(volatile uint32_t *)0xE000ED34)

void hardfault_dump(uint32_t *stack) {
    uint32_t r0  = stack[0];
    uint32_t r1  = stack[1];
    uint32_t r2  = stack[2];
    uint32_t r3  = stack[3];
    uint32_t r12 = stack[4];
    uint32_t lr  = stack[5];
    uint32_t pc  = stack[6];
    uint32_t psr = stack[7];

    hal_console_puts("\r\n*** HARD FAULT ***\r\n");
    hal_console_puts("R0  = "); hal_console_put_hex(r0);  hal_console_puts("\r\n");
    hal_console_puts("R1  = "); hal_console_put_hex(r1);  hal_console_puts("\r\n");
    hal_console_puts("R2  = "); hal_console_put_hex(r2);  hal_console_puts("\r\n");
    hal_console_puts("R3  = "); hal_console_put_hex(r3);  hal_console_puts("\r\n");
    hal_console_puts("R12 = "); hal_console_put_hex(r12); hal_console_puts("\r\n");
    hal_console_puts("LR  = "); hal_console_put_hex(lr);  hal_console_puts("\r\n");
    hal_console_puts("PC  = "); hal_console_put_hex(pc);  hal_console_puts("\r\n");
    hal_console_puts("xPSR= "); hal_console_put_hex(psr); hal_console_puts("\r\n");

    hal_console_puts("CFSR= "); hal_console_put_hex(SCB_CFSR); hal_console_puts("\r\n");
    hal_console_puts("HFSR= "); hal_console_put_hex(SCB_HFSR); hal_console_puts("\r\n");
    hal_console_puts("BFAR= "); hal_console_put_hex(SCB_BFAR); hal_console_puts("\r\n");
    hal_console_puts("MMFAR="); hal_console_put_hex(SCB_MMFAR); hal_console_puts("\r\n");
}
