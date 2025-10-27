#include "mmu.h"

/* MPU Registers for Cortex-M3 */
#define MPU_TYPE   (*(volatile uint32_t *)0xE000ED90)
#define MPU_CTRL   (*(volatile uint32_t *)0xE000ED94)
#define MPU_RNR    (*(volatile uint32_t *)0xE000ED98)
#define MPU_RBAR   (*(volatile uint32_t *)0xE000ED9C)
#define MPU_RASR   (*(volatile uint32_t *)0xE000EDA0)

#define MPU_ENABLE 0x1
#define MPU_REGION_ENABLE 0x1
#define MPU_AP_RW 0x3   // Full access
#define MPU_AP_RO 0x6   // Read-only

void mpu_init(void) {
    MPU_CTRL = 0;          // Disable MPU
    MPU_TYPE;               // Dummy read
}

void mpu_config_region(uint32_t base, uint32_t size, uint32_t attributes) {
    static uint32_t region = 0;
    MPU_RNR = region++;
    MPU_RBAR = base;
    MPU_RASR = (attributes & 0xFFF) | MPU_REGION_ENABLE;
}
