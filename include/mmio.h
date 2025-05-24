#ifndef MMIO_H
#define MMIO_H

#include <stdint.h>

static inline void mmio_write(uintptr_t reg, uint32_t data) {
    *(volatile uint32_t*)reg = data;
    // Consider a dmb sy or dsb sy here if strict ordering is needed for all
    // MMIO, though often it's handled more specifically where required.
}

static inline uint32_t mmio_read(uintptr_t reg) {
    return *(volatile uint32_t*)reg;
}

#endif  // MMIO_H