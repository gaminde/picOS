#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

// Structure to hold the saved processor state during an exception.
// This must match the order and number of registers saved by the
// 'save_all_regs' macro in vectors.s.
typedef struct {
    // General-purpose registers x0-x29, then lr (x30)
    uint64_t x0;
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
    uint64_t x4;
    uint64_t x5;
    uint64_t x6;
    uint64_t x7;
    uint64_t x8;
    uint64_t x9;
    uint64_t x10;
    uint64_t x11;
    uint64_t x12;
    uint64_t x13;
    uint64_t x14;
    uint64_t x15;
    uint64_t x16;
    uint64_t x17;
    uint64_t x18;
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t x29;
    uint64_t lr;  // x30 (Link Register)

    // Note: SPSR_EL1 and ELR_EL1 are not part of this struct as saved by
    // the current 'save_all_regs' macro. They are handled separately
    // (e.g., passed as direct arguments to c_sync_handler).
    // If you modify 'save_all_regs' to push them, add them here in the correct
    // order. uint64_t spsr_el1; uint64_t elr_el1;
} context_state_t;

// Function declarations
void exceptions_init(void);  // Initializes exception handling (e.g., VBAR_EL1)
void enable_interrupts(void);  // Enables IRQs (e.g., msr daifclr, #2)
void disable_interrupts(
    void);  // Disables IRQs (e.g., msr daifset, #2) - if you have it

// C handlers for exceptions (called from assembly)
void c_sync_handler(uint64_t esr, uint64_t elr, context_state_t *ctx);
void c_irq_handler(context_state_t *ctx);
void c_serror_handler(uint64_t esr, uint64_t elr);

#endif  // EXCEPTIONS_H