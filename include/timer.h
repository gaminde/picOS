#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// ARM Generic Timer Registers (EL1 Physical Timer)
// Accessible via MRS/MSR instructions

// CNTP_CTL_EL0: EL1 Physical Timer Control Register
// Bit 0 (ENABLE):   Enable timer.
// Bit 1 (IMASK):    Interrupt mask. 0 = interrupt not masked.
// Bit 2 (ISTATUS):  Interrupt status. 1 = interrupt pending.
#define CNTP_CTL_EL0_ENABLE (1 << 0)
#define CNTP_CTL_EL0_IMASK (1 << 1)
#define CNTP_CTL_EL0_ISTATUS (1 << 2)

// Function to initialize and start the EL1 physical timer
// freq_hz: The desired interrupt frequency in Hz
void timer_init(uint32_t freq_hz);

// Function to handle a timer interrupt (to be called from the IRQ handler)
void handle_timer_irq(void);

#endif  // TIMER_H