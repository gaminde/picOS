ENTRY(_start)
MEMORY {
    RAM (rw) : ORIGIN = 0x40000000, LENGTH = 64M
}

SECTIONS {
    . = ORIGIN(RAM); /* Set starting address */

    .text : {
        *(.text)
    } > RAM

    /* Ensure `.rodata` is placed directly after `.text` */
    . = ALIGN(16);
    .rodata : {
        *(.rodata)
    } > RAM

    .data : {
        *(.data)
    } > RAM

    .bss : {
        *(.bss)
    } > RAM
}
