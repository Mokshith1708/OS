#include <stdint.h>
#include "proc.h"
#include "hal/hal_console.h"

/* App RAM window (provided by linker) */
extern uint32_t __app_ram_start__[];
extern uint32_t __app_ram_end__[];

#define APP_BASE ((uintptr_t)__app_ram_start__)
#define APP_END  ((uintptr_t)__app_ram_end__)
#define APP_SIZE (APP_END - APP_BASE)


void start_process(const char *path) {
    uint32_t entry = 0, sp = (uint32_t)APP_END;

    hal_console_puts("Loading...\r\n");

    int rc = swap_in(path, &entry, &sp);
    hal_console_puts("hirc\r\n");
    if (rc < 0) {
        hal_console_puts("Load failed.\r\n");
        return;
    }
    hal_console_puts("swap_in OK\r\n");

    /* Read vector table from the APP window */
    uint32_t vt0 = ((uint32_t*)APP_BASE)[0];
    uint32_t vt1 = ((uint32_t*)APP_BASE)[1];

    hal_console_puts("VT0="); hal_console_put_hex(vt0); hal_console_puts("\r\n");
    hal_console_puts("VT1="); hal_console_put_hex(vt1); hal_console_puts("\r\n");

    if (entry == 0) entry = vt1;
    if (sp == (uint32_t)APP_END) sp = vt0 ? vt0 : (uint32_t)APP_END;

    uint32_t pc_addr = entry & ~1u;

    if (sp < APP_BASE || sp > APP_END) { hal_console_puts("Bad SP (OOB)\r\n"); return; }
    if (pc_addr < APP_BASE || pc_addr >= APP_END) { hal_console_puts("Bad PC (OOB)\r\n"); return; }

    hal_console_puts("About to jump:\r\n");
    hal_console_puts(" SP="); hal_console_put_hex(sp); hal_console_puts("\r\n");
    hal_console_puts(" PC="); hal_console_put_hex(entry); hal_console_puts("\r\n");
    __asm__ volatile (
        "msr msp, %0\n"
        "isb\n"
        "bx %1\n"
        :
        : "r"(sp), "r"(entry)
        : "memory"
    );

    hal_console_puts("app returned to kernel\r\n");
}
