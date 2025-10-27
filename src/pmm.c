// #include "pmm.h"

// // These are not variables; they are addresses/values from the linker script.
// // 'extern' tells the C compiler "trust me, these exist somewhere else".
// extern char _USER_SPACE_START[];
// extern char _USER_SPACE_SIZE[];

// void pmm_init(void) {
//     // Nothing to do for now.
// }

// uintptr_t pmm_get_user_space_base(void) {
//     // The "address of the symbol" is the value we want.
//     return (uintptr_t)_USER_SPACE_START;
// }

// size_t pmm_get_user_space_size(void) {
//     // Same concept here.
//     return (size_t)_USER_SPACE_SIZE;
// }

#include "include/pmm.h"
// #include "linker.h"
#include <stdint.h>
#include <stddef.h>

extern char __kheap_start[];
extern char __kheap_end[];
extern char __ustack_top[];
extern char __ustack_bottom[];


static uint8_t *kernel_heap;
static size_t kernel_heap_size;
static size_t kernel_heap_used;

void pmm_init(void) {
    kernel_heap = (uint8_t *)__kheap_start;
    kernel_heap_size = __kheap_end - __kheap_start;
    kernel_heap_used = 0;
}

void *pmm_alloc(size_t size) {
    if (kernel_heap_used + size > kernel_heap_size) return 0;
    void *ptr = kernel_heap + kernel_heap_used;
    kernel_heap_used += size;
    return ptr;
}

void pmm_free(void *ptr) {
    (void)ptr; // naive allocator: cannot free
}

uintptr_t pmm_get_user_space_base(void) {
    return __uheap_start;
}

size_t pmm_get_user_space_size(void) {
    return __uheap_end - __uheap_start;
}
