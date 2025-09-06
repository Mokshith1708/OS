#include "hal_console.h"

#define UART0_BASE 0x4000C000
#define UART0_DR   ((volatile uint32_t*)(UART0_BASE + 0x00))
#define UART0_FR   ((volatile uint32_t*)(UART0_BASE + 0x18))
#define UART_FR_TXFF (1 << 5)
#define UART_FR_RXFE (1 << 4)  /* RX FIFO empty */

static void utoa_hex(uint32_t n, char* buf) {
    const char* hex = "0123456789abcdef";
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        buf[2 + (7 - i)] = hex[(n >> (i * 4)) & 0xF];
    }
    buf[10] = '\0';
}

static void itoa(int n, char* buf) {
    if (n == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    int i = 0;
    int is_negative = n < 0;
    if (is_negative) n = -n;
    while (n != 0) { buf[i++] = (n % 10) + '0'; n /= 10; }
    if (is_negative) buf[i++] = '-';
    buf[i] = '\0';
    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j]; buf[j] = buf[i - j - 1]; buf[i - j - 1] = tmp;
    }
}

void hal_console_init(void) {
    /* For lm3s in QEMU, UART is pre-initialized to 115200 8N1.
       Nothing required unless you want to set registers. */
}

void hal_console_putc(char c) {
    while (*UART0_FR & UART_FR_TXFF);
    *UART0_DR = c;
}

void hal_console_puts(const char *s) {
    while (*s) hal_console_putc(*s++);
}

void hal_console_put_int(int n) {
    char buf[12]; itoa(n, buf); hal_console_puts(buf);
}

void hal_console_put_hex(uint32_t n) {
    char buf[11]; utoa_hex(n, buf); hal_console_puts(buf);
}

/* === NEW === */
int hal_console_getchar(void) {
    while (*UART0_FR & UART_FR_RXFE);   // wait until RX FIFO not empty
    return (int)(*UART0_DR & 0xFF);
}
