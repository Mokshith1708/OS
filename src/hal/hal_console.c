#include <stdio.h>
#include <stdint.h>

/* --- Console using semihosting --- */

void hal_console_init(void) {
    /* Nothing required; semihosting handles IO */
}

void hal_console_putc(char c) {
    putchar(c);
}

void hal_console_puts(const char *s) {
    fputs(s, stdout);
}

void hal_console_put_int(int n) {
    printf("%d", n);
}

void hal_console_put_hex(uint32_t n) {
    printf("0x%x", n);
}

int hal_console_getchar(void) {
    int c = getchar();
    if (c == EOF) return 0;
    return c;
}
