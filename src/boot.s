.syntax unified
.cpu cortex-m3
.thumb

/* Make all handlers visible to the linker */
.global Reset_Handler
.global NMI_Handler
.global HardFault_Handler
.global Default_Handler

/*
 * The vector table for a Cortex-M3.
 * The LSB of each handler address must be 1 to indicate Thumb mode.
 * The linker usually handles this automatically, but we will be explicit
 * by adding "+1" to the function addresses. This is a robust way to ensure
 * the CPU enters the correct execution state.
 */
.section .isr_vector
.word _estack                 /* Initial Stack Pointer */
.word Reset_Handler + 1       /* Reset Handler */
.word NMI_Handler + 1         /* NMI Handler */
.word HardFault_Handler + 1   /* HardFault Handler */
.word Default_Handler + 1     /* MemManage Fault Handler */
.word Default_Handler + 1     /* BusFault Handler */
.word Default_Handler + 1     /* UsageFault Handler */


.section .text
/*
 * Reset_Handler: This is the entry point of the system.
 */
.thumb_func
Reset_Handler:
    /* Copy the .data section from FLASH to RAM.
       The Cortex-M3 supports post-indexed addressing, which is more efficient. */
    ldr r0, =_etext
    ldr r1, =_sdata
    ldr r2, =_edata
copy_loop:
    cmp r1, r2
    bhs copy_done
    ldr r3, [r0], #4
    str r3, [r1], #4
    b copy_loop
copy_done:

    /* Zero out the .bss section in RAM */
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0
zero_loop:
    cmp r0, r1
    bhs zero_done
    str r2, [r0], #4
    b zero_loop
zero_done:

    /* Jump to the main C function */
    bl kmain

    /* If kmain ever returns, trap the CPU in an infinite loop */
    b .

/*
 * Exception Handlers
 * We provide distinct, simple handlers for critical faults.
 * If the system crashes, a debugger will show the PC stuck in one of these loops,
 * telling us what kind of fault occurred.
 */
.thumb_func
NMI_Handler:
    b .

.thumb_func
HardFault_Handler:
    b .

.thumb_func
Default_Handler:
    b .