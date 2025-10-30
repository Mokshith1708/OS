#pragma once
#include <stdint.h>

/**
 * @brief Initializes the SysTick timer to generate interrupts at a given frequency.
 * 
 * @param frequency The desired interrupt frequency in Hz.
 */
void systick_init(uint32_t frequency);
