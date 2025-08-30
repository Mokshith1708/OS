.syntax unified
.cpu cortex-m0
.thumb

.global Reset_Handler

/* Interrupt Vector Table */
.section .isr_vector
.word _estack
.word Reset_Handler

.section .text
Reset_Handler:
    /* Copy .data section from FLASH to RAM */
    ldr r0, =_etext
    ldr r1, =_sdata
    ldr r2, =_edata
copy_loop:
    cmp r1, r2
    bhs copy_done
    ldr r3, [r0]
    adds r0, r0, #4
    str r3, [r1]
    adds r1, r1, #4
    b copy_loop
copy_done:

    /* Zero .bss section */
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0
zero_loop:
    cmp r0, r1
    bhs zero_done
    str r2, [r0]
    adds r0, r0, #4
    b zero_loop
zero_done:

    /* Jump to kernel main */
    bl kmain

loop_forever:
    b loop_forever
