#ifndef MMU_H
#define MMU_H

#include <stdint.h>

void mpu_init(void);
void mpu_config_region(uint32_t base, uint32_t size, uint32_t attributes);

#endif
