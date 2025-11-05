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
#define FRAMEBUFFER_ADDR ((uint8_t*)0x17A06008) // EXAMPLE ADDRESS! YOU MUST CHANGE THIS!

// This is the entry point called from boot.s
void kmain(void) {
    // NOTE: The framebuffer address is a placeholder and MUST be updated
    // to match the physical address on your FPGA board.
    display_init(FRAMEBUFFER_ADDR);
    display_clear_screen(0x000000); // Clear screen to black

    hal_console_puts("OS Kernel Initializing...\r\n");

    // Initialize subsystems
    proc_init();
    fs_init();
    // systick_init(100); // Already commented out for non-preemptive

    // Create the first user process (the shell)
    start_process("shell_app.proc");

    hal_console_puts("Starting scheduler.\r\n");
    // Start the scheduler. This function should never return.
    scheduler_start();

    // If scheduler_start ever returns, something is wrong.
    hal_console_puts("FATAL: scheduler_start returned! Halting.\r\n");
    while(1);
}