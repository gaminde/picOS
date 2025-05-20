// filepath: include/uart.h
#ifndef UART_H
#define UART_H

// Initializes the UART.
void uart_init(void);

// Prints a null-terminated string to the UART.
void uart_puts(const char *s);

// Prints a single character to the UART.
// Note: The assembly implementation expects the char in w0 if called from C.
// The current uart.s is optimized for calls from uart_puts (char in w2).
// For direct C calls to uart_putc, uart.s would need adjustment or a wrapper.
// For now, C code should primarily use uart_puts.
// void uart_putc(char c); // Let's comment this out for C use for now to avoid
// confusion

#endif  // UART_H