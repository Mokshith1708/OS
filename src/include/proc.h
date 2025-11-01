#pragma once
#include "include/fs.h"

typedef enum {
    PROC_STATE_UNUSED = 0,
    PROC_STATE_READY,
    PROC_STATE_RUNNING,
    PROC_STATE_BLOCKED,
    PROC_STATE_SLEEPING,
    PROC_STATE_KILLED
} proc_state_t;

typedef struct {
    uint32_t ram_size;
    uint32_t entry_pc;
    uint32_t initial_sp;
    uint32_t flags;
} proc_img_hdr_t;

#define MAX_PROCESSES 8
#define MAX_OPEN_FILES 8 // Maximum number of open files per process
#define MAX_MESSAGES 4   // Maximum number of messages in a process's queue

// Process Control Block (PCB)
typedef struct {
    uint32_t sp;                // Stack pointer
    uint32_t pid;               // Process ID
    proc_state_t state;         // Current state of the process
    uint32_t heap_brk;          // Current break of the heap
    uint32_t sleep_ticks;       // Remaining ticks to sleep
    file_t* fd_table[MAX_OPEN_FILES]; // File descriptor table
    inode_t* cwd_inode;         // Current working directory inode
    // Message queue for IPC
    char* msg_queue[MAX_MESSAGES]; // Array of message pointers
    uint32_t msg_count;            // Number of messages currently in queue
    uint32_t msg_read_idx;         // Read index for message queue
    uint32_t msg_write_idx;        // Write index for message queue
} pcb_t;

// Function prototypes
void proc_init(void);
void start_process(const char *path);
void scheduler_start(void);
void schedule(void);
void* _sbrk(intptr_t increment);
int  swap_in (inode_t *inode, uint32_t *entry, uint32_t *sp, uint32_t *img_size);
int  swap_out(const char *path);
void start_process(const char *path);
int sys_exec(const char *path, int argc, char *const argv[]);
