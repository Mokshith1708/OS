    /* jump_trampoline.s */
    .syntax unified
    .cpu cortex-m0
    .thumb

    .global jump_to_entry
    .type   jump_to_entry, %function

/* void jump_to_entry(uint32_t sp, uint32_t entry) -- never returns */
jump_to_entry:
    /* r0 = stack pointer, r1 = entry point */
    cpsid   i          /* disable interrupts */
    msr     msp, r0    /* load new main stack pointer */
    isb                 /* flush pipeline */
    bx      r1         /* branch to entry (Thumb bit decides state) */
    b       .          /* never return */


    .global proc_switch_to_user
    .type   proc_switch_to_user, %function

/* void proc_switch_to_user(uint32_t user_sp) -- never returns */
proc_switch_to_user:
    msr psp, r0         // Set Process Stack Pointer from r0

    movs r0, #2          // Set CONTROL register: Use PSP for Thread mode.
    msr control, r0
    isb                 // Instruction barrier

    ldr r0, =0xFFFFFFFD // Load EXC_RETURN into r0 from literal pool
    bx r0               // Return from exception. This starts the user process.
