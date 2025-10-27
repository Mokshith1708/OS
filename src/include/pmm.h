// #ifndef PMM_H
// #define PMM_H

// #include <stdint.h> // For uintptr_t
// #include <stddef.h> // For size_t

// void pmm_init(void);
// uintptr_t pmm_get_user_space_base(void);
// size_t pmm_get_user_space_size(void);

// #endif // PMM_H

#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

void pmm_init(void);

void *pmm_alloc(size_t size);
void pmm_free(void *ptr);

uintptr_t pmm_get_user_space_base(void);
size_t pmm_get_user_space_size(void);

#endif
