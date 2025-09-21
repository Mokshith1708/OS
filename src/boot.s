/* boot.s - Corrected for MPS2-AN385 Cortex-M3 */
.syntax unified
.cpu cortex-m3
.thumb

.global Reset_Handler
.global NMI_Handler  
.global HardFault_Handler
.global Default_Handler

/* Vector Table */
.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object

g_pfnVectors:
    .word _estack                    /* Initial Stack Pointer */
    .word Reset_Handler + 1          /* Reset Handler */
    .word NMI_Handler + 1            /* NMI Handler */
    .word HardFault_Handler + 1      /* Hard Fault Handler */
    .word Default_Handler + 1        /* Memory Management */
    .word Default_Handler + 1        /* Bus Fault */
    .word Default_Handler + 1        /* Usage Fault */
    .word 0                          /* Reserved */
    .word 0                          /* Reserved */
    .word 0                          /* Reserved */
    .word 0                          /* Reserved */
    .word Default_Handler + 1        /* SVCall */
    .word Default_Handler + 1        /* Debug Monitor */
    .word 0                          /* Reserved */
    .word Default_Handler + 1        /* PendSV */
    .word Default_Handler + 1        /* SysTick */
    
    /* External Interrupts - add more as needed */
    .rept 32
    .word Default_Handler + 1
    .endr

.size g_pfnVectors, .-g_pfnVectors

.section .text
.thumb_func
Reset_Handler:
    /* Set stack pointer explicitly (safety measure) */
    ldr r0, =_estack
    mov sp, r0
    
    /* Copy .data section from flash to RAM */
    ldr r0, =_sidata      /* Source: LMA of .data in flash */
    ldr r1, =_sdata       /* Destination: VMA start in RAM */
    ldr r2, =_edata       /* End of .data in RAM */
    
    /* Check if we need to copy anything */
    cmp r1, r2
    beq zero_bss          /* Skip if no data to copy */
    
copy_data_loop:
    cmp r1, r2
    bhs zero_bss          /* Branch if r1 >= r2 */
    ldr r3, [r0]          /* Load from flash */
    str r3, [r1]          /* Store to RAM */
    adds r0, r0, #4       /* Increment source */
    adds r1, r1, #4       /* Increment destination */
    b copy_data_loop

zero_bss:
    /* Zero initialize .bss section */
    ldr r0, =_sbss        /* Start of .bss */
    ldr r1, =_ebss        /* End of .bss */
    movs r2, #0           /* Zero value */
    
    /* Check if we need to zero anything */
    cmp r0, r1
    beq call_main         /* Skip if no bss to zero */

zero_bss_loop:
    cmp r0, r1
    bhs call_main         /* Branch if r0 >= r1 */
    str r2, [r0]          /* Store zero */
    adds r0, r0, #4       /* Increment pointer */
    b zero_bss_loop

call_main:
    /* Call main function */
    bl kmain
    
    /* If main returns, loop forever */
hang:
    b hang

.thumb_func
NMI_Handler:
    b NMI_Handler

.thumb_func
HardFault_Handler:
    /* Determine which stack was used */
    movs r0, #4
    mov r1, lr
    tst r0, r1
    beq use_msp
    mrs r0, psp           /* Use Process Stack Pointer */
    b call_handler
use_msp:
    mrs r0, msp           /* Use Main Stack Pointer */

call_handler:
    /* Call C handler with stack frame pointer */
    bl hardfault_dump
    
    /* Loop forever after handling */
hardfault_loop:
    b hardfault_loop

.thumb_func
Default_Handler:
    b Default_Handler

.end