/*
 * MokshithOS Main Kernel File (kmain.c)
 */
#include "include/display.h" // Use the new standardized header
#include "include/shell.h"

// DEFINE THE PHYSICAL ADDRESS OF YOUR FRAMEBUFFER HERE
// This address comes from your hardware design (e.g., Vivado address editor).
#define FRAMEBUFFER_ADDR ((uint8_t*)0x21000000) // EXAMPLE ADDRESS! YOU MUST CHANGE THIS!

// This is the entry point called from boot.s
void kmain(void) {
    // Initialize our hardware drivers
    display_init(FRAMEBUFFER_ADDR);

    // Set up colors and clear the screen
    display_set_color(0x00FFFF, 0x00008B); // Cyan text on Dark Blue
    display_clear_screen(0x00008B);        // Dark Blue background

    // Set cursor position and print a welcome message
    display_set_cursor(10, 5);
    display_puts("MokshithOS Kernel Booted Successfully!\n");

    display_set_cursor(10, 7);
    display_set_color(0xFFD700, 0x00008B); // Gold text on Dark Blue
    display_puts("Testing integer print: ");
    display_put_int(-12345);
    display_puts("\n");

    display_set_cursor(10, 8);
    display_puts("Testing hex print: 0xDEADBEEF -> ");
    display_put_hex(0xDEADBEEF);
    display_puts("\n");

    // Start the shell
    shell_run();

    // We will add the rest of the OS here. For now, just halt.
    while(1);
}