.syntax unified
    .cpu cortex-a9
    .arm

    .global jump_to_entry
    .type   jump_to_entry, %function

/* void jump_to_entry(uint32_t sp, uint32_t entry) -- never returns */
jump_to_entry:
    /* r0 = stack pointer, r1 = entry point */
    mov sp, r0
    bx r1

    .global proc_switch_to_user
    .type   proc_switch_to_user, %function

/* void proc_switch_to_user(uint32_t user_sp, uint32_t entry) -- never returns */
proc_switch_to_user:
    mov sp, r0              // r0 has the user stack pointer

    // Restore the user context from the stack
    // The stack frame was created in sys_exec in proc.c
    // Frame: {r0-r3, r12, lr, pc, xpsr}
    ldmia sp!, {r0-r3, r12, lr}
    
    // Pop the user PC and xPSR into temporary registers
    ldmia sp!, {r4, r5}

    // Restore the SPSR from the popped xPSR
    msr spsr_cxsf, r5

    // Use movs to return from exception, which copies SPSR to CPSR
    // and branches to the address in r4 (the user PC)
    movs pc, r4