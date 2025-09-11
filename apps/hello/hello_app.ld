MEMORY
{
  USER_RAM (rwx) : ORIGIN = 0x20010000, LENGTH = 64K   /* User RAM starts after kernel RAM */
  FLASH    (rx)  : ORIGIN = 0x00000000, LENGTH = 256K  /* Flash memory */
}

_estack = ORIGIN(USER_RAM) + LENGTH(USER_RAM);  /* Stack pointer at end of user RAM */
ENTRY(Reset_Handler)

SECTIONS
{
  
  .vectors 0x20010000 :
  {
    KEEP(*(.vectors))
  } > USER_RAM

  /* Code and read-only data in USER_RAM */
  .text :
  {
    . = ALIGN(4);
    *(.text .text* .rodata .rodata*);
    . = ALIGN(4);
    _etext = .;
  } > USER_RAM

  /* Initialized data in USER_RAM */
  .data : AT (ADDR(.text) + SIZEOF(.text))
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data .data*);
    . = ALIGN(4);
    _edata = .;
  } > USER_RAM

  /* Zero-initialized data in USER_RAM */
  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss .bss*);
    *(COMMON);
    . = ALIGN(4);
    _ebss = .;
  } > USER_RAM
}

PROVIDE(__user_ram_start__ = ORIGIN(USER_RAM));
PROVIDE(__user_ram_end__   = ORIGIN(USER_RAM) + LENGTH(USER_RAM));
