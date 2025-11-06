#include "include/systick.h"
#include "include/hal_console.h" // For debugging
#include "include/proc.h" // For process management

// Cortex-A9 Private Timer Register Definitions
#define CORTEX_A9_TIMER_BASE 0xF8F00600
#define TIMER_LOAD   (*(volatile uint32_t *)(CORTEX_A9_TIMER_BASE + 0x00))
#define TIMER_COUNT  (*(volatile uint32_t *)(CORTEX_A9_TIMER_BASE + 0x04))
#define TIMER_CTRL   (*(volatile uint32_t *)(CORTEX_A9_TIMER_BASE + 0x08))
#define TIMER_ISR    (*(volatile uint32_t *)(CORTEX_A9_TIMER_BASE + 0x0C))

// Timer Control Register Flags
#define TIMER_CTRL_ENABLE      (1 << 0)
#define TIMER_CTRL_AUTO_RELOAD (1 << 1)
#define TIMER_CTRL_IRQ_ENABLE  (1 << 2)

#define SYSTEM_CLOCK_HZ 100000000

volatile uint32_t tick_count = 0;

extern pcb_t pcb_table[MAX_PROCESSES];

void systick_init(uint32_t frequency) {
    // Disable timer during setup
    TIMER_CTRL = 0;

    // Calculate reload value
    uint32_t reload_value = (SYSTEM_CLOCK_HZ / frequency) - 1;

    // Set reload value
    TIMER_LOAD = reload_value;

    // Enable timer with auto-reload and interrupt
    TIMER_CTRL = TIMER_CTRL_ENABLE | TIMER_CTRL_AUTO_RELOAD | TIMER_CTRL_IRQ_ENABLE;

    hal_console_puts("Cortex-A9 Private Timer initialized for ");
    hal_console_put_int(frequency);
    hal_console_puts(" Hz.\r\n");
}

void SysTick_Handler(void) {
    tick_count++;

    // Clear the interrupt flag
    TIMER_ISR = 1;

    // Update sleep timers for all processes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pcb_table[i].state == PROC_STATE_SLEEPING) {
            if (pcb_table[i].sleep_ticks > 0) {
                pcb_table[i].sleep_ticks--;
            } else {
                pcb_table[i].state = PROC_STATE_READY;
            }
        }
    }


}