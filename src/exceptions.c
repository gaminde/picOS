#include "exceptions.h"

#include "common_macros.h"
#include "gic.h"
#include "timer.h"
#include "uart.h"  // <<< ADD THIS LINE

// ... (context_state_t definition if it's here, or include where it is defined)
// ...

extern void *_exception_vector_table;  // Defined in vectors.s

static volatile uint64_t tick_counter = 0;

void exceptions_init(void) {
    uart_puts("Address of _exception_vector_table: 0x");
    print_hex((uintptr_t)&_exception_vector_table);
    uart_puts("\n");
    __asm__ __volatile__("msr vbar_el1, %0" : : "r"(&_exception_vector_table));
    uint64_t vbar_val;
    __asm__ __volatile__("mrs %0, vbar_el1" : "=r"(vbar_val));
    uart_puts("VBAR_EL1 set to: 0x");
    print_hex(vbar_val);
    uart_puts("\n");
}

// Synchronous exception handler
// Called from assembly with: x0=esr_el1, x1=elr_el1, x2=ctx_ptr (sp)
void c_sync_handler(uint64_t esr_el1, uint64_t elr_el1, context_state_t *ctx) {
    uart_puts("\n--- Synchronous Exception ---\n");
    uart_puts("ESR_EL1: 0x");
    print_hex(esr_el1);
    uart_puts("\n");
    uart_puts("ELR_EL1: 0x");
    print_hex(elr_el1);
    uart_puts("\n");

    uint32_t ec = (esr_el1 >> 26) & 0x3F;  // Extract Exception Class (EC)
    uart_puts("Exception Class (EC): 0x");
    print_hex(ec);
    // You can add a switch statement here to decode common EC values
    switch (ec) {
        case 0b010101:  // SVC instruction execution in AArch64 state
            uart_puts(" (SVC instruction)\n");
            // uint16_t imm = esr_el1 & 0xFFFF; // Extract immediate value for
            // SVC uart_puts("SVC immediate: "); print_uint(imm);
            // uart_puts("\n");
            break;
        case 0b100100:  // Data Abort from lower Exception level
        case 0b100101:  // Data Abort from same Exception level
            uart_puts(" (Data Abort)\n");
            // uint64_t far_el1;
            // __asm__ __volatile__("mrs %0, far_el1" : "=r"(far_el1));
            // uart_puts("FAR_EL1: 0x"); print_hex(far_el1); uart_puts("\n");
            break;
        // Add more cases as needed
        default:
            uart_puts(" (Unknown EC)\n");
            break;
    }

    uart_puts("System Halted.\n");
    while (1);  // Halt
}

void c_irq_handler(context_state_t *ctx) {
    uint32_t iar_val = gic_read_iar();
    uint32_t actual_irq_id = iar_val & 0x3FF;

    if (actual_irq_id == 1023) {
        // uart_puts("Spurious IRQ (ID 1023) received.\n");  // Optional: can be
        // noisy
        return;
    }

    if (actual_irq_id == TIMER_IRQ_ID) {
        handle_timer_irq();
        tick_counter++;
        if ((tick_counter % 100) ==
            0) {  // Print every 100 ticks (1 second if 10ms interval)
            uart_puts("Timer Tick: ");
            print_uint(tick_counter);
            uart_puts("\n");
        }
    } else {
        uart_puts("Unknown IRQ received! ID: ");
        print_uint(actual_irq_id);
        uart_puts("\nHalting due to unknown IRQ.\n");
        gic_write_eoir(iar_val);
        while (1);
    }
    gic_write_eoir(iar_val);
}

// void minimal_irq_print(void) { uart_puts("minimal_irq_print called!\n"); } //
// <<< REMOVE UNUSED
void minimal_fiq_print(void) { uart_puts("minimal_fiq_print called!\n"); }
void minimal_serror_print(void) { uart_puts("minimal_serror_print called!\n"); }