/*
 * src/linker.ld - CORRECTED
 * Target: lm3s6965evb board with 64MB of custom addon RAM
 */

MEMORY
{
  /* FLASH is at the standard location for this board */
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K

  /* RAM now points to our custom 64MB memory region added in QEMU */
  RAM (rwx)  : ORIGIN = 0x60000000, LENGTH = 64M
}

/* --- Define the Memory Partition --- */
_KERNEL_RAM_START = ORIGIN(RAM);

/* Partition the 64MB RAM. Give the kernel 4MB and the rest to the user process. */
_KERNEL_RAM_SIZE = 4M;
_USER_SPACE_SIZE = LENGTH(RAM) - _KERNEL_RAM_SIZE;
_USER_SPACE_START = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;

/* Set the program's entry point */
ENTRY(Reset_Handler)

/* The kernel's stack will be at the top of its reserved 4MB region */
_estack = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;

SECTIONS
{
    /* The kernel's code (.text) and initial data values are stored in FLASH. */
    .text : {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        *(.text .text* .rodata .rodata*)
        . = ALIGN(4);
        _etext = .; /* Symbol marks end of text in FLASH */
    } > FLASH

    /*
     * The data section is loaded from FLASH into our new RAM at boot time.
     * The BSS section is also created in our new RAM.
     */
    .data : AT (ADDR(.text) + SIZEOF(.text)) {
        . = ALIGN(4);
        _sdata = .;
        *(.data .data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    .bss (NOLOAD) : {
        . = ALIGN(4);
        _sbss = .;
        *(.bss .bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM
}

/* --- Provide Global Symbols for the OS C code --- */
/*
 * This is the corrected section. These symbols are used by your
 * existing C code (like pmm.c, proc.c, and shell.c) to know the memory map.
 */
PROVIDE(__ram_start__ = ORIGIN(RAM));
PROVIDE(__ram_end__   = ORIGIN(RAM) + LENGTH(RAM));

/* These symbols now point to the correct, valid memory region within our 64MB RAM */
__app_ram_start__ = _USER_SPACE_START;
__app_ram_end__   = _USER_SPACE_START + _USER_SPACE_SIZE;