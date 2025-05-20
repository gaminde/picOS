#include <kernel.h>  // For the kernel_main declaration
#include <uart.h>    // For uart_puts

// This is the definition
void kernel_main(void) {
    uart_puts("Hello from Kernel (C) - New Structure!\n");

    // Loop forever
    while (1) {
        // Future kernel tasks here
    }
}