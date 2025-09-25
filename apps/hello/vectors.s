.syntax unified
.cpu cortex-a9

.section .vectors, "a"
.word _estack           /* Initial Stack Pointer */
.word _start            /* Reset Handler (PC) */

.text
.global _start
_start:
    ldr sp, =_estack
    bl main
1:  b 1b
