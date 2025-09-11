MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 128K   
}

_KERNEL_RAM_START = ORIGIN(RAM);
_USER_SPACE_SIZE = 64K;                      /* 64 KB user space */
_KERNEL_RAM_SIZE = LENGTH(RAM) - _USER_SPACE_SIZE;  /* 128K - 64K = 64 KB kernel */
_USER_SPACE_START = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;

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

__app_ram_start__ = _USER_SPACE_START;     /* User app starts after kernel RAM */
__app_ram_end__   = _USER_SPACE_START + _USER_SPACE_SIZE;  /* Ends at 0x20020000 (128K total base + 64K user) */
