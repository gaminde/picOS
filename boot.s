.global _start
.global kernel_main

.section .rodata
    hello_msg: .asciz "Hello from Boot.s\n"  // Null-terminated string
    kernel_msg: .asciz "Hello from Kernel\n"  // Null-terminated string

.text
_start:
    // Set up stack pointer to a safe address (just below our code)
    ldr x30, =0x40100000     // 1MB after our code should be safe
    mov sp, x30              // Initialize stack pointer

    // UART0 base address for QEMU virt machine
    ldr x0, =0x09000000      // UART_BASE in x0

    // Enable UART: UARTEN=1, TXE=1
    mov w1, #0x101           // UARTCR: UARTEN (bit 0), TXE (bit 8)
    str w1, [x0, #0x30]      // UARTCR (0x30)
    dsb sy                   // Ensure write completes

    // Print our string from .rodata
    ldr x1, =hello_msg        // Get address of string
    bl uart_puts             // Call string output routine
    
    // Print another newline
    mov w2, #10
    bl uart_putc

    bl kernel_main

end_loop:
    b end_loop

kernel_main:
    adr x1, kernel_msg       // Get address of string
    bl uart_puts             // Call string output routine
    ret           // Infinite loop

// Subroutine to output a string to UART
// Input: x1 = address of null-terminated string
// Uses: x1, w2, w3
uart_puts:
    // Save link register to preserve return address
    stp x30, x1, [sp, #-16]!

puts_loop:
    // Load byte (character) from string
    ldrb w2, [x1], #1
    
    // Check for null terminator
    cbz w2, puts_done
    
    // Output the character
    bl uart_putc
    
    // Loop for next character
    b puts_loop

puts_done:
    // Restore link register and return
    ldp x30, x1, [sp], #16
    ret

// Character output subroutine
uart_putc:
    // Save link register
    str x30, [sp, #-16]!

    // Wait for UART to be ready
uart_wait:
    ldr w3, [x0, #0x18]      // Load UARTFR (Flag Register)
    tst w3, #(1 << 5)        // Test bit 5 (TXFF)
    b.ne uart_wait           // If FIFO full, keep waiting
    
    // Write character
    strb w2, [x0]            // Write to UARTDR (0x0)
    
    // Restore link register and return
    ldr x30, [sp], #16
    ret
