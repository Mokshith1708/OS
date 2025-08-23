#include "process.h"
#include "uart.h"

#define MAX_PROCESSES 4

static Process* ready_queue[MAX_PROCESSES];
static int process_count = 0;
static int current = 0;

Process::Process(int id) : pid(id), stack_size(4096), state(NEW) {
    regs.sp = (uint64_t)(stack + stack_size);
    regs.pc = 0;
}

void Process::run() {
    uart_puts("Process ");
    uart_putc('0' + pid);
    uart_puts(" running\n");
}

void add_process(Process* p) {
    if (process_count < MAX_PROCESSES)
        ready_queue[process_count++] = p;
}

void switch_to(Process* next) {
    Process* prev = ready_queue[current];
    current = (current + 1) % process_count;
    context_switch(&prev->regs, &next->regs);
}

void schedule() {
    Process* next = ready_queue[current];
    switch_to(next);
}

// Minimal stub for context_switch
extern "C" void context_switch(CPURegisters* old_regs, CPURegisters* new_regs) {
    // TODO: implement real ARM64 context switch
}
