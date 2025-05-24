#include "exceptions.h"

#include "common_macros.h"  // For INTERRUPT_ID_CNTPNSIRQ, etc.
#include "gic.h"
#include "kernel.h"  // For enable_interrupts, disable_interrupts
#include "task.h"    // For schedule()
#include "timer.h"
#include "uart.h"

// ... other code like g_tick_count ...

// Synchronous exception handler
// ESR_EL1 contains the reason for the exception.
// ctx points to the saved context on the stack, which includes ELR_EL1.
void c_sync_handler(uint64_t esr_el1, context_state_t *ctx) {
    disable_interrupts();  // Should be safe to call, or ensure it's idempotent
    uart_puts("\n--- Synchronous Exception Caught ---\n");
    uart_puts("ESR_EL1: 0x");
    print_hex(esr_el1);
    uart_puts("\nELR_EL1 (from context): 0x");
    print_hex(ctx->elr_el1);  // Access ELR_EL1 from the context structure
    uart_puts("\nSPSR_EL1 (from context): 0x");
    print_hex(ctx->spsr_el1);
    uart_puts("\n");

    // Decode ESR_EL1
    uint8_t ec = (esr_el1 >> 26) & 0x3F;  // Exception Class
    // uint32_t iss = esr_el1 & 0x1FFFFFF;  // Instruction Specific Syndrome

    uart_puts("Exception Class (EC): 0x");
    print_hex(ec);

    switch (ec) {
        case 0b010101:  // SVC instruction execution in AArch64 state
            uart_puts(" (SVC instruction)\n");
            // Handle SVC: extract SVC number from ISS, call appropriate
            // handler. For now, just print and proceed. The ELR_EL1 in ctx
            // points to the SVC instruction itself. To "return" past it,
            // ELR_EL1 should be incremented by 4 before eret. ctx->elr_el1 +=
            // 4; // Example: if SVC is handled and we want to resume
            break;
        case 0b100100:  // Data Abort from lower Exception level (e.g., EL0
                        // trying to access restricted EL1 memory)
        case 0b100101:  // Data Abort from same Exception level (e.g., EL1 bad
                        // memory access)
            uart_puts(" (Data Abort)\n");
            // Read FAR_EL1 for faulting address
            uint64_t far_el1;
            __asm__ __volatile__("mrs %0, far_el1" : "=r"(far_el1));
            uart_puts("FAR_EL1 (Fault Address Register): 0x");
            print_hex(far_el1);
            uart_puts("\n");
            break;
        case 0b100000:  // Instruction Abort from lower Exception level
        case 0b100001:  // Instruction Abort from same Exception level
            uart_puts(" (Instruction Abort)\n");
            uint64_t ifar_el1;  // Instruction Fault Address Register (alias for
                                // FAR_EL1 for Ins Aborts)
            __asm__ __volatile__(
                "mrs %0, far_el1"
                : "=r"(ifar_el1));  // FAR_EL1 is used for Inst Aborts too
            uart_puts("FAR_EL1 (Instruction Fault Address): 0x");
            print_hex(ifar_el1);
            uart_puts("\n");
            break;
        default:
            uart_puts(" (Unhandled EC)\n");
            break;
    }

    uart_puts("------------------------------------\n");
    // For critical unhandled synchronous exceptions, we might halt.
    // For now, we allow it to attempt to return via eret in vectors.s
    // If it was an SVC that the scheduler handled, eret will go to the new
    // context. If it was a fault, eret will likely re-trigger the fault if
    // elr_el1 isn't advanced. enable_interrupts(); // Re-enable if it's safe to
    // continue
}

void minimal_fiq_print(void) {
    uart_puts("\n--- FIQ Exception Caught (Minimal Handler) ---\n");
    // Loop forever or handle
    while (1);
}

void minimal_serror_print(void) {
    uart_puts("\n--- SError Exception Caught (Minimal Handler) ---\n");
    // Loop forever or handle
    while (1);
}

extern char
    _exception_vector_table[];  // Declare the symbol from linker/vectors.s

void exceptions_init(void) {
    uint64_t vector_table_addr =
        (uint64_t)_exception_vector_table;  // Use the symbol directly
    __asm__ __volatile__("msr vbar_el1, %0" : : "r"(vector_table_addr));

    uart_puts("Address of _exception_vector_table: 0x");
    print_hex(vector_table_addr);
    uart_puts("\n");

    uint64_t vbar_check;
    __asm__ __volatile__("mrs %0, vbar_el1" : "=r"(vbar_check));
    uart_puts("VBAR_EL1 set to: 0x");
    print_hex(vbar_check);
    uart_puts("\n");
}

// IRQ handler
// ctx points to the saved context_state_t on the stack of the interrupted
// execution. Returns the stack pointer (kernel_sp) of the next task to run.
uint64_t c_irq_handler(context_state_t *ctx) {
    uint64_t next_task_sp = (uint64_t)ctx;  // Default to current context SP

    uint32_t irq_id = gic_read_iar();

    if (irq_id == INTERRUPT_ID_CNTPNSIRQ) {
        handle_timer_irq();  // This will re-arm the timer
        // Timer interrupt is a good place to call the scheduler for preemption
        next_task_sp = schedule((uint64_t)ctx);
    } else if (irq_id < 1020) {
        uart_puts("Unhandled IRQ ID: ");
        print_uint(irq_id);
        uart_puts("\n");
    } else {
        uart_puts("Spurious or special IRQ ID: ");
        print_uint(irq_id);
        uart_puts("\n");
    }

    gic_write_eoir(irq_id);
    return next_task_sp;  // Return SP of the task to switch to (or current if
                          // no switch)
}