#include "task.h"

#include "uart.h"  // For uart_puts for debugging
// #include "utils.h"  // For simple_memset (or your equivalent)

// Define the global task table
tcb_t task_table[MAX_TASKS];

// Define the pointer to the currently running task's TCB
// Initially, no task is running, so current_task is NULL.
// The first task will be set by the scheduler or initial task setup.
tcb_t *current_task = NULL;

// Define the next available PID
// PIDs usually start from 1. PID 0 can be reserved for an idle task or kernel
// itself.
uint32_t next_pid = 1;

// Statically allocated stack space for all tasks.
// Each task will get a slice of this.
// Ensure this is aligned, e.g., 16-byte boundary as AArch64 SP must be 16-byte
// aligned.
static uint8_t task_stacks[MAX_TASKS][TASK_STACK_SIZE]
    __attribute__((aligned(16)));

// A simple memset implementation if you don't have one from a library
// If you have utils.h with simple_memset, you can remove this.
#ifndef UTILS_H_WITH_MEMSET  // Conditional compilation guard
void simple_memset(void *s, int c, uint64_t n) {
    unsigned char *p = (unsigned char *)s;
    for (uint64_t i = 0; i < n; ++i) {
        p[i] = (unsigned char)c;
    }
}
#endif

// Initialize the tasking system
void task_init_system(void) {
    uart_puts("Initializing Tasking System...\n");

    // Initialize all TCBs in the task_table
    for (int i = 0; i < MAX_TASKS; i++) {
        task_table[i].pid = 0;  // Mark as invalid PID initially
        task_table[i].state = TASK_UNUSED;
        task_table[i].kernel_sp = 0;
        task_table[i].stack =
            (uint64_t *)task_stacks[i];  // Assign pre-allocated stack base

        // Optional: Zero out the stack memory for cleanliness and to catch
        // stack overflows more easily This can be slow for many large stacks,
        // but good for debugging.
        simple_memset(task_stacks[i], 0x00,
                      TASK_STACK_SIZE);  // Or use your own memset
    }

    // current_task is still NULL. It will be set when the first task is
    // scheduled to run. next_pid is initialized to 1.

    uart_puts("Tasking System Initialized.\n");
}