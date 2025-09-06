.syntax unified
.cpu cortex-m0
.thumb

.global Reset_Handler
.global NMI_Handler
.global HardFault_Handler
.global Default_Handler

.section .isr_vector
.word _estack
.word Reset_Handler + 1
.word NMI_Handler + 1
.word HardFault_Handler + 1
.word Default_Handler + 1
.word Default_Handler + 1
.word Default_Handler + 1

.section .text
.thumb_func
Reset_Handler:
    /* copy .data */
    ldr r0, =_etext
    ldr r1, =_sdata
    ldr r2, =_edata
1:  cmp r1, r2
    bhs 2f
    ldr r3, [r0]
    str r3, [r1]
    adds r0, #4
    adds r1, #4
    b 1b
2:
    /* zero .bss */
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0
3:  cmp r0, r1
    bhs 4f
    str r2, [r0]
    adds r0, #4
    b 3b
4:
    bl  kmain
    b   .

.thumb_func
NMI_Handler:
    b .

/* Print '!' on UART (0x4000C000 DR, 0x4000C018 FR) then loop.
   This makes invisible HardFaults obvious. */
.thumb_func
HardFault_Handler:
    ldr r0, =0x4000C018     /* UART FR */
    ldr r1, [r0]
    movs r2, #(1<<5)        /* TXFF bit */
    tst r1, r2
    bne 1f                  /* if full, skip once (best effort) */
    ldr r3, =0x4000C000     /* UART DR */
    movs r4, #'!'
    str r4, [r3]
1:  b .

.thumb_func
Default_Handler:
    b .
