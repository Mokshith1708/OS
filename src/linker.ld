/*
 * MokshithOS Linker Script (linker.ld)
 * Target: Cortex-A9
 *
 * Description: Defines the memory map for the OS kernel.
 */
ENTRY(_start)

/* --- Memory Layout --- */
MEMORY
{
    RAM (rwx) : ORIGIN = 0x10000000, LENGTH = 128M
}

/* --- Global Symbols --- */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > RAM

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        _edata = .;
        . = ALIGN(4);
    } > RAM

    .bss :
    {
        __bss_start__ = .;
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
        __bss_end__ = .;
    } > RAM

    .ramdisk :
    {
        . = ALIGN(4);
        __ramdisk_start = .;
        KEEP(*ramdisk.o(.data))
        __ramdisk_end = .;
    } > RAM

    _end = .;

    __app_ram_start__ = _end;
    __app_ram_end__ = _estack;
}
