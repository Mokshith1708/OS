#include <stdint.h>
#include "include/proc.h"

// The PendSV handler is used to perform context switching.
// It is triggered by the SysTick handler and has a low priority to ensure
// it only runs when no other higher-priority interrupts are active.

__attribute__((naked))
void PendSV_Handler(void) {
    __asm__ volatile (
        // 1. Save context of current process
        "ldr r2, =current_process\n"
        "ldr r1, [r2]\n"
        "cmp r1, #0\n"
        "beq .L_restore_context_pendsv\n" // No process running, skip save

        // Save the process stack pointer (PSP)
        "mrs r0, psp\n"

        // Push r4-r11 onto the process stack.
        // Cortex-M0 doesn't have stmdb. We need to manually push.
        "sub r0, #32\n" // Make space for r4-r11 (8 regs * 4 bytes)
        "str r0, [r1]\n" // Save new SP to pcb->sp

        "stmia r0!, {r4-r7}\n" // Store r4-r7
        "mov r4, r8\n"
        "mov r5, r9\n"
        "mov r6, r10\n"
        "mov r7, r11\n"
        "stmia r0!, {r4-r7}\n" // Store r8-r11

        ".L_restore_context_pendsv:\n"
        // 2. Call the C scheduler to choose the next process
        "cpsie i\n"
        "bl schedule\n"
        "cpsid i\n"

        // 3. Restore context of the next process
        "ldr r2, =current_process\n"
        "ldr r1, [r2]\n"
        "ldr r0, [r1]\n"      // r0 = new_process->sp

        // Pop r4-r11 from the new process's stack
        "ldmia r0!, {r4-r7}\n" // Restore r4-r7
        "mov r8, r4\n"
        "mov r9, r5\n"
        "mov r10, r6\n"
        "mov r11, r7\n"
        "ldmia r0!, {r4-r7}\n" // Restore r8-r11

        // Load the PSP with the new stack pointer
        "msr psp, r0\n"

        // 4. Return from interrupt
        "ldr r0, =0xFFFFFFF9\n" // EXC_RETURN
        "bx r0\n"
    );
}