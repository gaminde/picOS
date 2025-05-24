#ifndef TASK_H
#define TASK_H

#include <stdint.h>

#include "common_macros.h"  // <<< ENSURE THIS IS HERE, AT THE TOP

// Define task states
typedef enum {
    TASK_UNUSED,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_ZOMBIE
} task_state_e;

// Task Control Block (TCB) structure
typedef struct tcb {
    uint32_t pid;
    task_state_e state;
    uint64_t kernel_sp;
    uint64_t *stack_base;
    uint32_t stack_size;
    uint8_t stack_idx;
    void (*entry_point)(void *);
    void *arg;
    struct tcb *next_in_queue;
    void *page_table_base;
} tcb_t;

// Global task management variables (declared as extern here)
extern tcb_t
    task_table[MAX_TASKS];  // MAX_TASKS needs to be defined before this line
extern tcb_t *current_task;
extern uint32_t next_pid;
extern tcb_t *ready_queue_head;
extern tcb_t *idle_task_tcb;
extern uint8_t task_stacks_status[MAX_TASKS];  // MAX_TASKS needs to be defined
                                               // before this line

// Function declarations
void task_init_system(void);
int task_create(void (*entry_point)(void *arg), void *arg, const char *name);
uint64_t schedule(uint64_t current_task_sp_val);
void add_to_ready_queue(tcb_t *task);
tcb_t *get_next_ready_task(void);
void task_exit(void);

#endif  // TASK_H