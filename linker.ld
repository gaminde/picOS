ENTRY(_start)

PHDRS
{
    text_segment PT_LOAD FLAGS(5);   /* R E (Read + Execute = 4 + 1 = 5) */
    rodata_segment PT_LOAD FLAGS(4); /* R   (Read = 4) */
    data_segment PT_LOAD FLAGS(6);   /* R W (Read + Write = 4 + 2 = 6) */
}

SECTIONS
{
    /* Define the base address for the kernel image */
    . = 0x40000000;

    .text : ALIGN(4K) {
        KEEP(*(.text.vectors)) /* Place the exception vectors first and ensure they are kept */
        *(.text*)              /* All other text sections */
        *(.iplt)               /* Include .iplt here if present */
    } :text_segment /* Assign to 'text_segment' PHDR */

    .rodata : ALIGN(8) {
        *(.rodata*)            /* All read-only data sections */
        *(.eh_frame*)          /* Include .eh_frame here */
        *(.rela.dyn)           /* Include .rela.dyn here */
    } :rodata_segment /* Assign to 'rodata_segment' PHDR */

    .data : ALIGN(8) {
        *(.data*)              /* All initialized data sections */
        *(.igot.plt)           /* Include .igot.plt here if present */
    } :data_segment /* Assign to 'data_segment' PHDR */

    .bss : ALIGN(8) {
        __bss_start = .;       /* Define a symbol for the start of .bss */
        *(.bss*)               /* All uninitialized data sections */
        *(COMMON)              /* Common symbols */
        . = ALIGN(8);
        __bss_end = .;         /* Define a symbol for the end of .bss */
    } :data_segment /* .bss also goes into the data_segment (it's RW, zero-filled) */

    /* Define a symbol for the end of the image, useful for heap start etc. */
    . = ALIGN(8);
    __end__ = .;

    /* Discard notes and comments to avoid them being loaded */
    /DISCARD/ : {
        *(.note*)
        *(.comment*)
    }
}
