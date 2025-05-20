.global _start
.global kernel_main
.global uart_puts   // Make available to C and other assembly files
.global uart_putc   // ditto 

.extern exception_vectors_start // Declare external symbol defined in vectors.s

.equ UART_BASE_CONST, 0x09000000 // UART Base Address for QEMU virt machine

.section .rodata
    hello_msg: .asciz "Hello from Boot.s\n"

.text
_start:
    // Set up stack pointer
    // Ensure this address is valid and provides enough space for your stack.
    ldr x30, =0x40100000 // Example stack top (grows downwards)
    mov sp, x30

    // Set VBAR_EL1 to point to our exception vector table
    ldr x0, =exception_vectors_start // Load the address of the vector table
    msr VBAR_EL1, x0                 // Move this address into VBAR_EL1
    isb                              // Instruction Synchronization Barrier, ensure VBAR write is seen

    // Enable UART (PL011 on QEMU virt)
    ldr x0, =UART_BASE_CONST   // Load UART base address
    mov w1, #0x101             // UARTCR: UARTEN (bit 0), TXE (bit 8) to enable UART and transmitter
    str w1, [x0, #0x30]        // Write to UARTCR (offset 0x30)
    dsb sy                     // Data Synchronization Barrier, ensure write completes

    // Print our string from .rodata
    ldr x0, =hello_msg          // Load address of hello_msg into x0 (C convention for uart_puts)
    bl uart_puts               // Call string output routine

    // --- Test an exception
    // If we uncomment the next line - it will trigger a synchronous exception.
    // svc #0

    bl kernel_main             // Hand off to the kernel main function in our kernel.c file

end_loop:
    b end_loop                 // Infinite loop if kernel_main returns (it shouldn't)

// Subroutine to output a string to UART
// Input: x0 = address of null-terminated string (AArch64 C calling convention)
// Uses: x19 (callee-saved for string pointer), w2 (for character)
// Calls: uart_putc
uart_puts:
    stp x30, x19, [sp, #-16]! // Save Link Register (x30) and x19 (callee-saved)
    mov x19, x0               // Move string pointer from x0 to x19 (callee-saved)

puts_loop:
    ldrb w2, [x19], #1        // Load byte (character) from string in x19, post-increment x19
    cbz w2, puts_done         // Check for null terminator (if w2 is zero, branch to puts_done)
    
    // uart_putc expects char in w2. It will load its own UART base.
    bl uart_putc              // Output the character
    
    b puts_loop               // Loop for next character

puts_done:
    ldp x30, x19, [sp], #16   // Restore Link Register (x30) and x19
    ret                       // Return from subroutine

// Character output subroutine
// Input: w2 = character to print
// Uses: x0 (for UART base), w3 (for UARTFR)
uart_putc:
    stp x30, x0, [sp, #-16]!  // Save Link Register (x30) and x0 (we will overwrite x0)

    ldr x0, =UART_BASE_CONST  // Load UART base address into x0

uart_wait:
    ldr w3, [x0, #0x18]       // Load UARTFR (Flag Register, offset 0x18) using base in x0
    tst w3, #(1 << 5)         // Test bit 5 (TXFF - Transmit FIFO Full)
    b.ne uart_wait            // If FIFO full (TXFF is set), keep waiting
    
    strb w2, [x0]             // Write character in w2 to UARTDR (Data Register, offset 0x00) using base in x0
    
    ldp x30, x0, [sp], #16    // Restore Link Register (x30) and original x0
    ret                       // Return from subroutine
