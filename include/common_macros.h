#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

#define TIMER_IRQ_ID 30  // The IRQ ID for the EL1 Physical Timer (nCNTPNSIRQ)
#define KERNEL_TIMER_INTERVAL_MS \
    10  // Desired kernel timer tick interval in milliseconds

// Add other common constants or simple macros here if needed

#endif  // COMMON_MACROS_H