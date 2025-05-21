#include <exceptions.h>
#include <gic.h>
#include <stdint.h>
#include <timer.h>
#include <uart.h>

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
            i == 0)  // Print leading zeros after first non-zero, or if it's
                     // the last digit
            buffer[current_char++] = hex_chars[nibble];
    }
    buffer[current_char] = '\0';
    uart_puts(buffer);
}

// Helper function to print an unsigned 64-bit integer in decimal
// Note: This is a simple implementation. For very large numbers, it might be
// slow or could overflow the buffer if not careful. Max uint64_t is 20 digits.
void print_uint(uint64_t val) {
    char buffer[21];  // Max 20 digits for uint64_t + null terminator
    int i = 20;
    buffer[i--] = '\0';

    if (val == 0) {
        uart_puts("0");
        return;
    }

    while (val > 0 && i >= 0) {
        buffer[i--] = (val % 10) + '0';
        val /= 10;
    }
    uart_puts(&buffer[i + 1]);
}

void c_sync_handler(trap_frame_t *frame) {
    uint64_t esr_el1;
    // Read ESR_EL1 to find out the cause of the exception
    // Use __asm__ instead of asm
    __asm__ volatile("mrs %0, esr_el1" : "=r"(esr_el1));

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

static volatile uint64_t tick_counter = 0;  // Our new global tick counter

void c_irq_handler(trap_frame_t *frame) {
    (void)frame;

    uint32_t irq_id = gic_read_iar();

    if (irq_id == 1023) {
        uart_puts("Spurious IRQ received (ID 1023).\n");
        gic_write_eoir(irq_id);
        return;
    }

    if (irq_id == TIMER_IRQ_ID) {
        handle_timer_irq();
        tick_counter++;
        uart_puts("Timer Tick: ");
        print_uint(tick_counter);
        uart_puts("\n");
    } else {
        uart_puts("Unknown IRQ received! ID: ");
        print_uint(irq_id);  // Use print_uint for the unknown ID
        uart_puts("\nHalting.\n");
        while (1);
    }

    gic_write_eoir(irq_id);
}