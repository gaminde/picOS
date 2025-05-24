#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define KERNEL_TIMER_INTERVAL_MS \
    10  // Default kernel timer interval in milliseconds
#define TIMER_IRQ_ID \
    30  // IRQ ID for nCNTPNSIRQ (EL1 Physical Timer) // <<< ADD OR ENSURE THIS
        // LINE EXISTS

// Function to initialize the EL1 Physical Timer
void timer_init(uint32_t interval_ms);

// Function to handle the timer interrupt
// This is the C part, called from the main IRQ handler
void handle_timer_irq(void);

// Functions to access timer registers (if needed externally, otherwise keep
// static in timer.c)
uint64_t read_cntp_ctl_el0(void);
void write_cntp_tval_el0(uint64_t val);

#endif  // TIMER_H