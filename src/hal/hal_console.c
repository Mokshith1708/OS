#include "include/hal_console.h"
#include <stdint.h>
#include <stdbool.h>
#include "include/display.h" // For display output

// --- New Keyboard Input ---
// Memory-mapped location for keyboard input, as provided by the user.
volatile char *keyboard_input_addr = (volatile char*)0x17A06000 ;

/* Initialize console - nothing needed for this simple implementation */
void hal_console_init(void) {
    /* Initialization for keyboard/display could go here if needed */
}

/* --- Output Functions (Redirected to Display Driver) --- */

/* Send a single character to the display */
void hal_console_putc(char c) {
    display_put_char(c);
}

/* Send a null-terminated string */
void hal_console_puts(const char *s) {
    display_puts(s);
}

/* Print an integer in decimal */
void hal_console_put_int(int n) {
    display_put_int(n);
}

/* Print an unsigned integer in hexadecimal */
void hal_console_put_hex(uint32_t n) {
    display_put_hex(n);
}


/* --- Input Functions (Updated for Keyboard) --- */

/* Read a single character (blocking) */
int hal_console_getchar(void) {
    // Wait (poll) until a character appears at the memory location.
    while (*keyboard_input_addr == 0) {
        // This is a busy-wait loop.
    }
    char t = *keyboard_input_addr;
    *keyboard_input_addr = 0; // Consume the character
    return (int)t;
}
/*
// --- Old UART Implementation ---
int hal_console_getchar(void) {
    // Wait until receive FIFO is not empty
    while (UART_FR & UART_FR_RXFE);
    return (int)UART_DR;
}
*/


/* Try to read a single character (non-blocking) */
int hal_console_try_getchar(void) {
    if (*keyboard_input_addr != 0) {
        char t = *keyboard_input_addr;
        *keyboard_input_addr = 0; // Consume the character
        return (int)t;
    }
    return -1; // No character available
}
/*
// --- Old UART Implementation ---
int hal_console_try_getchar(void) {
    if (UART_FR & UART_FR_RXFE) {
        return -1; // No character available
    }
    return (int)UART_DR;
}
*/


/* Check if input is available */
bool hal_console_input_available(void) {
    return (*keyboard_input_addr != 0);
}
/*
// --- Old UART Implementation ---
bool hal_console_input_available(void) {
    return !(UART_FR & UART_FR_RXFE);
}
*/
