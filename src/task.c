#include "task.h"

#include "common_macros.h"
#include "exceptions.h"  // For context_state_t to know its size/layout for stack setup
#include "kernel.h"  // For disable_interrupts/enable_interrupts if needed for critical sections
#include "string.h"  // For simple_memset or a real memset
#include "uart.h"

// Define global task management variables from task.h
tcb_t task_table[MAX_TASKS];
tcb_t *current_task = NULL;
uint32_t next_pid = 0;
tcb_t *ready_queue_head = NULL;
tcb_t *idle_task_tcb = NULL;

// Statically allocated stacks for simplicity
static uint8_t task_stacks[MAX_TASKS][TASK_STACK_SIZE]
    __attribute__((aligned(16)));
// static uint32_t next_stack_idx = 0; // This is now managed by
// task_stacks_status
uint8_t
    task_stacks_status[MAX_TASKS];  // 0 for free, 1 for used. Define it here.

// A simple memset (if not available from a standard library equivalent)
void simple_memset(void *ptr, int value, uint64_t num) {
    unsigned char *p = ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
}

void task_init_system(void) {
    uart_puts("Initializing Tasking System...\n");
    simple_memset(task_table, 0, sizeof(task_table));  // Zero out the TCB table

    for (int i = 0; i < MAX_TASKS; ++i) {
        task_table[i].state = TASK_UNUSED;
        task_table[i].pid = (uint32_t)-1;  // Indicate invalid/unused PID
        task_table[i].stack_base = NULL;   // Will be assigned in task_create
        task_table[i].stack_size = TASK_STACK_SIZE;
        task_table[i].next_in_queue = NULL;
        task_table[i].page_table_base = NULL;  // Initialize placeholder
    }
    current_task = NULL;  // No task is running initially
    ready_queue_head = NULL;
    next_pid = 0;
    // next_stack_idx = 0; // Not needed if using task_stacks_status
    simple_memset(task_stacks_status, 0,
                  sizeof(task_stacks_status));  // Initialize all stacks as free

    // The kernel itself runs in an implicit "task 0" context before scheduling
    // starts. We might create an explicit "idle" task later.
    uart_puts("Tasking System Initialized.\n");
}

// Function to allocate a stack from the static pool
// Returns stack index on success, -1 on failure.
static int allocate_static_stack(void) {  // Changed return type to int
    for (int i = 0; i < MAX_TASKS; ++i) {
        if (task_stacks_status[i] == 0) {
            task_stacks_status[i] = 1;  // Mark as used
            uart_puts("Allocated stack at index: ");
            print_uint(i);
            uart_puts("\n");
            return i;  // Return the index
        }
    }
    uart_puts("Error: No more static stacks available!\n");
    return -1;  // No stack available
}

