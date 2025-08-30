#include "hal/hal_console.h"

/*
 * This is the main entry point for the C portion of our kernel.
 * It is called by the Reset_Handler in boot.s after memory is initialized.
 */

int kmain(void) {
    hal_console_puts("Project Monolith Kernel Initialized.\n");
    while (1) { }
    return 0;
}