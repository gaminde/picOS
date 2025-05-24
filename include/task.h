#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#include "exceptions.h"  // For context_state_t

// Define NULL if not already defined (common in freestanding environments)
#ifndef NULL
#define NULL ((void *)0)
#endif

// Maximum number of tasks
#define MAX_TASKS 10  // Example value, can be adjusted
// Stack size for each task (e.g., 4KB)
#define TASK_STACK_SIZE (4 * 1024)

// Task states
typedef enum {
    TASK_UNUSED,   // TCB is not in use
    TASK_READY,    // Task is ready to run
    TASK_RUNNING,  // Task is currently running
    TASK_BLOCKED,  // Task is waiting for an event (not used initially)
    TASK_ZOMBIE    // Task has finished execution (not used initially)
} task_state_t;

// Task Control Block (TCB) structure
typedef struct {
    uint64_t kernel_sp;  // Kernel stack pointer: points to the saved
                         // context_state_t on the task's stack This is the
                         // value of SP_EL1 when the task is switched out.
    uint32_t pid;        // Task ID
    task_state_t state;  // Current state of the task
    uint64_t *
        stack;  // Pointer to the base of the allocated stack memory for this
                // task. The stack grows downwards from stack + TASK_STACK_SIZE.
    // Add more fields later as needed, e.g., priority, page_table_base, etc.
} tcb_t;

// Global array of TCBs (or this could be managed dynamically)
extern tcb_t task_table[MAX_TASKS];
// Pointer to the TCB of the currently running task
extern tcb_t *current_task;
// Counter for next available PID
extern uint32_t next_pid;

// Function declarations (to be implemented in a .c file, e.g., task.c or
// scheduler.c)
void task_init_system(void);  // Initialize the tasking system (e.g., TCB array)
// int task_create(void (*entry_point)(void)); // Create a new task
// void schedule(void); // The scheduler function

#endif  // TASK_H