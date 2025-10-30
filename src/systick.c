#include "include/systick.h"
#include "include/hal_console.h" // For debugging

// SysTick Register Definitions (Cortex-M0)
#define SYSTICK_CTRL   (*(volatile uint32_t *)0xE000E010)
#define SYSTICK_LOAD   (*(volatile uint32_t *)0xE000E014)
#define SYSTICK_VAL    (*(volatile uint32_t *)0xE000E018)

// SysTick Control Register Flags
#define SYSTICK_CTRL_ENABLE    (1 << 0)
#define SYSTICK_CTRL_TICKINT   (1 << 1)
#define SYSTICK_CTRL_CLKSOURCE (1 << 2) // 1 = Processor clock

// This should be configured for the specific target hardware's clock speed.
// A common value for FPGA projects is 100MHz.
#define SYSTEM_CLOCK_HZ 100000000

#define ICSR (*(volatile uint32_t *)0xE000ED04)
#define PENDSVSET (1 << 28)

static volatile uint32_t tick_count = 0;

void systick_init(uint32_t frequency) {
    // Disable SysTick during setup
    SYSTICK_CTRL = 0;

    // Calculate reload value. The timer is a 24-bit down-counter.
    uint32_t reload_value = (SYSTEM_CLOCK_HZ / frequency) - 1;

    // Set reload and clear the current value
    SYSTICK_LOAD = reload_value;
    SYSTICK_VAL = 0;

    // Enable SysTick with processor clock and interrupts
    SYSTICK_CTRL = SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_CLKSOURCE;

    hal_console_puts("SysTick initialized for ");
    hal_console_put_int(frequency);
    hal_console_puts(" Hz.\r\n");
}

/**
 * @brief This is the SysTick interrupt handler.
 * The function name is fixed and is referenced by the vector table in boot.s
 */
void SysTick_Handler(void) {
    // This is the heartbeat of the OS. On each tick, we request the
    // PendSV exception, which has a lower priority. The PendSV handler
    // will perform the actual context switch when it is safe to do so.
    ICSR = PENDSVSET;
}
