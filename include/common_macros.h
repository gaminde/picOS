#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

// Define NULL if not already defined (though stdint.h or other system headers
// might provide it)
#ifndef NULL
#define NULL ((void *)0)
#endif

// Alignment directive for assembly
#define ALIGN_DIRECTIVE .align

// Timer related IRQ ID for EL1 Physical Timer (CNTPNSIRQ)
// For QEMU 'virt' machine, this is typically IRQ ID 30 (Secure Physical Timer)
// or IRQ ID 29 (Non-Secure Physical Timer).
// The Generic Timer framework uses PPIs (Private Peripheral Interrupts).
// IRQ ID 30 is CNTP_IRQ (EL1 Physical Timer Interrupt).
// IRQ ID 29 is CNTV_IRQ (EL1 Virtual Timer Interrupt).
// IRQ ID 27 is CNTHP_IRQ (EL2 Physical Timer Interrupt).
// We are using the EL1 Physical Timer, so ID 30.
#define INTERRUPT_ID_CNTPNSIRQ 30  // EL1 Physical Timer IRQ ID

// Other common macros can go here

#endif  // COMMON_MACROS_H