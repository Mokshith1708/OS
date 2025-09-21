.syntax unified
.cpu cortex-m3
.thumb

/* === Exported symbols === */
.global Reset_Handler
.global NMI_Handler
.global HardFault_Handler
.global MemManage_Handler
.global BusFault_Handler
.global UsageFault_Handler
.global SVC_Handler
.global DebugMon_Handler
.global PendSV_Handler
.global SysTick_Handler
.global Default_Handler

/* === Vector Table (M3) === */
.section .isr_vector, "a", %progbits
.align 2
.global g_pfnVectors
.type g_pfnVectors, %object
g_pfnVectors:
    .word _estack               /* Initial stack pointer */
    .word Reset_Handler + 1
    .word NMI_Handler + 1
    .word HardFault_Handler + 1
    .word MemManage_Handler + 1
    .word BusFault_Handler + 1
    .word UsageFault_Handler + 1
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler + 1
    .word DebugMon_Handler + 1
    .word 0
    .word PendSV_Handler + 1
    .word SysTick_Handler + 1
    /* Fill remaining entries with Default_Handler */
    .rept 32
        .word Default_Handler + 1
    .endr
.size g_pfnVectors, .-g_pfnVectors

/* === Reset Handler === */
.section .text
.thumb_func
Reset_Handler:
    /* Initialize stack pointer */
    ldr r0, =_estack
    mov sp, r0

    /* Copy .data section from FLASH to RAM */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
    cmp r1, r2
    beq zero_bss

copy_data_loop:
    cmp r1, r2
    bhs zero_bss
    ldr r3, [r0]
    str r3, [r1]
    adds r0, r0, #4
    adds r1, r1, #4
    b copy_data_loop

/* Zero .bss section */
zero_bss:
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0
    cmp r0, r1
    beq call_main

zero_bss_loop:
    cmp r0, r1
    bhs call_main
    str r2, [r0]
    adds r0, r0, #4
    b zero_bss_loop

/* Call kernel main */
call_main:
    bl kmain
hang:
    b hang

/* === Default Handlers === */
.thumb_func
NMI_Handler:        b Default_Handler
.thumb_func
HardFault_Handler:  b Default_Handler
.thumb_func
MemManage_Handler:  b Default_Handler
.thumb_func
BusFault_Handler:   b Default_Handler
.thumb_func
UsageFault_Handler: b Default_Handler
.thumb_func
SVC_Handler:        b Default_Handler
.thumb_func
DebugMon_Handler:   b Default_Handler
.thumb_func
PendSV_Handler:     b Default_Handler
.thumb_func
SysTick_Handler:    b Default_Handler

/* === Default Handler: debug-friendly === */
.thumb_func
Default_Handler:
    bkpt #0          /* halt in debugger */
    b .              /* infinite loop */

.end
