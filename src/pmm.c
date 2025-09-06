#include "pmm.h"

// These are not variables; they are addresses/values from the linker script.
// 'extern' tells the C compiler "trust me, these exist somewhere else".
extern char _USER_SPACE_START[];
extern char _USER_SPACE_SIZE[];

void pmm_init(void) {
    // Nothing to do for now.
}

uintptr_t pmm_get_user_space_base(void) {
    // The "address of the symbol" is the value we want.
    return (uintptr_t)_USER_SPACE_START;
}

size_t pmm_get_user_space_size(void) {
    // Same concept here.
    return (size_t)_USER_SPACE_SIZE;
}