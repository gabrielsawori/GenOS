/* link.ld - REVISI */
OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
{
    . = 1M;
    .text : {
        *(.multiboot)   /* <--- INI KUNCI PENYELESAIANNYA */
        *(.text)
    }
    .data : { *(.data) }
    .bss  : { *(.bss) }
}