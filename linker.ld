ENTRY(_start)

SECTIONS
{
    . = 0x40080000;
    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .bss : { *(.bss*) }
}
