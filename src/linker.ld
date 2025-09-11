MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
  /* FIX: The lm3s6965evb only has 64K of RAM */
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 64K
}

_KERNEL_RAM_START = ORIGIN(RAM);
/* FIX: Split the 64K RAM into 32K for the user, 32K for the kernel */
_USER_SPACE_SIZE = 32K;
_KERNEL_RAM_SIZE = LENGTH(RAM) - _USER_SPACE_SIZE;
_USER_SPACE_START = _KERNEL_RAM_START + _KERNEL_RAM_SIZE; /* This will now be 0x20008000 */

ENTRY(Reset_Handler)
_estack = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;

SECTIONS
{
    .text : { . = ALIGN(4); KEEP(*(.isr_vector)) *(.text .text* .rodata .rodata*) . = ALIGN(4); _etext = .; } > FLASH
    .data : AT (ADDR(.text) + SIZEOF(.text)) { . = ALIGN(4); _sdata = .; *(.data .data*) . = ALIGN(4); _edata = .; } > RAM
    .bss (NOLOAD) : { . = ALIGN(4); _sbss = .; *(.bss .bss*) *(COMMON) . = ALIGN(4); _ebss = .; } > RAM
}

PROVIDE(__ram_start__ = ORIGIN(RAM));
PROVIDE(__ram_end__   = ORIGIN(RAM) + LENGTH(RAM));

/* FIX: These symbols now point to the correct, valid memory region */
__app_ram_start__ = _USER_SPACE_START;     /* User app starts at 0x20008000 */
__app_ram_end__   = _USER_SPACE_START + _USER_SPACE_SIZE;  /* Ends at 0x20010000 */