ENTRY(_start)

SECTIONS
{
  . = 0x10000;  /* آدرس بارگذاری در حافظه */
  
  .text : { *(.text) }
  .rodata : { *(.rodata) }
  .data : { *(.data) }
  .bss : { *(.bss) }
}