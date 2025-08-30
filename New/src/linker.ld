/* linker_microbit.ld - Linker script tuned for QEMU microbit (Cortex-M0) */

MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    RAM   (rwx): ORIGIN = 0x20000000, LENGTH = 16K
}

/* Partition kernel RAM and optional user space (adjust if you don't need user space) */
_KERNEL_RAM_START = ORIGIN(RAM);
_USER_SPACE_SIZE = 4K;                      /* reserve 4K for user process (change if needed) */
_KERNEL_RAM_SIZE = LENGTH(RAM) - _USER_SPACE_SIZE;
_USER_SPACE_START = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;

/* Entry point */
ENTRY(Reset_Handler)

/* Top of kernel stack (stack grows downward) */
_estack = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;

SECTIONS
{
    /* Vector table + code should live at start of FLASH (0x00000000) */
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))    /* ensure vector table is kept at start of flash */
        *(.text)                /* program code */
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;             /* end of code in FLASH */
    } > FLASH

    /* .data: VMA in RAM, LMA in FLASH (copied at boot) */
    .data : AT (ADDR(.text) + SIZEOF(.text))
    {
        . = ALIGN(4);
        _sdata = .;             /* start of data in RAM */
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;             /* end of data in RAM */
    } > RAM

    /* .bss: zero-initialized data in RAM */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;              /* start of .bss in RAM */
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;              /* end of .bss in RAM */
    } > RAM

    /* Provide stack/top-of-stack symbol (optional repeat) */
    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        _stack_start = .;
        . = . + 0; /* no actual allocation here, just symbol */
    } > RAM
}
