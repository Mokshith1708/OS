/*
 * MokshithOS Startup File (boot.s)
 * Target: Cortex-M0
 */
.syntax unified
.cpu cortex-m0
.thumb

.global Reset_Handler
/* We will define other handlers in C using the 'weak' attribute */
.weak NMI_Handler
.weak HardFault_Handler
.weak SVC_Handler
.weak PendSV_Handler
.weak SysTick_Handler

/* --- The Vector Table --- */
.section .isr_vector, "a", %progbits
.global g_pfnVectors
g_pfnVectors:
    .word _estack            /* Initial Stack Pointer */
    .word Reset_Handler + 1      /* Reset Handler */
    .word NMI_Handler
    .word HardFault_Handler
    .word 0, 0, 0, 0, 0, 0, 0 /* Reserved */
    .word SVC_Handler        /* Supervisor Call */
    .word 0, 0               /* Reserved */
    .word PendSV_Handler
    .word SysTick_Handler
    /* Add other device-specific interrupt vectors here if needed */

.section .text
.thumb_func
Reset_Handler:
    /* Call the kernel's main entry point (in C) */
    bl kmain

/* If kmain ever returns, loop forever */
hang:
    b hang

/* --- Default Handler for unused interrupts --- */
.thumb_func
Default_Handler:
    b .

/*
 * Use the .weak attribute to allow C functions to override these.
 * If no C function is provided, this default handler will be used.
 */
.macro def_weak_handler handler_name
    .weak \handler_name
    .thumb_set \handler_name, Default_Handler
.endm

def_weak_handler NMI_Handler
def_weak_handler HardFault_Handler
def_weak_handler SVC_Handler
def_weak_handler PendSV_Handler
def_weak_handler SysTick_Handler