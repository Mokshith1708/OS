MEMORY
{
  /* 
   * FIX: This now matches the user space defined by the corrected OS linker script.
   * It starts at 0x20008000 and has a size of 32K.
   */
  USER_RAM (rwx) : ORIGIN = 0x20008000, LENGTH = 32K
  FLASH    (rx)  : ORIGIN = 0x00000000, LENGTH = 256K
}

/* The stack pointer is at the top of the app's allocated RAM */
_estack = ORIGIN(USER_RAM) + LENGTH(USER_RAM);  /* This correctly becomes 0x20010000 */
ENTRY(Reset_Handler)

SECTIONS
{
  /* FIX: The vector table must be placed at the new starting address */
  .vectors ORIGIN(USER_RAM) :
  {
    KEEP(*(.vectors))
  } > USER_RAM

  /* All other sections are placed within the USER_RAM region */
  .text :
  {
    . = ALIGN(4);
    *(.text .text* .rodata .rodata*);
    . = ALIGN(4);
    _etext = .;
  } > USER_RAM

  .data : AT (ADDR(.text) + SIZEOF(.text))
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data .data*);
    . = ALIGN(4);
    _edata = .;
  } > USER_RAM

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