// Create a new task
// entry_point: function pointer for the task to start execution.
// arg: argument to be passed to the entry_point function (in x0).
// name: a string name for the task (optional, for debugging).
// Returns PID on success, -1 on failure.
int task_create(void (*entry_point)(void *arg), void *arg, const char *name) {
    // disable_interrupts(); // Protect critical sections for finding TCB and
    // stack

    tcb_t *new_tcb = NULL;
    int i;
    for (i = 0; i < MAX_TASKS; ++i) {
        if (task_table[i].state == TASK_UNUSED) {
            new_tcb = &task_table[i];
            break;
        }
    }

    if (!new_tcb) {
        uart_puts("Error: No free TCBs available!\n");
        // enable_interrupts();
        return -1;  // No free TCBs
    }

    int stack_idx = allocate_static_stack();  // Get stack index
    if (stack_idx < 0) {                      // Check for failure
        uart_puts("Error: Failed to allocate stack for new task!\n");
        new_tcb->state = TASK_UNUSED;  // Release TCB if stack allocation failed
        // enable_interrupts();
        return -1;
    }
    uint8_t *stack_memory =
        task_stacks[stack_idx];  // Get stack pointer from index

    // Initialize the TCB
    new_tcb->pid =
        next_pid++;  // Assuming PIDs are assigned sequentially and might not
                     // match task_table index directly If PID is meant to be
                     // the index, then new_tcb->pid = i;
    new_tcb->state = TASK_READY;
    new_tcb->stack_base = (uint64_t *)stack_memory;
    new_tcb->stack_size = TASK_STACK_SIZE;
    new_tcb->stack_idx = (uint8_t)stack_idx;  // Store the allocated stack index

    // Now, set up the initial stack frame for the new task.
    // The stack grows downwards. The "top" of the stack is at the highest
    // address. kernel_sp will point to the "bottom" of the saved
    // context_state_t structure (i.e., the lowest address of the context data
    // on the stack).

    // Point to the highest address of the stack memory.
    uint64_t stack_top = (uint64_t)stack_memory + TASK_STACK_SIZE;

    // Align the stack pointer to 16 bytes, context_state_t will be pushed below
    // this. The actual SP for the task when it starts will be stack_top -
    // sizeof(context_state_t). The context_state_t itself must be 16-byte
    // aligned if SP must be. The `stp` and `ldp` instructions used for context
    // saving/restoring require 16-byte alignment for SP. So, the address where
    // context_state_t begins (new_tcb->kernel_sp) must be 16-byte aligned.

    // Calculate the starting address for context_state_t on the stack
    // Ensure it's aligned. sizeof(context_state_t) is (31 GPRs + SPSR + ELR) *
    // 8 bytes = 33 * 8 = 264 bytes. 264 is not 16-byte aligned. The
    // save_gprs_lr macro pushes 256 bytes, then SPSR/ELR push 16 bytes. Total
    // context size on stack = 256 (GPRs) + 16 (SPSR/ELR) = 272 bytes. This IS
    // 16-byte aligned.
    uint64_t context_frame_start = (stack_top - sizeof(context_state_t)) &
                                   ~0xFLL;  // Align down to 16 bytes

    new_tcb->kernel_sp = context_frame_start;

    // Get a pointer to where the context_state_t structure will live on the new
    // task's stack.
    context_state_t *ctx = (context_state_t *)new_tcb->kernel_sp;

    // Zero out the entire context state area initially
    simple_memset(ctx, 0, sizeof(context_state_t));

    // Set up the crucial registers for the new task to start
    ctx->elr_el1 = (uint64_t)
        entry_point;  // Address to return to (start of the task function)

    // SPSR_EL1: EL1h (using SP_EL1), No FIQ/IRQ/SError masked (0x00000000)
    // DAIF bits: D=0, A=0, I=0, F=0. Mode EL1h = 0b0101.
    // SPSR_EL1 = 0x0000000000000005 (for EL1h, all interrupts unmasked)
    // Or, to start with interrupts masked: SPSR_EL1 = 0x00000000000003C5 (DAIF
    // all set, EL1h) Let's start with interrupts enabled for EL1 tasks.
    ctx->spsr_el1 =
        0x0000000000000005;  // EL1h, DAIF all clear (interrupts enabled)

    // Pass the argument to the task's entry function via x0
    ctx->x0 = (uint64_t)arg;

    // Other GPRs (x1-x29, lr) are already zeroed by simple_memset.
    // lr (x30) could be set to a specific exit point if desired, but for now, 0
    // is fine. If the task function returns, it will return to address 0, which
    // will fault. A proper task should loop or call a task_exit() function.

    uart_puts("Task created: PID ");
    print_uint(new_tcb->pid);
    uart_puts(", Entry: 0x");
    print_hex((uint64_t)entry_point);
    uart_puts(", Stack Base: 0x");
    print_hex((uint64_t)new_tcb->stack_base);
    uart_puts(", Initial SP (kernel_sp): 0x");
    print_hex(new_tcb->kernel_sp);
    uart_puts("\n");
    uart_puts("  Initial ELR_EL1: 0x");
    print_hex(ctx->elr_el1);
    uart_puts(", SPSR_EL1: 0x");
    print_hex(ctx->spsr_el1);
    uart_puts(", X0 (arg): 0x");
    print_hex(ctx->x0);
    uart_puts("\n");

    add_to_ready_queue(new_tcb);

    // enable_interrupts(); // Restore interrupts if disabled at the start
    return new_tcb->pid;
}

void task_exit(void) {
    if (current_task &&
        current_task != idle_task_tcb) {  // Idle task should not exit
        uart_puts("Task PID ");
        print_uint(current_task->pid);
        uart_puts(" calling task_exit(). Setting state to ZOMBIE.\n");
        current_task->state = TASK_ZOMBIE;

        // The task will now spin here. The next timer interrupt will trigger
        // the scheduler. The scheduler will see its ZOMBIE state, clean it up,
        // and pick another task. This task will not run again.
        while (1) {
            __asm__ __volatile__(
                "wfi");  // Wait for interrupt to be descheduled
        }
    } else if (current_task == idle_task_tcb) {
        uart_puts("Error: Idle task attempted to exit!\n");
        // Idle task should loop forever.
    } else {
        uart_puts(
            "Error: task_exit() called with no current_task or invalid "
            "task!\n");
    }
}

