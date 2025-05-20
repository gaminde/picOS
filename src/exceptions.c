#include <exceptions.h>
#include <stdint.h>  // For uint64_t
#include <uart.h>    // For uart_puts

// A helper function to print hex values, useful for debugging
void print_hex(uint64_t val) {
    char hex_chars[] = "0123456789abcdef";
    char buffer[19] = "0x";  // "0x" + 16 hex digits + null terminator
    int i;
    int current_char = 2;

    if (val == 0) {
        uart_puts("0x0");
        return;
    }

    for (i = 60; i >= 0; i -= 4) {
        uint64_t nibble = (val >> i) & 0xF;
        if (nibble != 0 || current_char > 2 ||
            i == 0) {  // Print leading zeros after first non-zero, or if it's
                       // the last digit
            buffer[current_char++] = hex_chars[nibble];
        }
    }
    buffer[current_char] = '\0';
    uart_puts(buffer);
}

void c_sync_handler(trap_frame_t *frame) {
    uint64_t esr_el1;
    // Read ESR_EL1 to find out the cause of the exception
    asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));

    uart_puts("C Synchronous Handler Reached!\n");
    uart_puts("  SPSR_EL1: ");
    print_hex(frame->spsr_el1);
    uart_puts("\n");
    uart_puts("  ELR_EL1:  ");
    print_hex(frame->elr_el1);
    uart_puts("\n");
    uart_puts("  ESR_EL1:  ");
    print_hex(esr_el1);
    uart_puts("\n");

    // Decode ESR_EL1 (Exception Class and ISS)
    uint32_t ec = (esr_el1 >> 26) & 0x3F;  // Exception Class, bits 31:26
    // uint32_t iss = esr_el1 & 0x1FFFFFF;  // Instruction Specific Syndrome,
    // bits 24:0

    uart_puts("  Exception Class (EC): ");
    print_hex(ec);
    uart_puts("\n");

    if (ec == 0x15) {  // 0x15 is SVC instruction execution in AArch64 state
        uart_puts("  Reason: SVC Instruction\n");
        // uint16_t imm = iss & 0xFFFF; // For SVC, ISS bits 15:0 can be an
        // immediate uart_puts("  SVC Immediate: "); print_hex(imm);
        // uart_puts("\n");
    } else {
        uart_puts("  Reason: Other synchronous exception\n");
    }

    // For now, hang here. In a real OS, you might terminate the process or take
    // other action.
    uart_puts("Halting in C handler.\n");
    while (1);
}

void c_irq_handler(trap_frame_t *frame) {
    // Suppress unused parameter warning for now, as frame isn't used yet.
    (void)frame;

    uart_puts("C IRQ Handler Reached!\n");
    // In a real IRQ handler, you would:
    // 1. Identify the source of the interrupt (e.g., by reading GIC's IAR).
    // 2. Handle the interrupt (e.g., service the timer, process device input).
    // 3. Acknowledge the interrupt (e.g., by writing to GIC's EOIR).
    // For now, since no IRQs are enabled, this code won't be hit unless
    // something unexpected happens or we explicitly trigger one later for
    // testing.

    uart_puts("Halting in C IRQ handler (unexpected).\n");
    while (1);
}