/* === MPS2-AN385 MEMORY LAYOUT (16MB RAM, Kernel/User Split) === */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 4M
    RAM   (rw) : ORIGIN = 0x20000000, LENGTH = 16M
}

/* Kernel/User split */
_KERNEL_RAM_START = ORIGIN(RAM);
_KERNEL_RAM_SIZE  = LENGTH(RAM)/2;          /* 8 MB kernel */
_USER_SPACE_START = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;
_USER_SPACE_SIZE  = LENGTH(RAM)/2;          /* 8 MB user */

/* Minimum stack & heap sizes */
_Min_Stack_Size = 0x100000; /* 1 MB */
_Min_Heap_Size  = 0x2000;   /* 8 KB */

/* Stack pointer and kernel end */
PROVIDE(_estack = _KERNEL_RAM_START + _KERNEL_RAM_SIZE); /* initial SP */
PROVIDE(_kernel_end = _KERNEL_RAM_START + _KERNEL_RAM_SIZE);

/* Entry point */
ENTRY(Reset_Handler)

SECTIONS
{
    /* === Vector Table in FLASH === */
    .isr_vector :
    {
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* === Text + RO Data in FLASH === */
    .text :
    {
        *(.text*)
        *(.rodata*)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
        _sidata = LOADADDR(.kernel_data);
    } > FLASH

    /* === Kernel RAM Sections === */
    .kernel_data :
    {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT> FLASH

    .kernel_bss (NOLOAD) :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    .kernel_heap (NOLOAD) :
    {
        _kheap_start = .;
        . = . + _Min_Heap_Size;
        _kheap_end = .;
    } > RAM

    .kernel_stack (NOLOAD) :
    {
        _kstack_top = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;
        _kstack_bottom = _kstack_top - _Min_Stack_Size;
    } > RAM

    /* === User RAM Sections === */
    .user_data :
    {
        _usdata = .;
        *(.user_data*)
        _uedata = .;
    } > RAM AT> FLASH

    .user_bss (NOLOAD) :
    {
        _usbss = .;
        *(.user_bss*)
        _uebss = .;
    } > RAM

    .user_heap (NOLOAD) :
    {
        _uheap_start = .;
        . = . + _Min_Heap_Size;
        _uheap_end = .;
    } > RAM

    .user_stack (NOLOAD) :
    {
        _ustack_top = _USER_SPACE_START + _USER_SPACE_SIZE;
        _ustack_bottom = _ustack_top - _Min_Stack_Size;
    } > RAM
}

/* --- User App RAM Region --- */
PROVIDE(__app_ram_start__ = _USER_SPACE_START);
PROVIDE(__app_ram_end__   = _USER_SPACE_START + _USER_SPACE_SIZE);

/* --- RAM Start/End --- */
PROVIDE(__ram_start__ = ORIGIN(RAM));
PROVIDE(__ram_end__   = ORIGIN(RAM) + LENGTH(RAM));

/* --- Heap/Stack Symbols --- */
PROVIDE(__kheap_start   = _kheap_start);
PROVIDE(__kheap_end     = _kheap_end);
PROVIDE(__kstack_top    = _kstack_top);
PROVIDE(__kstack_bottom = _kstack_bottom);

PROVIDE(__uheap_start   = _uheap_start);
PROVIDE(__uheap_end     = _uheap_end);
PROVIDE(__ustack_top    = _ustack_top);
PROVIDE(__ustack_bottom = _ustack_bottom);

/* --- Debug Info --- */
PROVIDE(_flash_start = ORIGIN(FLASH));
PROVIDE(_flash_size  = LENGTH(FLASH));
PROVIDE(_ram_start   = ORIGIN(RAM));
PROVIDE(_ram_size    = LENGTH(RAM));
