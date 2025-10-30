#include "include/mmu.h"
#include <stdint.h>

/*
 * mmu.c
 *
 * Memory Management Unit / Memory Protection Unit configuration.
 *
 * The standard Cortex-M0 processor does not include an MPU.
 * This file's functions are stubs for API compatibility. They perform
 * no operations, ensuring correct linkage without functional impact.
 * This allows for future development on processors that do support
 * an MPU (e.g., Cortex-M0+ with MPU option).
 */

void mpu_init(void) {
    // No-op for Cortex-M0
}

void mpu_config_region(uint32_t base, uint32_t size, uint32_t attributes) {
    // No-op for Cortex-M0
    (void)base;
    (void)size;
    (void)attributes;
}
