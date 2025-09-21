/* hal_console.c - Semihosting console for MPS2-QEMU */
#include "hal/hal_console.h"
#include "semihost.h"
#include <stdint.h>

/* Initialize console - nothing needed for semihosting */
void hal_console_init(void) {
    /* Semihosting handles I/O automatically */
}


/* Send a single character */
void hal_console_putc(char c) {
    char buf[2] = {c, '\0'};
    sh_write0(buf);
}

/* Send a null-terminated string */
void hal_console_puts(const char *s) {
    sh_write0(s);
}

/* Print an integer in decimal */
void hal_console_put_int(int n) {
    char buf[12];
    int i = 0;

    if (n == 0) { buf[0]='0'; buf[1]='\0'; sh_write0(buf); return; }

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

    sh_write0(buf);
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

    sh_write0(buf);
}

/* Read a single character (blocking) */
int hal_console_getchar(void) {
    char c;
    int ret = sh_read(0, &c, 1); // file descriptor 0 = stdin
    if (ret < 0) return 0;
    return (int)c;
}
