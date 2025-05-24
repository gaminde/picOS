// filepath: src/uart.s
.global uart_init
.global uart_puts
.global uart_putc
.global print_hex
.global print_uint

.equ UART_BASE_CONST, 0x09000000 // UART Base Address for QEMU virt machine

// Initializes the UART
uart_init:
    stp x30, x0, [sp, #-16]!  // Save LR and a scratch register (x0)
    stp x1, x2, [sp, #-16]!   // Save x1, x2

    ldr x0, =UART_BASE_CONST   // Load UART base address
    mov w1, #0x101             // UARTCR: UARTEN (bit 0), TXE (bit 8) - Enable UART and TX
    str w1, [x0, #0x30]        // Write to UARTCR (offset 0x30)
    dsb sy                     // Data Synchronization Barrier

    ldp x1, x2, [sp], #16
    ldp x30, x0, [sp], #16
    ret

// Character output subroutine
// Input: w0 = character to print (AAPCS64 for first argument)
uart_putc:
    stp x30, x1, [sp, #-16]!  // Save LR (x30) and x1 (used for UART base)
    stp x2, x3, [sp, #-16]!   // Save x2 (used for char) and x3 (used for UARTFR)

    mov w2, w0                 // Character to print is in w0, move to w2 for storing
    ldr x1, =UART_BASE_CONST   // Load UART base address into x1

uart_wait_putc_loop:
    ldr w3, [x1, #0x18]        // Read UARTFR from [UART_BASE + 0x18] into w3
    tst w3, #(1 << 5)          // Test TXFF bit (Transmit FIFO full, bit 5)
    b.ne uart_wait_putc_loop   // Loop if FIFO is full
    
    strb w2, [x1]              // Store the character from w2 to UARTDR [UART_BASE + 0x00]
    
    ldp x2, x3, [sp], #16
    ldp x30, x1, [sp], #16
    ret

// Subroutine to output a string to UART
// Input: x0 = address of null-terminated string
uart_puts:
    stp x30, x19, [sp, #-16]! // Save LR (x30) and x19 (callee-saved, used for string pointer)
    
    mov x19, x0               // x19 = current address in string

puts_loop:
    ldrb w0, [x19], #1        // Load byte into w0 (argument for uart_putc), advance x19
    cbz w0, puts_done         // If char is null, done
    bl uart_putc              // Call uart_putc (expects char in w0)
    b puts_loop

puts_done:
    ldp x30, x19, [sp], #16   // Restore LR and x19
    ret

// Prints a 64-bit value in hexadecimal
// Input: x0 = value to print
print_hex:
    stp x30, x19, [sp, #-16]! // Save LR, x19 (loop counter)
    stp x20, x21, [sp, #-16]! // Save x20 (original value), x21 (divisor 10)
    stp x1, x2, [sp, #-16]!   // Save x1 (current nibble), x2 (shift amount)
    stp x3, x4, [sp, #-16]!   // Save x3, x4 (scratch)

    mov x20, x0               // Copy value to x20
    
    // Print "0x" prefix
    mov w0, #'0'
    bl uart_putc
    mov w0, #'x'
    bl uart_putc

    mov x19, #16              // Loop 16 times (16 hex digits)

print_hex_digit_loop:
    // Calculate shift amount for the current most significant nibble remaining
    // Shift amount = (x19 - 1) * 4
    sub x2, x19, #1           
    mov x3, #4
    mul x2, x2, x3            // x2 = right shift amount to get MS nibble to LS position
    
    lsr x1, x20, x2           // Shift value right to get current nibble in lowest 4 bits
    and x1, x1, #0xF          // Isolate the nibble (lowest 4 bits)

    // Convert nibble (x1) to ASCII
    cmp x1, #9
    b.gt hex_is_letter
    add w0, w1, #'0'          // It's a digit '0'-'9'
    b hex_char_done
hex_is_letter:
    sub x4, x1, #10           // Convert 10-15 to 0-5 (use x4 as temp for w1)
    add w0, w4, #'A'          // Add to 'A'
hex_char_done:
    bl uart_putc              // Print the character

    subs x19, x19, #1         // Decrement loop counter
    b.ne print_hex_digit_loop // Loop if not zero

    ldp x3, x4, [sp], #16
    ldp x1, x2, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x30, x19, [sp], #16
    ret

// Prints a 64-bit unsigned integer in decimal
// Input: x0 = value to print
print_uint:
    stp x30, x19, [sp, #-16]! // Save LR, x19 (value being processed)
    stp x20, x21, [sp, #-16]! // Save x20 (buffer pointer), x21 (divisor 10)
    stp x22, x23, [sp, #-16]! // Save x22 (quotient), x23 (remainder/digit)
    stp x4, x5, [sp, #-16]!   // Save x4 (digit count), x5 (scratch)

    mov x19, x0               // x19 = value to print
    mov x21, #10              // x21 = 10 (divisor)

    // Handle zero case
    cmp x19, #0
    b.ne print_uint_not_zero
    mov w0, #'0'
    bl uart_putc
    b print_uint_cleanup      // Go to cleanup and return

print_uint_not_zero:
    // Allocate space on stack for char buffer (max 20 digits for 2^64-1)
    // We'll use 24 bytes for safety and alignment.
    sub sp, sp, #24
    mov x20, sp               // x20 = buffer pointer (points to start of buffer)
    
    mov x4, #0                // x4 = digit_count, tracks number of digits stored in buffer

print_uint_convert_loop:
    udiv x22, x19, x21        // x22 (quotient) = x19 / 10
    msub x23, x22, x21, x19   // x23 (remainder) = x19 - (x22 * 10)
                              // x23 is the current least significant digit (0-9)

    add w5, w23, #'0'         // Convert digit to ASCII char (use w5 as temp for w23)
    strb w5, [x20, x4]        // Store char in buffer: buffer[digit_count] = char
    add x4, x4, #1            // Increment digit_count

    mov x19, x22              // value = quotient
    cmp x19, #0               // If value is zero, all digits extracted
    b.ne print_uint_convert_loop

    // Digits are stored in buffer[0]...buffer[digit_count-1] in reverse order (LSD first)
    // Now print digits from buffer in correct order (MSD first)
    // which means printing from buffer[digit_count-1] down to buffer[0]
print_uint_print_loop:
    subs x4, x4, #1           // Decrement digit_count (now it's an index from count-1 down to 0)
    bmi print_uint_print_done // If index < 0 (i.e., x4 becomes -1), all digits printed

    ldrb w0, [x20, x4]        // Load char from buffer[index] into w0 (arg for uart_putc)
    bl uart_putc              // Print char

    b print_uint_print_loop

print_uint_print_done:
    add sp, sp, #24           // Deallocate stack buffer

print_uint_cleanup:
    ldp x4, x5, [sp], #16
    ldp x22, x23, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x30, x19, [sp], #16
    ret
