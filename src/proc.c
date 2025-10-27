#include <stdint.h>
#include <stddef.h>
#include "include/proc.h"
#include "include/hal_console.h"

extern uint32_t __app_ram_start__[];
extern uint32_t __app_ram_end__[];

#define APP_BASE ((uintptr_t)__app_ram_start__)
#define APP_END  ((uintptr_t)__app_ram_end__)
#define APP_SIZE (APP_END - APP_BASE)

#define CORTEX_M3_VTOR (*(volatile uint32_t*)0xE000ED08)

static void hexdump(const void *data, size_t size) {
    const uint8_t *p = (const uint8_t*)data;
    for (size_t i = 0; i<size; i+=16) {
        hal_console_put_hex((uint32_t)(p+i)); hal_console_puts(": ");
        for(size_t j=0;j<16;j++){
            if(i+j<size){
                uint8_t b=p[i+j]; if(b<16) hal_console_putc('0');
                hal_console_put_hex(b); hal_console_putc(' ');
            } else hal_console_puts("   ");
        }
        hal_console_puts("\r\n");
    }
}

void start_process(const char *path) {
    uint32_t entry = 0;
    uint32_t sp = 0;

    hal_console_puts("Loading...\r\n");

    int rc = swap_in(path, &entry, &sp);
    if (rc < 0) {
        hal_console_puts("Load failed.\r\n");
        return;
    }

    if (sp == 0) sp = (uint32_t)APP_END; // Ensure SP is valid

    hal_console_puts("swap_in OK\r\n");

    // --- MEMORY INSPECTION ---
    hal_console_puts("\r\n--- MEMORY INSPECTION PRE-JUMP ---\r\n");
    hal_console_puts("Base Address: "); hal_console_put_hex(APP_BASE); hal_console_puts("\r\n");
    hal_console_puts("Size:         "); hal_console_put_int(APP_SIZE); hal_console_puts(" bytes\r\n\r\n");

    // --- VALIDATION ---
    uint32_t pc_addr = entry & ~1u;  // Ensure halfword alignment
    if (sp <= APP_BASE || sp > APP_END) { hal_console_puts("Bad SP (OOB)\r\n"); return; }
    if (pc_addr < APP_BASE || pc_addr >= APP_END) { hal_console_puts("Bad PC (OOB)\r\n"); return; }

    // --- SET VTOR (optional, keep for vector correctness) ---
    CORTEX_M3_VTOR = APP_BASE;

    hal_console_puts("About to jump:\r\n");
    hal_console_puts(" SP="); hal_console_put_hex(sp); hal_console_puts("\r\n");
    hal_console_puts(" PC="); hal_console_put_hex(entry); hal_console_puts("\r\n");
    hal_console_puts(" VTOR="); hal_console_put_hex(APP_BASE); hal_console_puts("\r\n");

    // --- Minimal trampoline: just set SP and jump ---
    __asm__ volatile (
        "mov sp, %0\n"   // set stack pointer
        "bx %1\n"        // jump to entry point
        :
        : "r"(sp), "r"(entry)
        : "memory"
    );

    // will only reach here if app returns
    hal_console_puts("app returned to kernel\r\n");
}
