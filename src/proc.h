#pragma once
#include <stdint.h>

typedef struct {
    uint32_t ram_size;
    uint32_t entry_pc;
    uint32_t initial_sp;
    uint32_t flags;
} proc_img_hdr_t;

int  swap_in (const char *path, uint32_t *entry, uint32_t *sp);
int  swap_out(const char *path);
void start_process(const char *path);
