#ifndef HAL_CONSOLE_H
#define HAL_CONSOLE_H

#include <stdint.h>

void hal_console_putc(char c);
void hal_console_puts(const char *s);
void hal_console_put_int(int n);
void hal_console_put_hex(uint32_t n);

#endif // HAL_CONSOLE_H