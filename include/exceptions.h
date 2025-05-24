#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

// Structure to hold the saved processor state during an exception.
// This must match the order of registers pushed onto the stack by vectors.s
// The 'sp' passed to C handlers will point to the beginning of this structure.
typedef struct {
    // System registers saved first by the extended exception entry
    uint64_t spsr_el1;
    uint64_t elr_el1;

    // General-purpose registers x0-x29, then lr (x30)
    // This order must match the save_gprs_lr macro.
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
} context_state_t;

// Function declarations
void exceptions_init(void);  // Initializes exception handling (e.g., VBAR_EL1)
void enable_interrupts(void);  // Enables IRQs (e.g., msr daifclr, #2)
void disable_interrupts(
    void);  // Disables IRQs (e.g., msr daifset, #2) - if you have it

// C handlers for exceptions (called from assembly)
void c_sync_handler(uint64_t esr_el1,
                    context_state_t *ctx);     // Pass ESR_EL1 explicitly
uint64_t c_irq_handler(context_state_t *ctx);  // c_irq_handler now returns the
                                               // SP of the next task to run
void minimal_fiq_print(void);
void minimal_serror_print(void);

#endif  // EXCEPTIONS_H