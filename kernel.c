// Declare the assembly function we want to call from boot.s
// C calling convention: first argument (const char *s) is in register x0
extern void uart_puts(const char *s);

// Kernel entry point, called from boot.s
void kernel_main(void) {
    const char *kernel_msg = "Hello from Kernel (C)\n";
    uart_puts(kernel_msg);

    // Infinite loop to prevent returning to the bootloader
    while (1) {
        // Kernel idle loop or future scheduler point
    }
}