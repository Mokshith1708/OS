#ifndef KMALLOC_H
#define KMALLOC_H

#include <stddef.h>
#include <stdint.h> // For intptr_t

void* kmalloc(size_t size);
void kfree(void* ptr);
void *_sbrk(intptr_t incr);

#endif // KMALLOC_H
