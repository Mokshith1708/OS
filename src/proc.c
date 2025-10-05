#include <stdint.h>
#include <stddef.h> // For size_t
#include "proc.h"
#include "hal/hal_console.h"

/* App RAM window (provided by linker) */
extern uint32_t __app_ram_start__[];
extern uint32_t __app_ram_end__[];

#define APP_BASE ((uintptr_t)__app_ram_start__)
#define APP_END  ((uintptr_t)__app_ram_end__)
#define APP_SIZE (APP_END - APP_BASE)

// Address of the Cortex-M3 Vector Table Offset Register (VTOR)
#define CORTEX_M3_VTOR (*(volatile uint32_t*)0xE000ED08)

/**
 * @brief Prints a region of memory as a hexdump.
 * @param data Pointer to the memory to dump.
 * @param size Number of bytes to dump.
 */
static void hexdump(const void *data, size_t size) {
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < size; i += 16) {
        // Print address
        hal_console_put_hex((uint32_t)(p + i));
        hal_console_puts(": ");

        // Print hex bytes
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                uint8_t byte = p[i + j];
                // Print a leading zero for single-digit hex values
                if (byte < 16) hal_console_putc('0');
                hal_console_put_hex(byte);
                hal_console_putc(' ');
            } else {
                hal_console_puts("   ");
            }
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

    // --- SET VTOR ---
    CORTEX_M3_VTOR = APP_BASE;
    uint32_t final_entry = entry | 1; // Thumb mode

    hal_console_puts("About to jump:\r\n");
    hal_console_puts(" SP="); hal_console_put_hex(sp); hal_console_puts("\r\n");
    hal_console_puts(" PC="); hal_console_put_hex(final_entry); hal_console_puts("\r\n");
    hal_console_puts(" VTOR="); hal_console_put_hex(APP_BASE); hal_console_puts("\r\n");

    // --- Setup exception stack frame and start user app ---
    __asm__ volatile (
        "mov r0, %0\n"            // r0 = user SP (top of RAM)
        "sub r0, r0, #32\n"       // reserve space for exception frame
        "mov r1, #0\n"
        "str r1, [r0, #0]\n"      // R0
        "str r1, [r0, #4]\n"      // R1
        "str r1, [r0, #8]\n"      // R2
        "str r1, [r0, #12]\n"     // R3
        "str r1, [r0, #16]\n"     // R12
        "str r1, [r0, #20]\n"     // LR

        "ldr r2, %1\n"            // load PC
        "str r2, [r0, #24]\n"     // PC
        "ldr r2, =0x01000000\n"   // xPSR (Thumb bit)
        "str r2, [r0, #28]\n"

        "msr psp, r0\n"           // set process stack pointer
        "mov r0, #2\n"
        "msr control, r0\n"       // switch to PSP, thread mode
        "isb\n"

        "ldr r0, =0xFFFFFFFD\n"   // EXC_RETURN to thread mode using PSP
        "bx r0\n"
        :
        : "r"(sp), "m"(final_entry)
        : "r0","r1","r2","memory"
    );

    hal_console_puts("app returned to kernel\r\n");
}
