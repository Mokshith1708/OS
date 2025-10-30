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

    mov r0, #2          // Set CONTROL register: Use PSP for Thread mode.
    msr control, r0
    isb                 // Instruction barrier

    mov lr, #0xFFFFFFFD // Load EXC_RETURN into LR. Return to Thread mode, use PSP.
    bx lr               // Return from exception. This starts the user process.


    .global PendSV_Handler
    .type   PendSV_Handler, %function
PendSV_Handler:
    // Disable interrupts during the context switch
    cpsid   i

    // If current_process is NULL, we are starting the first process, so skip saving context.
    ldr     r2, =current_process
    ldr     r3, [r2]
    cbz     r3, .L_restore_context_pendsv

    // --- Save the context of the current process ---
    // Get current process stack pointer
    mrs     r0, psp
    // Push R4-R11 and LR onto the process stack. `stmdb` decrements before storing.
    stmdb   r0!, {r4-r11, lr}
    // Save the new stack pointer value back into the PCB (current_process->sp)
    str     r0, [r3]

.L_call_scheduler_pendsv:
    // Call the C scheduler to choose the next process. The result will be in current_process.
    bl      schedule

.L_restore_context_pendsv:
    // --- Restore the context of the next process ---
    // Get the new current_process
    ldr     r2, =current_process
    ldr     r3, [r2]
    // Get the new process's stack pointer from its PCB
    ldr     r0, [r3]
    // Pop R4-R11 and LR from the new process's stack. `ldmia` increments after loading.
    ldmia   r0!, {r4-r11, lr}
    // Update the process stack pointer
    msr     psp, r0

    // Re-enable interrupts and return from exception
    cpsie   i
    bx      lr
