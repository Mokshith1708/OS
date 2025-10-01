/* hal_console.c - Semihosting console for MPS2-QEMU */
#include "hal_console.h"
#include "../semihost.h"
#include "../drivers/driver.h"
#include <stdint.h>
#include <stdlib.h>
#include "xuartps.h"
XUartPs Uart_PS;

volatile char *keyboard=(volatile unsigned int*)0x1180AA;

void hal_console_init(void) {
	int status;
	XUartPs_Config *Config;

	    // Lookup the configuration for UART device by device ID (usually XPAR_XUARTPS_0_DEVICE_ID)
	    Config = XUartPs_LookupConfig(XPAR_XUARTPS_0_DEVICE_ID);
	    if (Config == NULL) {
	        return XST_FAILURE;
	    }

	    // Initialize the UART driver
	    status = XUartPs_CfgInitialize(&Uart_PS, Config, Config->BaseAddress);
	    if (status != XST_SUCCESS) {
	        return status;
	    }

	    // Optional: Set UART options like baud rate if needed
	     XUartPs_SetBaudRate(&Uart_PS, 115200);

	    return XST_SUCCESS;
}


/* Send a single character */
void hal_console_putc(char c) {
    xil_printf("%c",c);
    console_putc(c);
}

/* Send a null-terminated string */
void hal_console_puts(const char *s) {
    xil_printf("%s",s);
    console_puts(s);
}

/* Print an integer in decimal */
void hal_console_put_int(int n) {
    xil_printf("%d",n);
    console_put_int(n);
}

/* Print an unsigned integer in hexadecimal */
void hal_console_put_hex(uint32_t n) {
    xil_printf("%x",n);
    console_put_hex(n);
}

/* Read a single character (blocking) */
int hal_console_getchar(void) {
	while(!*keyboard){}
	char t = *keyboard;
	*keyboard = 0;
    return (int)t;
}
