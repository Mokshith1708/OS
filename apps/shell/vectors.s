.syntax unified
.cpu cortex-a9
.arm

.global Reset_Handler

/* Place vector table at start of RAM */
.section .vectors, "a", %progbits
.word _estack             /* initial SP */
.word Reset_Handler       /* entry PC */

.text
.global Reset_Handler
Reset_Handler:
    ldr r0, =#1             @ argc = 1
    ldr r1, =argv_array     @ r1 = &argv_array
    bl main
1:  b 1b

.section .rodata
shell_str:
    .asciz "shell"
argv_array:
    .word shell_str
    .word 0 @ NULL terminator for argv
