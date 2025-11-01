#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "include/proc.h"
#include "include/hal_console.h"
#include "include/fs.h"

extern void proc_switch_to_user(uint32_t user_sp);

// --- Scheduler Globals ---
pcb_t pcb_table[MAX_PROCESSES];
pcb_t *current_process = NULL;


void proc_init(void) {
    inode_t* root_inode = fs_lookup("/"); // Get root inode

    for (int i = 0; i < MAX_PROCESSES; i++) {
        pcb_table[i].state = PROC_STATE_UNUSED;
        pcb_table[i].cwd_inode = root_inode;
        for (int j = 0; j < MAX_OPEN_FILES; j++) {
            pcb_table[i].fd_table[j] = NULL;
        }
        // Initialize message queue
        pcb_table[i].msg_count = 0;
        pcb_table[i].msg_read_idx = 0;
        pcb_table[i].msg_write_idx = 0;
        for (int j = 0; j < MAX_MESSAGES; j++) {
            pcb_table[i].msg_queue[j] = NULL;
        }    }
}


extern uint32_t __app_ram_start__[];
extern uint32_t __app_ram_end__[];

#define APP_BASE ((uintptr_t)__app_ram_start__)
#define APP_END  ((uintptr_t)__app_ram_end__)
#define APP_SIZE (APP_END - APP_BASE)

static pcb_t* proc_create(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pcb_table[i].state == PROC_STATE_UNUSED) {
            pcb_table[i].pid = i + 1; // PID 0 is reserved
            pcb_table[i].state = PROC_STATE_READY;
            return &pcb_table[i];
        }
    }
    return NULL; // No free PCBs
}

// This is the core function for loading and starting a process.
// It is used by both start_process (for the first process) and the exec syscall.
int sys_exec(const char *path, int argc, char *const argv[]) {
    pcb_t *pcb;

    if (current_process && current_process->state == PROC_STATE_RUNNING) {
        // This is an exec syscall replacing the current process
        pcb = current_process;
    } else {
        // This is a new process being started by the kernel
        pcb = proc_create();
        if (pcb == NULL) {
            hal_console_puts("Failed to create process: no free PCBs.\r\n");
            return -1;
        }
    }

    uint32_t entry = 0;
    uint32_t initial_sp_from_file = 0;
    uint32_t img_size = 0;

    inode_t* inode = fs_find_file(path);
    if (inode == NULL) {
        hal_console_puts("exec: File not found.\r\n");
        return -1;
    }

    int rc = swap_in(inode, &entry, &initial_sp_from_file, &img_size);
    if (rc < 0) {
        hal_console_puts("exec: Load failed.\r\n");
        if (pcb != current_process) pcb->state = PROC_STATE_UNUSED; // Clean up
        return -1;
    }

    // Initialize the heap break. The heap starts right after the app's loaded image.
    pcb->heap_brk = (APP_BASE + img_size + 7) & ~7u; // 8-byte aligned

    // --- Argument and Stack Setup ---
    uint32_t sp = (uint32_t)APP_END;

    // 1. Calculate total size of argument strings
    size_t total_str_len = 0;
    for (int i = 0; i < argc; i++) {
        total_str_len += strlen(argv[i]) + 1; // +1 for null terminator
    }

    // 2. Copy strings to the top of the stack
    sp -= total_str_len;
    char *str_area = (char *)sp;
    char *current_str = str_area;
    for (int i = 0; i < argc; i++) {
        strcpy(current_str, argv[i]);
        current_str += strlen(argv[i]) + 1;
    }

    // 3. Align stack (must be 4-byte aligned)
    sp &= ~3u;

    // 4. Create the argv pointer array on the stack
    sp -= (argc + 1) * sizeof(char*); // +1 for null terminator
    char **new_argv = (char **)sp;

    char *str_ptr = str_area;
    for (int i = 0; i < argc; i++) {
        new_argv[i] = str_ptr;
        str_ptr += strlen(str_ptr) + 1;
    }
    new_argv[argc] = NULL; // Null-terminate the argv array

    // 5. Final stack alignment for ARM ABI (8-byte)
    sp &= ~7u;

    // --- Create the fake exception frame ---
    uint32_t *user_stack = (uint32_t *)sp;

    *--user_stack = 0x01000000u; // xPSR: Set T-bit for Thumb mode
    *--user_stack = entry;       // PC: The process entry point
    *--user_stack = 0;           // LR (return from main)
    *--user_stack = 0;           // R12
    *--user_stack = 0;           // R3
    *--user_stack = 0;           // R2
    *--user_stack = (uint32_t)new_argv; // R1: argv
    *--user_stack = argc;        // R0: argc

    pcb->sp = (uint32_t)user_stack;
    pcb->state = PROC_STATE_READY;

    hal_console_puts("Process created/exec'd with PID ");
    hal_console_put_int(pcb->pid);
    hal_console_puts(", state=READY\r\n");

    return 0; // Success
}

