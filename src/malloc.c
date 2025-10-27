#include "malloc.h"
#include "pmm.h"

static uint8_t *heap_start;
static uint8_t *heap_end;
static uint8_t *heap_ptr;

void malloc_init(void) {
    heap_start = (uint8_t *)__kheap_start;
    heap_end   = (uint8_t *)__kheap_end;
    heap_ptr   = heap_start;
}

void *malloc(size_t size) {
    if (heap_ptr + size > heap_end) return 0;
    void *ptr = heap_ptr;
    heap_ptr += size;
    return ptr;
}

void free(void *ptr) {
    (void)ptr; // no-op
}
