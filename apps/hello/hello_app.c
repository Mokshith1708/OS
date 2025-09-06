#include <stdint.h>
#define UART0_BASE 0x4000C000u
#define UART0_DR   (*(volatile uint32_t*)(UART0_BASE + 0x00))
#define UART0_FR   (*(volatile uint32_t*)(UART0_BASE + 0x18))
#define UART_FR_TXFF (1u<<5)
static void putc(char c){ while (UART0_FR & UART_FR_TXFF){} UART0_DR = (uint32_t)c; }
static void puts(const char*s){ while(*s) putc(*s++); }
int main(void){
    puts("[APP] Hello from user program!\r\n");
    for(;;){}
}
