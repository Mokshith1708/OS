#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include <stdint.h>

void malloc_init(void);
void *malloc(size_t size);
void free(void *ptr);

#endif
