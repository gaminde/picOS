#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

typedef struct {
    uint64_t x[31];  // x0-x30
    uint64_t spsr_el1;
    uint64_t elr_el1;
} trap_frame_t;

// C handler for synchronous exceptions
void c_sync_handler(trap_frame_t *frame);

// C handler for IRQ exceptions
void c_irq_handler(trap_frame_t *frame);  // New declaration

#endif  // EXCEPTIONS_H