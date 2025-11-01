#include "include/hal_console.h"
#include <stdint.h>
#include <stdbool.h>

// Dummy UART Register Definitions (replace with actual hardware addresses)
#define UART_BASE       0x40002000 // Example base address
#define UART_DR         (*(volatile uint32_t *)(UART_BASE + 0x00)) // Data Register
#define UART_FR         (*(volatile uint32_t *)(UART_BASE + 0x18)) // Flag Register
#define UART_FR_RXFE    (1 << 4) // Receive FIFO Empty
#define UART_FR_TXFF    (1 << 5) // Transmit FIFO Full

/* Initialize console - nothing needed for semihosting */
void hal_console_init(void) {
    /* UART initialization would go here */
}


/* Send a single character */
void hal_console_putc(char c) {
    // Wait until transmit FIFO is not full
    while (UART_FR & UART_FR_TXFF);
    UART_DR = c;
}

/* Send a null-terminated string */
void hal_console_puts(const char *s) {
    while (*s) {
        hal_console_putc(*s++);
    }
}

/* Print an integer in decimal */
void hal_console_put_int(int n) {
    char buf[12];
    int i = 0;

    if (n == 0) { buf[0]='0'; buf[1]='\0'; hal_console_puts(buf); return; }

    int neg = n < 0;
    if (neg) n = -n;

    while (n) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    if (neg) buf[i++] = '-';
    buf[i] = '\0';

    // Reverse string
    for (int j = 0; j < i/2; j++) {
        char t = buf[j]; buf[j] = buf[i-j-1]; buf[i-j-1] = t;
    }

    hal_console_puts(buf);
}

/* Print an unsigned integer in hexadecimal */
void hal_console_put_hex(uint32_t n) {
    char buf[11];
    const char *hex = "0123456789abcdef";

    buf[0] = '0'; buf[1] = 'x';
    for (int i = 0; i < 8; i++) {
        buf[2 + i] = hex[(n >> ((7-i)*4)) & 0xF];
    }
    buf[10] = '\0';

    hal_console_puts(buf);
}

/* Read a single character (blocking) */
int hal_console_getchar(void) {
    // Wait until receive FIFO is not empty
    while (UART_FR & UART_FR_RXFE);
    return (int)UART_DR;
}

/* Try to read a single character (non-blocking) */
int hal_console_try_getchar(void) {
    if (UART_FR & UART_FR_RXFE) {
        return -1; // No character available
    }
    return (int)UART_DR;
}

/* Check if input is available */
bool hal_console_input_available(void) {
    return !(UART_FR & UART_FR_RXFE);
}
