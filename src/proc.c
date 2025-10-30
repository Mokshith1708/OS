#include <stdint.h>
#include <stddef.h>
#include "include/proc.h"
#include "include/hal_console.h"

extern void proc_switch_to_user(uint32_t user_sp);

// --- Scheduler Globals ---
static pcb_t pcb_table[MAX_PROCESSES];
pcb_t *current_process = NULL;


void proc_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        pcb_table[i].state = PROC_STATE_UNUSED;
    }
}


extern uint32_t __app_ram_start__[];
extern uint32_t __app_ram_end__[];

#define APP_BASE ((uintptr_t)__app_ram_start__)
#define APP_END  ((uintptr_t)__app_ram_end__)
#define APP_SIZE (APP_END - APP_BASE)

static void hexdump(const void *data, size_t size) {
    const uint8_t *p = (const uint8_t*)data;
    for (size_t i = 0; i<size; i+=16) {
        hal_console_put_hex((uint32_t)(p+i)); hal_console_puts(": ");
        for(size_t j=0;j<16;j++){
            if(i+j<size){
                uint8_t b=p[i+j]; if(b<16) hal_console_putc('0');
                hal_console_put_hex(b); hal_console_putc(' ');
            } else hal_console_puts("   ");
        }
        hal_console_puts("\r\n");
    }
}

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

void start_process(const char *path) {
    pcb_t *pcb = proc_create();
    if (pcb == NULL) {
        hal_console_puts("Failed to create process: no free PCBs.\r\n");
        return;
    }

    uint32_t entry = 0;
    uint32_t sp = 0;

    hal_console_puts("Loading...\r\n");

    int rc = swap_in(path, &entry, &sp);
    if (rc < 0) {
        hal_console_puts("Load failed.\r\n");
        pcb->state = PROC_STATE_UNUSED; // Free the PCB
        return;
    }

    if (sp == 0) sp = (uint32_t)APP_END; // Ensure SP is valid

    hal_console_puts("swap_in OK\r\n");

    // --- VALIDATION ---
    uint32_t pc_addr = entry & ~1u;  // Ensure halfword alignment
    if (sp <= APP_BASE || sp > APP_END) {
        hal_console_puts("Bad SP (OOB)\r\n");
        pcb->state = PROC_STATE_UNUSED;
        return;
    }
    if (pc_addr < APP_BASE || pc_addr >= APP_END) {
        hal_console_puts("Bad PC (OOB)\r\n");
        pcb->state = PROC_STATE_UNUSED;
        return;
    }

    // --- SET UP USER STACK ---
    // The ARM ABI requires the stack to be 8-byte aligned.
    sp &= ~7u;

    uint32_t *user_stack = (uint32_t *)sp;

    *--user_stack = 0x01000000u; // xPSR: Set T-bit for Thumb mode
    *--user_stack = entry;       // PC: The process entry point
    *--user_stack = 0;           // LR
    *--user_stack = 0;           // R12
    *--user_stack = 0;           // R3
    *--user_stack = 0;           // R2
    *--user_stack = 0;           // R1
    *--user_stack = 0;           // R0

    pcb->sp = (uint32_t)user_stack;

    hal_console_puts("Process created with PID ");
    hal_console_put_int(pcb->pid);
    hal_console_puts(", state=READY\r\n");
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
