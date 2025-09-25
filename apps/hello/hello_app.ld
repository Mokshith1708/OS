/*******************************************************************/
/* User Program Linker Script                                       */
/* Based on reference example with expanded sections                */
/*******************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x2000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x2000;

/* Define Memories */
MEMORY
{
   ps7_ddr_user : ORIGIN = 0x10100000, LENGTH = 0x1EF00000
}

/* Entry Point */
ENTRY(_start)

/* Sections */
SECTIONS
{
  /* User Text Section */
  .text : {
    . = ALIGN(4);
    _text_start = .;
    *(.vectors)       /* Interrupt vectors, if any */
    *(.text)          /* Code */
    *(.text.*)
    *(.rodata*)       /* Read-only data sometimes here */
    _text_end = .;
  } > ps7_ddr_user

  /* Initialized data */
  .data : {
    . = ALIGN(4);
    _data_start = .;
    *(.data)
    *(.data.*)
    _data_end = .;
  } > ps7_ddr_user

  /* Read-only data */
  .rodata : {
    . = ALIGN(4);
    _rodata_start = .;
    *(.rodata)
    *(.rodata.*)
    _rodata_end = .;
  } > ps7_ddr_user

  /* Small data sections if used */
  .sdata : {
    . = ALIGN(4);
    _sdata_start = .;
    *(.sdata)
    *(.sdata.*)
    _sdata_end = .;
  } > ps7_ddr_user

  /* Uninitialized data / BSS */
  .bss (NOLOAD) : {
    . = ALIGN(4);
    _bss_start = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)
    _bss_end = .;
  } > ps7_ddr_user

  /* Small BSS */
  .sbss (NOLOAD) : {
    . = ALIGN(4);
    _sbss_start = .;
    *(.sbss)
    *(.sbss.*)
    _sbss_end = .;
  } > ps7_ddr_user

  /* Heap (no load) */
  .heap (NOLOAD) : {
    . = ALIGN(4);
    _heap_start = .;
    . += _HEAP_SIZE;
    _heap_end = .;
  } > ps7_ddr_user

  /* Stack (no load) */
  .stack (NOLOAD) : {
    . = ALIGN(4);
    _stack_end = .;
    . += _STACK_SIZE;
    _stack_start = .;
  } > ps7_ddr_user

  /* Provide start and end addresses for the application RAM */
  __app_ram_start__ = ORIGIN(ps7_ddr_user);
  __app_ram_end__   = ORIGIN(ps7_ddr_user) + LENGTH(ps7_ddr_user);
  _estack = _stack_start;
  /* User space size */
  _user_space_size = __app_ram_end__ - __app_ram_start__;
}
