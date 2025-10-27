#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

typedef void (*irq_handler_t)(void);

/* Register an interrupt handler in NVIC table */
void irq_register(uint32_t irq_num, irq_handler_t handler);

/* Enable interrupt in NVIC */
void irq_enable(uint32_t irq_num);

/* Disable interrupt in NVIC */
void irq_disable(uint32_t irq_num);

/* Common handler for unhandled interrupts */
void irq_default_handler(void);

#endif
