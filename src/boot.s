/*
 * MokshithOS Startup File (boot.s)
 * Target: Cortex-A9
 */
.syntax unified
.cpu cortex-a9
.arm

.global Reset_Handler
.global _start

_start:
    b Reset_Handler

/* --- The Vector Table --- */
.section .isr_vector, "a", %progbits
.global g_pfnVectors
g_pfnVectors:
    .word _start             /* Reset */
    .word UndefinedInstruction_Handler /* Undefined Instruction */
    .word SVC_Handler        /* Software Interrupt */
    .word PrefetchAbort_Handler /* Prefetch Abort */
    .word DataAbort_Handler  /* Data Abort */
    .word 0                  /* Reserved */
    .word IRQ_Handler        /* IRQ */
    .word FIQ_Handler        /* FIQ */

.weak UndefinedInstruction_Handler
.weak SVC_Handler
.weak PrefetchAbort_Handler
.weak DataAbort_Handler
.weak IRQ_Handler
.weak FIQ_Handler

.thumb_func
UndefinedInstruction_Handler:
    b .

.thumb_func
SVC_Handler:
    b .

.thumb_func
PrefetchAbort_Handler:
    b .

.thumb_func
DataAbort_Handler:
    b .

.thumb_func
IRQ_Handler:
    b .

.thumb_func
FIQ_Handler:
    b .

.section .text
Reset_Handler:
    /*
     * Disable interrupts
     */
    mrs r0, cpsr
    orr r0, r0, #0xc0
    msr cpsr_c, r0

    /*
     * Set up stack for SVC mode
     */
    ldr sp, =_estack

    /*
     * Invalidate L1 I-cache and D-cache, and TLB
     */
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 0  @ Invalidate I-cache
    mcr p15, 0, r0, c7, c10, 4 @ Clean and invalidate D-cache
    mcr p15, 0, r0, c8, c7, 0  @ Invalidate unified TLB

    /*
     * Call the kernel's main entry point (in C)
     */
    bl kmain

/* If kmain ever returns, loop forever */
hang:
    b hang
