SECTIONS
{
    . = 0x80000;     /* Kernel load address for AArch64 */
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    . = ALIGN(4096);
    PROVIDE(_data = .);
    .data : { *(.data .data.* .gnu.linkonce.d*) }
    .bss (NOLOAD): {
        . = ALIGN(16);
        __bss_start = .; 
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(16); 
        __bss_end = .; 
    }
    _end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}