// Add a task to the end of the ready queue (FIFO)
void add_to_ready_queue(tcb_t *task) {
    if (!task) {
        uart_puts("Error: Tried to add NULL task to ready queue.\n");
        return;
    }
    task->next_in_queue = NULL;  // Ensure it's the new tail

    // uart_puts("add_to_ready_queue: Adding PID "); print_uint(task->pid);
    // uart_puts("\n");

    if (!ready_queue_head) {
        // Queue was empty
        ready_queue_head = task;
    } else {
        // Find the end of the queue
        tcb_t *current = ready_queue_head;
        while (current->next_in_queue != NULL) {
            current = current->next_in_queue;
        }
        current->next_in_queue = task;
    }
}

// Get the next task from the head of the ready queue (FIFO)
tcb_t *get_next_ready_task(void) {
    if (!ready_queue_head) {
        // uart_puts("get_next_ready_task: Ready queue is empty.\n");
        return NULL;  // No tasks ready
    }

    tcb_t *task_to_run = ready_queue_head;
    ready_queue_head = ready_queue_head->next_in_queue;  // Dequeue

    task_to_run->next_in_queue = NULL;  // Isolate the dequeued task

    // uart_puts("get_next_ready_task: Got PID "); print_uint(task_to_run->pid);
    // uart_puts("\n");
    return task_to_run;
}

// The scheduler.
// Called from an interrupt context (e.g., timer IRQ).
// current_task_sp_val: The value of SP for the task that was just interrupted,
//                      pointing to its saved context_state_t.
// Returns: The kernel_sp of the next task to run.
uint64_t schedule(uint64_t current_task_sp_val) {
    tcb_t *previous_task = current_task;

    // Handle ZOMBIE task cleanup first
    if (previous_task != NULL && previous_task->state == TASK_ZOMBIE &&
        previous_task != idle_task_tcb) {
        uart_puts("Scheduler: Cleaning up ZOMBIE task PID ");
        print_uint(previous_task->pid);
        uart_puts(".\n");

        // Mark TCB as unused (assuming PID is the index in task_table)
        // If PIDs are not direct indices, you'd need to find the TCB in
        // task_table by PID.
        task_table[previous_task->pid].state = TASK_UNUSED;

        // Mark stack as free
        if (previous_task->stack_idx < MAX_TASKS) {  // Basic bounds check
            task_stacks_status[previous_task->stack_idx] = 0;
        } else {
            uart_puts("Scheduler: Invalid stack_idx for zombie task PID ");
            print_uint(previous_task->pid);
            uart_puts("\n");
        }

        // Optionally: Clear other TCB fields, add PID to a free PID list for
        // reuse. For now, we just mark as UNUSED.

        if (current_task == previous_task) {
            current_task = NULL;  // This task is gone.
        }
        previous_task = NULL;  // Don't process this zombie task further (for
                               // saving SP or re-queuing).
    }

    // Save context of the (non-zombie) previously running task
    if (previous_task != NULL && previous_task != idle_task_tcb) {
        previous_task->kernel_sp = current_task_sp_val;
        if (previous_task->state ==
            TASK_RUNNING) {  // Only re-queue if it was running and not a zombie
            previous_task->state = TASK_READY;
            add_to_ready_queue(previous_task);
        }
    } else if (previous_task == idle_task_tcb) {
        idle_task_tcb->kernel_sp = current_task_sp_val;
    } else if (previous_task == NULL && current_task != NULL) {
        // This case might occur if current_task was set to NULL due to zombie
        // cleanup and there was no *other* previous_task. Essentially, the
        // initial state or post-zombie state.
    }

    tcb_t *next_task = get_next_ready_task();

    if (next_task == NULL) {  // Ready queue is empty
        if (!idle_task_tcb) {
            uart_puts("FATAL: Idle task TCB is NULL! Halting.\n");
            while (1) __asm__ __volatile__("wfi");
        }
        current_task = idle_task_tcb;
    } else {
        current_task = next_task;
    }

    if (current_task != NULL) {
        current_task->state = TASK_RUNNING;
        return current_task->kernel_sp;
    } else {
        // This should only be reached if idle_task_tcb was somehow NULL and
        // ready queue was empty.
        uart_puts(
            "FATAL: current_task is NULL after scheduling decision! Returning "
            "original SP.\n");
        // This might return to kernel_main's WFI or whatever was running before
        // the interrupt.
        return current_task_sp_val;
    }
}  // This should be the only closing brace for the function.