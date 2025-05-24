#ifndef TASK_H
#define TASK_H

#include <stdint.h>

// We will rely on the exception entry mechanism (vectors.s) to save
// ALL necessary context (GPRs x0-x30, SPSR_EL1, ELR_EL1) onto the stack.
// The 'kernel_sp' in the TCB will point to this complete context frame.
// Therefore, we don't need to include "exceptions.h" for context_state_t here
// in the TCB definition itself, as the TCB only stores the pointer.

// Define NULL if not already defined
#ifndef NULL
#define NULL ((void *)0)
#endif

// Maximum number of tasks
#define MAX_TASKS 10
// Stack size for each task
#define TASK_STACK_SIZE (4 * 1024)

// Task states
typedef enum {
    TASK_UNUSED,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_ZOMBIE
} task_state_t;

// Forward declaration for self-referential pointer
struct tcb_struct;

// Task Control Block (TCB) structure
typedef struct tcb_struct {
    uint64_t
        kernel_sp;  // Kernel stack pointer: points to the FULL saved
                    // context (GPRs, SPSR_EL1, ELR_EL1) on the task's stack.
    uint32_t pid;   // Task ID
    task_state_t state;    // Current state of the task
    uint64_t *stack_base;  // Pointer to the base of the allocated stack memory.
    uint32_t stack_size;   // Size of the allocated stack.

    void *page_table_base;  // For Phase 4 (MMU) - unused for now

    struct tcb_struct *next_in_queue;  // For linking in ready/wait queues
    // char name[32]; // Optional: task name for debugging
} tcb_t;

// Global array of TCBs
extern tcb_t task_table[MAX_TASKS];
// Pointer to the TCB of the currently running task
extern tcb_t *current_task;
// Counter for next available PID
extern uint32_t next_pid;
// Head of the ready task queue (simple linked list for now)
extern tcb_t *ready_queue_head;

// Function declarations
void task_init_system(void);
int task_create(void (*entry_point)(void *arg), void *arg, const char *name);
// Schedule returns the kernel_sp of the next task to run.
// It takes the kernel_sp of the currently interrupted task (or current
// context).
uint64_t schedule(uint64_t current_task_sp_val);
// Helper to add to ready queue
void add_to_ready_queue(tcb_t *task);
// Helper to get next ready task
tcb_t *get_next_ready_task(void);

#endif  // TASK_H