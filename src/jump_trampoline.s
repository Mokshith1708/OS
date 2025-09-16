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
