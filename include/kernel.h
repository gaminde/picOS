// filepath: include/kernel.h
#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>  // For uint64_t etc.

// Function prototypes
void kernel_main(void);

// Architecture specific interrupt control
extern void enable_interrupts(void);
extern void disable_interrupts(void);

#endif  // KERNEL_H