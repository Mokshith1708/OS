MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 64K
}
_KERNEL_RAM_START = ORIGIN(RAM);
_USER_SPACE_SIZE = 32K;
_KERNEL_RAM_SIZE = LENGTH(RAM) - _USER_SPACE_SIZE;
_USER_SPACE_START = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;
ENTRY(Reset_Handler)
_estack = _KERNEL_RAM_START + _KERNEL_RAM_SIZE;
SECTIONS
{
    .text : { . = ALIGN(4); KEEP(*(.isr_vector)) *(.text .text* .rodata .rodata*) . = ALIGN(4); _etext = .; } > FLASH
    .data : AT (ADDR(.text) + SIZEOF(.text)) { . = ALIGN(4); _sdata = .; *(.data .data*) . = ALIGN(4); _edata = .; } > RAM
    .bss (NOLOAD) : { . = ALIGN(4); _sbss = .; *(.bss .bss*) *(COMMON) . = ALIGN(4); _ebss = .; } > RAM
}

