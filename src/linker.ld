/*
 * MokshithOS Linker Script (linker.ld)
 * Target: Cortex-M0
 *
 * Description: Defines the memory map for the OS kernel.
 */
ENTRY(Reset_Handler)

/* --- Memory Layout --- */
/* NOTE: You MUST update these values for your specific chip! */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 128K
    RAM   (rwx): ORIGIN = 0x20000000, LENGTH = 16K
}

/* --- Global Symbols --- */
_estack = ORIGIN(RAM) + LENGTH(RAM); /* Top of stack, grows down */
_ram_end = _estack;
_os_ram_size = 4K; /* Reserve 4K of RAM for the OS */
_app_ram_start = ORIGIN(RAM) + _os_ram_size;

SECTIONS
{
    /* The vector table must be at the beginning of FLASH */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* Kernel code and read-only data follows the vector table */
    .text :
    {
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .init :
    {
        KEEP(*(.init))
        KEEP(*(.init_array))
        KEEP(*(.fini_array))
    } > FLASH

    /* This symbol marks where the .data initial values are in FLASH */
    _sidata = .;

    /* Kernel's initialized data section in RAM */
    /* AT> FLASH specifies where to load it from */
    .data : AT ( _sidata )
    {
        . = ALIGN(4);
        _sdata = .;        /* .data start in RAM */
        *(.data*)
        _edata = .;        /* .data end in RAM */
        . = ALIGN(4);
    } > RAM

    /* Kernel's uninitialized data section (BSS) in RAM */
    .bss (NOLOAD) :
    {
        _sbss = .;         /* .bss start in RAM */
        *(.bss*)
        *(COMMON)
        _ebss = .;         /* .bss end in RAM */
    } > RAM

    .ramdisk :
    {
        . = ALIGN(4);
        __ramdisk_start = .;
        KEEP(*ramdisk.o(.data))
        __ramdisk_end = .;
    } > RAM

    /* _end marks the end of the kernel's static memory usage */
    /* The kernel heap will start here */
    _end = .;

    /* Define user heap symbols */
    __uheap_start = _app_ram_start;
    __uheap_end = _ram_end;

    /* Define app ram symbols */
    __app_ram_start__ = _app_ram_start;
    __app_ram_end__ = _ram_end;

    /* Define total RAM symbols */
    __ram_start__ = ORIGIN(RAM);
    __ram_end__ = ORIGIN(RAM) + LENGTH(RAM);
}