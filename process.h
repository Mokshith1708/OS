#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <cstddef>

struct CPURegisters {
    uint64_t x[31];
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
};

class Process {
public:
    int pid;
    CPURegisters regs;
    enum State { NEW, READY, RUNNING, BLOCKED, TERMINATED } state;

    uint8_t stack[4096];
    size_t stack_size;

    Process(int id);         // only declaration
    void run();              // only declaration
};

void add_process(Process* p);
void schedule();
void switch_to(Process* next);

// Stub for context switch
extern "C" void context_switch(CPURegisters* old_regs, CPURegisters* new_regs);

#endif // PROCESS_H
