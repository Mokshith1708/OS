ENTRY(Reset_Handler)

/* App RAM window must match kernel: 0x20000000..0x2000F000 (60KB) */
MEMORY {
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x0000F000
}

_estack = ORIGIN(RAM) + LENGTH(RAM);  /* 0x2000F000 */

SECTIONS {
    .vectors 0x20000000 : {
        KEEP(*(.vectors))
    } > RAM

    .text : {
        *(.text.Reset_Handler)
        *(.text*)
        *(.rodata*)
    } > RAM

    .data : {
        *(.data*)
    } > RAM

    .bss : {
        *(.bss*) *(COMMON)
    } > RAM
}
