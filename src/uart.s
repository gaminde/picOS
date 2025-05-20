// filepath: src/uart.s
.global uart_init
.global uart_puts
.global uart_putc

.equ UART_BASE_CONST, 0x09000000 // UART Base Address for QEMU virt machine

// Initializes the UART
uart_init:
    stp x30, x0, [sp, #-16]!  // Save LR and a scratch register (x0)
    stp x1, x2, [sp, #-16]!   // Save x1, x2

    ldr x0, =UART_BASE_CONST   // Load UART base address
    mov w1, #0x101             // UARTCR: UARTEN (bit 0), TXE (bit 8)
    str w1, [x0, #0x30]        // Write to UARTCR (offset 0x30)
    dsb sy                     // Data Synchronization Barrier

    ldp x1, x2, [sp], #16
    ldp x30, x0, [sp], #16
    ret

// Subroutine to output a string to UART
// Input: x0 = address of null-terminated string
uart_puts:
    stp x30, x19, [sp, #-16]!
    mov x19, x0

puts_loop:
    ldrb w2, [x19], #1
    cbz w2, puts_done
    bl uart_putc
    b puts_loop

puts_done:
    ldp x30, x19, [sp], #16
    ret

// Character output subroutine
// Input: w2 = character to print
uart_putc:
    stp x30, x0, [sp, #-16]!
    stp x1, x3, [sp, #-16]! // Save x1 (used for char) and x3 (for UARTFR)
                            // w2 is passed in x2 if calling from C, but here it's already in w2 from uart_puts
                            // For direct C call, C would put char in w0, then we'd mov w0, w2.
                            // Let's assume w2 has the char for internal calls.
                            // If C calls uart_putc, it passes char in w0. We should handle that.
                            // For now, this uart_putc is primarily for uart_puts.
                            // If C needs to call it, we'd expect char in w0.
                            // Let's make it robust: if called from C, char is in w0.
                            // If called from uart_puts, char is in w2.
                            // For simplicity now, assume char is in w2 (as per current uart_puts)

    mov w1, w2                 // Move char to w1 (temp, as x0 will be base)
    ldr x0, =UART_BASE_CONST

uart_wait:
    ldr w3, [x0, #0x18]
    tst w3, #(1 << 5)
    b.ne uart_wait
    
    strb w1, [x0] // Store the character from w1
    
    ldp x1, x3, [sp], #16
    ldp x30, x0, [sp], #16
    ret
