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
    /* r0 = user_sp, r1 = entry */
    mov sp, r0
    mov lr, #0
    mov r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0
    cpsie i
    msr cpsr_c, #0x10
    bx r1