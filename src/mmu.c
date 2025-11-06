#include "include/mmu.h"
#include <stdint.h>

/*
 * mmu.c
 *
 * Memory Management Unit configuration for Cortex-A9.
 */

// 16KB page tables
__attribute__((aligned(16384))) static uint32_t page_table[4096];

void mmu_init(void) {
    // Create a 1-to-1 mapping for the entire 4GB address space
    for (uint32_t i = 0; i < 4096; i++) {
        page_table[i] = (i << 20) | (1 << 10) | 2;
    }

    // Set the page table base address
    uint32_t ttb_base = (uint32_t)page_table;
    asm volatile("mcr p15, 0, %0, c2, c0, 0" : : "r"(ttb_base));

    // Enable the MMU
    uint32_t sctlr;
    asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(sctlr));
    sctlr |= 1;
    asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(sctlr));
}

void mpu_config_region(uint32_t base, uint32_t size, uint32_t attributes) {
    // This function is a stub for now.
    // In a real implementation, this would modify the page table entries.
    (void)base;
    (void)size;
    (void)attributes;
}