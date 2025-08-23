.global _start
_start:
    ldr x0, =stack_top
    mov sp, x0
    bl kmain
hang:
    b hang

.section .bss.stack
    .space 16384
stack_top:
