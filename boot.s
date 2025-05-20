.global _start
.global kernel_main // This will be provided by kernel.c
.global uart_puts   // Make uart_puts available to C
.global uart_putc   // Make uart_putc available to C

.equ UART_BASE_CONST, 0x09000000 // Define as an absolute constant

.section .rodata
    hello_msg: .asciz "Hello from Boot.s\n"  // Null-terminated string

.text
_start:
    // Set up stack pointer
    ldr x30, =0x40100000
    mov sp, x30

    // Enable UART
    ldr x0, =UART_BASE_CONST   // Load the constant value directly
    mov w1, #0x101           // UARTCR: UARTEN (bit 0), TXE (bit 8)
    str w1, [x0, #0x30]      // UARTCR (0x30) using base in x0
    dsb sy                   // Ensure write completes

    // Print our string from .rodata
    adr x0, hello_msg        // Load address of hello_msg into x0 (C convention for uart_puts)
    bl uart_puts             // Call string output routine

    bl kernel_main           // Call the C kernel_main

end_loop:
    b end_loop

// Subroutine to output a string to UART
// Input: x0 = address of null-terminated string (AArch64 C calling convention)
// Uses: x19 (callee-saved for string pointer), w2, (x0, w3 implicitly by uart_putc)
uart_puts:
    stp x30, x19, [sp, #-16]! // Save Link Register and x19 (callee-saved)
    mov x19, x0               // Move string pointer from x0 to x19

puts_loop:
    ldrb w2, [x19], #1        // Load byte (character) from string in x19, post-increment
    cbz w2, puts_done         // Check for null terminator
    
    // uart_putc expects char in w2. It will load its own UART base.
    bl uart_putc              // Output the character
    
    b puts_loop

puts_done:
    ldp x30, x19, [sp], #16   // Restore Link Register and x19
    ret

// Character output subroutine
// Input: w2 = character to print
// Uses: x0 (for UART base), w3
uart_putc:
    stp x30, x0, [sp, #-16]!  // Save Link Register and x0 (we will overwrite x0)

    ldr x0, =UART_BASE_CONST    // Load the constant value directly

uart_wait:
    ldr w3, [x0, #0x18]      // Load UARTFR (Flag Register) using base in x0
    tst w3, #(1 << 5)        // Test bit 5 (TXFF - Transmit FIFO Full)
    b.ne uart_wait           // If FIFO full, keep waiting
    
    strb w2, [x0]            // Write character in w2 to UARTDR (Data Register) using base in x0
    
    ldp x30, x0, [sp], #16   // Restore Link Register and original x0
    ret
