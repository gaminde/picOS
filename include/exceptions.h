#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

typedef struct {
    uint64_t x[31];  // x0-x30. x[0] is x0, x[30] is x30.
    uint64_t spsr_el1;
    uint64_t elr_el1;
    // The remaining part of the 272-byte frame is padding if not used.
} trap_frame_t;

void c_sync_handler(trap_frame_t *frame);

#endif  // EXCEPTIONS_H