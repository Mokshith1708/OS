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
    hal_console_puts("Dumping entire defined User Space Memory:\r\n");
    hal_console_puts("Base Address: "); hal_console_put_hex(APP_BASE); hal_console_puts("\r\n");
    hal_console_puts("Size:         "); hal_console_put_int(APP_SIZE); hal_console_puts(" bytes\r\n\r\n");
    
    // This is the new part: dump the whole region
    // hexdump((void*)APP_BASE, APP_SIZE);
    
    // hal_console_puts("------------------------------------\r\n\r\n");

    // --- VALIDATION ---
    uint32_t pc_addr = entry & ~1u;
    if (sp <= APP_BASE || sp > APP_END) { hal_console_puts("Bad SP (OOB)\r\n"); return; }
    if (pc_addr < APP_BASE || pc_addr >= APP_END) { hal_console_puts("Bad PC (OOB)\r\n"); return; }


    // --- SET THE VECTOR TABLE AND JUMP ---
    CORTEX_M3_VTOR = APP_BASE;
    uint32_t final_entry = entry;

    hal_console_puts("About to jump:\r\n");
    hal_console_puts(" SP="); hal_console_put_hex(sp); hal_console_puts("\r\n");
    hal_console_puts(" PC="); hal_console_put_hex(final_entry); hal_console_puts("\r\n");
    hal_console_puts(" VTOR="); hal_console_put_hex(APP_BASE); hal_console_puts("\r\n");

    __asm__ volatile (
        "cpsid if\n"            // Disable IRQ and FIQ interrupts
        "mov sp, %0\n"          // Set stack pointer to the value in %0
        "isb\n"                 // Instruction Synchronization Barrier
        "bx %1\n"               // Branch to final_entry (address in %1)
        : : "r"(sp), "r"(final_entry) : "memory"
    );

    hal_console_puts("app returned to kernel\r\n");

}