void start_process(const char *path) {
    // For the first process, we have no arguments.
    char *const argv[] = {(char*)path, NULL};
    int argc = 1;
    sys_exec(path, argc, argv);
}

void scheduler_start(void) {
    pcb_t *target = NULL;
    // Find a ready process (simple linear scan)
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pcb_table[i].state == PROC_STATE_READY) {
            target = &pcb_table[i];
            break;
        }
    }

    if (target == NULL) {
        hal_console_puts("FATAL: scheduler_start called with no ready processes! Halting.\r\n");
        while(1);
    }

    current_process = target;
    current_process->state = PROC_STATE_RUNNING;

    hal_console_puts("Scheduler: Starting PID ");
    hal_console_put_int(current_process->pid);
    hal_console_puts("\r\n");

    proc_switch_to_user(current_process->sp);

    // This point should be unreachable
    hal_console_puts("FATAL: Returned from scheduler_start!\r\n");
}

/**
 * @brief C-level scheduler, called from PendSV_Handler to choose the next process.
 * 
 * Implements a simple round-robin scheduling policy.
 */
void schedule(void) {
    int current_pid_idx = -1;
    if (current_process != NULL) {
        current_pid_idx = current_process->pid - 1;
        // If the process wasn't blocked, set it to ready
        if (current_process->state == PROC_STATE_RUNNING) {
            current_process->state = PROC_STATE_READY;
        }
    }

    // Find the next ready process
    int next_pid_idx = (current_pid_idx == -1) ? (MAX_PROCESSES - 1) : current_pid_idx;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        next_pid_idx = (next_pid_idx + 1) % MAX_PROCESSES;
        if (pcb_table[next_pid_idx].state == PROC_STATE_READY) {
            current_process = &pcb_table[next_pid_idx];
            current_process->state = PROC_STATE_RUNNING;
            return;
        }
    }

    // If no other process is ready, and the original process is still ready,
    // just continue running it.
    if (current_pid_idx != -1 && pcb_table[current_pid_idx].state == PROC_STATE_READY) {
        current_process = &pcb_table[current_pid_idx];
        current_process->state = PROC_STATE_RUNNING;
        return;
    }
    
    // If we get here, there are no ready processes to switch to.
    // The context switch will just restore the same context if a process is running.
    // If not, a real OS would switch to an idle task.
}

void* _sbrk(intptr_t increment) {
    if (!current_process) {
        return (void*)-1; // No process running
    }

    uint32_t old_brk = current_process->heap_brk;
    uint32_t new_brk = old_brk + increment;

    // Check for collision with the stack. The stack grows down from APP_END.
    // We must ensure the new break does not collide with the current stack pointer.
    if (new_brk >= current_process->sp) {
        return (void*)-1; // Heap collision
    }

    // The break must also stay within the application's memory region.
    if (new_brk >= (uint32_t)APP_END) {
        return (void*)-1; // Out of memory
    }

    current_process->heap_brk = new_brk;
    return (void*)old_brk;
}
