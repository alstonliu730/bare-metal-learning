SECTIONS
{
    . = 0x80000;     /* Kernel load address for AArch64 */
    .text : {
        __text_start = .;
        KEEP(*(.text.boot))
        *(.text .text.* .gnu.linkonce.t*) 
        __text_end = .;
    }
    .rodata : { 
        *(.rodata .rodata.* .gnu.linkonce.r*) 
    }
    . = ALIGN(4096);
    PROVIDE(_data = .);
    .data : { 
        *(.data .data.* .gnu.linkonce.d*) 
    }
    .bss (NOLOAD): {
        . = ALIGN(16);
        __bss_start = .; 
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(16); 
        __bss_end = .; 
    }

    . = ALIGN(4096);
    . = . + 0x1000;    /* Adding 20KB padding after BSS */
    __stack_bottom = .;
    . = . + 0x20000;  /* 128KB stack */
    . = ALIGN(16);
    __stack_top = .;

    _end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}