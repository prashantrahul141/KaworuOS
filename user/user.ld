OUTPUT_FORMAT(elf64-littleaarch64)

ENTRY(_start)

PHDRS
{
    text   PT_LOAD FLAGS(5); /* R_X */
    rodata PT_LOAD FLAGS(4); /* R__ */
    data   PT_LOAD FLAGS(6); /* RW_ */
}

SECTIONS
{
    . = 0x400000;

    .text : ALIGN(0x1000)
    {
        *(.text*)
    } :text

    .rodata : ALIGN(0x1000)
    {
        *(.srodata .srodata.*)
        *(.rodata .rodata.*)
    } :rodata

    .data : ALIGN(0x1000)
    {
        *(.sdata .sdata.*)
        *(.data .data.*)
    } :data

    .bss : ALIGN(0x1000)
    {
        *(.sbss .sbss.*)
        *(.bss .bss.*)
        *(COMMON)
    } :data
}
