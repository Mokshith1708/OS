/* === MPS2-AN385 MEMORY LAYOUT (QEMU Specific) === */
MEMORY
{
    /* MPS2-AN385 Flash/ROM */
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 4M
    
    /* MPS2-AN385 ZBT SSRAM1 (main RAM) */
    RAM (rwx)  : ORIGIN = 0x21000000, LENGTH = 4M
    
    /* Alternative SRAM regions (if needed) */
    SRAM2 (rwx) : ORIGIN = 0x20000000, LENGTH = 1M
}

/* Kernel and User memory split */
_KERNEL_RAM_START = ORIGIN(RAM);
_USER_SPACE_SIZE  = LENGTH(RAM) / 2;        /* 2 MB user */
_KERNEL_RAM_SIZE  = LENGTH(RAM) - _USER_SPACE_SIZE; /* 2 MB kernel */
_USER_SPACE_START = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;

/* Stack pointer - at top of RAM */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* End of kernel symbol for C code */
_kernel_end = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;

/* === ENTRY POINT === */
ENTRY(Reset_Handler)

/* === SECTIONS === */
SECTIONS
{
    /* --- VECTOR TABLE --- */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH
    
    /* --- TEXT SECTION --- */
    .text :
    {
        . = ALIGN(4);
        *(.text .text*)
        *(.rodata .rodata*)
        
        /* Constructor/Destructor tables */
        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP(*(.preinit_array*))
        __preinit_array_end = .;
        
        __init_array_start = .;
        KEEP(*(.init_array*))
        __init_array_end = .;
        
        __fini_array_start = .;
        KEEP(*(.fini_array*))
        __fini_array_end = .;
        
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* --- INIT SECTIONS --- */
    .init :
    {
        . = ALIGN(4);
        KEEP(*(.init))
        . = ALIGN(4);
    } > FLASH
    
    .fini :
    {
        . = ALIGN(4);
        KEEP(*(.fini))
        . = ALIGN(4);
    } > FLASH

    /* --- DATA SECTION (initialized RAM) --- */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data .data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH
    
    /* Store the LMA of .data section */
    _sidata = LOADADDR(.data);

    /* --- BSS SECTION (zero-initialized RAM) --- */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss .bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM
    
    /* --- HEAP SECTION --- */
    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        __heap_start__ = .;
        . = . + 0x1000; /* 4KB heap, adjust as needed */
        __heap_end__ = .;
    } > RAM
    
    /* --- STACK SECTION --- */
    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        . = . + 0x1000; /* 4KB stack, adjust as needed */
        . = ALIGN(8);
    } > RAM

    /* Discard unwanted sections */
    /DISCARD/ :
    {
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }
}

/* --- USER APP RAM REGION --- */
__app_ram_start__ = _USER_SPACE_START;
__app_ram_end__   = _USER_SPACE_START + _USER_SPACE_SIZE;

/* --- RAM START/END --- */
PROVIDE(__ram_start__ = ORIGIN(RAM));
PROVIDE(__ram_end__   = ORIGIN(RAM) + LENGTH(RAM));

/* --- HEAP/STACK SYMBOLS --- */
PROVIDE(__heap_start = __heap_start__);
PROVIDE(__heap_end = __heap_end__);

/* --- DEBUG INFO --- */
PROVIDE(_flash_start = ORIGIN(FLASH));
PROVIDE(_flash_size = LENGTH(FLASH));
PROVIDE(_ram_start = ORIGIN(RAM));
PROVIDE(_ram_size = LENGTH(RAM));