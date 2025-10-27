#include "interrupt.h"
#include "hal/hal_console.h"

#define NVIC_ISER ((volatile uint32_t *)0xE000E100)
#define NVIC_ICER ((volatile uint32_t *)0xE000E180)

#define IRQ_MAX 32

static irq_handler_t irq_table[IRQ_MAX];

void irq_register(uint32_t irq_num, irq_handler_t handler) {
    if (irq_num >= IRQ_MAX) return;
    irq_table[irq_num] = handler;
}

void irq_enable(uint32_t irq_num) {
    if (irq_num >= IRQ_MAX) return;
    NVIC_ISER[irq_num >> 5] = (1 << (irq_num & 0x1F));
}

void irq_disable(uint32_t irq_num) {
    if (irq_num >= IRQ_MAX) return;
    NVIC_ICER[irq_num >> 5] = (1 << (irq_num & 0x1F));
}

void irq_default_handler(void) {
    hal_console_puts("[IRQ] Unhandled interrupt!\r\n");
    while (1);
}
