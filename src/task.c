#include "task.h"

#include "common_macros.h"
#include "exceptions.h"  // For context_state_t to know its size/layout for stack setup
#include "kernel.h"  // For disable_interrupts/enable_interrupts if needed for critical sections
#include "string.h"  // For simple_memset or a real memset
#include "uart.h"

// Define global task management variables from task.h
tcb_t task_table[MAX_TASKS];
tcb_t *current_task = NULL;
uint32_t next_pid = 0;  // Initialize PID counter
tcb_t *ready_queue_head = NULL;

// Statically allocated stacks for simplicity
static uint8_t task_stacks[MAX_TASKS][TASK_STACK_SIZE]
    __attribute__((aligned(16)));
static uint32_t next_stack_idx = 0;

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
    next_stack_idx = 0;

    // The kernel itself runs in an implicit "task 0" context before scheduling
    // starts. We might create an explicit "idle" task later.
    uart_puts("Tasking System Initialized.\n");
}

// Function to allocate a stack from the static pool
// Returns NULL if no stacks are available.
static uint8_t *allocate_static_stack(void) {
    // This critical section should ideally be protected if tasks could be
    // created concurrently For now, assuming task creation is serialized or
    // happens before scheduler starts fully. disable_interrupts(); // Example
    // of protection
    if (next_stack_idx < MAX_TASKS) {
        uint8_t *stack_ptr = task_stacks[next_stack_idx];
        next_stack_idx++;
        // enable_interrupts(); // Restore interrupts
        uart_puts("Allocated stack at index: ");
        print_uint(next_stack_idx - 1);
        uart_puts("\n");
        return stack_ptr;
    }
    // enable_interrupts(); // Restore interrupts
    uart_puts("Error: No more static stacks available!\n");
    return NULL;
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

    uint8_t *stack_memory = allocate_static_stack();
    if (!stack_memory) {
        uart_puts("Error: Failed to allocate stack for new task!\n");
        // enable_interrupts();
        return -1;  // No stack memory
    }

    // Initialize the TCB
    new_tcb->pid = next_pid++;
    new_tcb->state = TASK_READY;  // Set to ready, not running yet
    new_tcb->stack_base =
        (uint64_t *)stack_memory;  // Base of the allocated memory
    new_tcb->stack_size = TASK_STACK_SIZE;
    // new_tcb->name = name; // If you add a name field to TCB

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
    // uart_puts("Scheduler entered.\n"); // Keep this for debugging if you like

    tcb_t *previous_task = current_task;

    // 1. Save the context of the previously running task (if any)
    if (previous_task != NULL) {
        previous_task->kernel_sp = current_task_sp_val;
        if (previous_task->state ==
            TASK_RUNNING) {  // Only re-queue if it was running
            previous_task->state = TASK_READY;
            add_to_ready_queue(previous_task);  // Uses the real function now
            // uart_puts("Task PID "); print_uint(previous_task->pid);
            // uart_puts(" moved to ready queue.\n");
        }
    } else {
        // uart_puts("Scheduler: No previous task (current_task is NULL).
        // Initial schedule.\n");
    }

    // 2. Select the next task to run
    tcb_t *next_task = get_next_ready_task();  // Uses the real function now

    if (next_task == NULL) {
        // No tasks in the ready queue.
        // uart_puts("Scheduler: Ready queue is empty.\n");
        if (previous_task != NULL && previous_task->state != TASK_BLOCKED &&
            previous_task->state != TASK_ZOMBIE) {
            // If there was a previous task and it's still viable (e.g. not
            // blocked), let it run again. This handles the case of a single
            // task running. uart_puts("Scheduler: Resuming previous task PID
            // "); print_uint(previous_task->pid); uart_puts(" as no other tasks
            // are ready.\n");
            current_task = previous_task;
        } else {
            // Truly no task to run, and no viable previous task.
            // This is where an idle task would be essential.
            // For now, returning current_task_sp_val will continue
            // kernel_main's WFI loop or whatever was running before the
            // interrupt. uart_puts("Scheduler: No tasks to run and no viable
            // previous task! Returning original SP.\n");
            current_task = NULL;  // Explicitly no current task
            return current_task_sp_val;
        }
    } else {
        // A new task was found in the ready queue
        current_task = next_task;
    }

    // 3. Update current_task state (if a task is chosen)
    if (current_task != NULL) {
        current_task->state = TASK_RUNNING;
        // uart_puts("Scheduler: Switching to task PID ");
        // print_uint(current_task->pid); uart_puts(" with SP: 0x");
        // print_hex(current_task->kernel_sp); uart_puts("\n");
        return current_task->kernel_sp;
    } else {
        // This case should ideally be handled by an idle task.
        // If current_task is NULL here, it means next_task was NULL and
        // previous_task was not viable. uart_puts("Scheduler: Fallthrough - no
        // current task, returning original SP.\n");
        return current_task_sp_val;  // Should be SP of kernel_main context
    }
}  // This should be the only closing brace for the function.