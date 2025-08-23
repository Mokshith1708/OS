#include "uart.h"

void uart_putc(char c) {
    volatile uint32_t* UART0_DR = (uint32_t*)0x09000000;
    *UART0_DR = c;
}

void uart_puts(const char* str) {
    while (*str) uart_putc(*str++);
}
