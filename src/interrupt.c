#include <stdint.h>
#include "include/proc.h"
#include "include/hal_console.h"

// GIC Register Definitions
#define GIC_DIST_BASE 0xF8F01000
#define GIC_CPU_BASE  0xF8F00100

#define GIC_DIST_CTRL      (*(volatile uint32_t *)(GIC_DIST_BASE + 0x000))
#define GIC_DIST_ISENABLER ((volatile uint32_t *)(GIC_DIST_BASE + 0x100))
#define GIC_DIST_ICENABLER ((volatile uint32_t *)(GIC_DIST_BASE + 0x180))
#define GIC_DIST_IPRIORITYR ((volatile uint32_t *)(GIC_DIST_BASE + 0x400))
#define GIC_DIST_ITARGETSR ((volatile uint32_t *)(GIC_DIST_BASE + 0x800))

#define GIC_CPU_CTRL       (*(volatile uint32_t *)(GIC_CPU_BASE + 0x00))
#define GIC_CPU_PMR        (*(volatile uint32_t *)(GIC_CPU_BASE + 0x04))
#define GIC_CPU_IAR        (*(volatile uint32_t *)(GIC_CPU_BASE + 0x0C))
#define GIC_CPU_EOIR       (*(volatile uint32_t *)(GIC_CPU_BASE + 0x10))

void SysTick_Handler(void);

void gic_init(void) {
    // Disable the GIC distributor
    GIC_DIST_CTRL = 0;

    // Set all interrupts to be level-sensitive
    for (int i = 0; i < 32; i++) {
        GIC_DIST_ICENABLER[i] = 0xFFFFFFFF;
    }

    // Set all interrupts to the lowest priority
    for (int i = 0; i < 255; i++) {
        GIC_DIST_IPRIORITYR[i] = 0xFFFFFFFF;
    }

    // Route all interrupts to CPU0
    for (int i = 0; i < 255; i++) {
        GIC_DIST_ITARGETSR[i] = 0x01010101;
    }

    // Enable all interrupts
    for (int i = 0; i < 32; i++) {
        GIC_DIST_ISENABLER[i] = 0xFFFFFFFF;
    }

    // Enable the GIC distributor
    GIC_DIST_CTRL = 1;

    // Configure the CPU interface
    // Set the priority mask to allow all priorities
    GIC_CPU_PMR = 0xFF;

    // Enable the CPU interface
    GIC_CPU_CTRL = 1;
}

void irq_handler(void) {
    // Read the interrupt ID
    uint32_t irq = GIC_CPU_IAR;

    // Handle the interrupt
    switch (irq) {
        case 29: // Private timer interrupt
            SysTick_Handler();
            break;
        default:
            hal_console_puts("Unknown interrupt: ");
            hal_console_put_int(irq);
            hal_console_puts("\n");
            break;
    }

    // End of interrupt
    GIC_CPU_EOIR = irq;
}