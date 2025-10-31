#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>

void *malloc(uint32_t size);
void free(void *ptr);

#endif