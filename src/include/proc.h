#pragma once
#include <stdint.h>

typedef struct {
    uint32_t ram_size;
    uint32_t entry_pc;
    uint32_t initial_sp;
    uint32_t flags;
} proc_img_hdr_t;

#define MAX_PROCESSES 8

typedef enum {
    PROC_STATE_UNUSED,
    PROC_STATE_READY,
    PROC_STATE_RUNNING,
    PROC_STATE_BLOCKED,
    PROC_STATE_SLEEPING,
} proc_state_t;

typedef struct pcb {
    uint32_t sp;            // Stack Pointer (must be first element for context switching)
    proc_state_t state;     // Process state
    uint32_t pid;           // Process ID
    struct pcb *next;       // Pointer for linked list scheduling
} pcb_t;


void proc_init(void);
void scheduler_start(void);
void schedule(void);
int  swap_in (const char *path, uint32_t *entry, uint32_t *sp);
int  swap_out(const char *path);
void start_process(const char *path);
