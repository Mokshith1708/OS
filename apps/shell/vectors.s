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
    bl main
1:  b 1b
