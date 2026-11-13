ENTRY(_start)

SECTIONS
{
    . = 0x40080000;
    __kernel_start = .;

    . = ALIGN(0x1000);
    __text_start = .;
    .text :
    {
        KEEP(*(.text.boot))
        KEEP(*(.text.vectors))
        *(.text .text.*)
    }
    . = ALIGN(0x1000);
    __text_end = .;

    __rodata_start = .;
    .rodata :
    {
        *(.rodata .rodata.*)
    }
    . = ALIGN(0x1000);
    __rodata_end = .;

    .data :
    {
        *(.data .data.*)
    }
    . = ALIGN(0x1000);

    .bss (NOLOAD) :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        __bss_end = .;
    }

    . = ALIGN(16);
    __kernel_end = .;

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
    }
}
