/*
 * MokshithOS Main Kernel File (kmain.c)
 */
#include "include/display.h" 
#include "include/proc.h"
#include "include/systick.h"
#include "include/hal_console.h"
#include "include/fs.h"

// DEFINE THE PHYSICAL ADDRESS OF YOUR FRAMEBUFFER HERE
// This address comes from your hardware design (e.g., Vivado address editor).
#define FRAMEBUFFER_ADDR ((uint8_t*)0x21000000) // EXAMPLE ADDRESS! YOU MUST CHANGE THIS!

// This is the entry point called from boot.s
void kmain(void) {
    hal_console_puts("MokshithOS Kernel Initializing...\r\n");

    // Initialize subsystems
    proc_init();
    fs_init();
    systick_init(100); // Initialize for 100Hz ticks

    // Create the first user process (the shell)
    start_process("shell_app.proc");

    hal_console_puts("Starting scheduler.\r\n");
    // Start the scheduler. This function should never return.
    scheduler_start();

    // If scheduler_start ever returns, something is wrong.
    hal_console_puts("FATAL: scheduler_start returned! Halting.\r\n");
    while(1);
}