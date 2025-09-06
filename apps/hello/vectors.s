.syntax unified
.cpu cortex-m0
.thumb

.global Reset_Handler

/* Place vector table at start of RAM */
.section .vectors, "a", %progbits
.word _estack             /* initial SP = 0x2000F000 (set by linker) */
.word Reset_Handler + 1   /* entry PC, Thumb bit set */

.text
.thumb_func
.global Reset_Handler
Reset_Handler:
    bl main
1:  b 1b
