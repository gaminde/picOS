.global _start

// Functions from other files we will call or use
.extern kernel_main
.extern exception_vectors_start
.extern uart_init // From uart.s
.extern uart_puts // From uart.s

.section .rodata
    boot_msg: .asciz "picOS Bootloader Initialized.\n"

.text
_start:
    // 1. Set up stack pointer
    ldr x30, =0x40100000 // Example stack top
    mov sp, x30

    // 2. Set VBAR_EL1 to point to our exception vector table
    ldr x0, =exception_vectors_start
    msr VBAR_EL1, x0
    isb                              // Ensure VBAR write is seen

    // 3. Initialize UART
    bl uart_init

    // 4. Print a boot message
    adr x0, boot_msg
    bl uart_puts

    // trigger an exception to test the vector table
    // svc #0

    // 5. Jump to kernel_main in C
    bl kernel_main

// Infinite loop if kernel_main returns (it shouldn't)
end_loop:
    b end_loop
