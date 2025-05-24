#include "kernel.h"  // For print_uint, print_hex if used directly here

#include "exceptions.h"
#include "gic.h"
#include "task.h"  // <<< Ensure this is included for task_exit()
#include "timer.h"
#include "uart.h"

// Simple task function 1
void simple_task_1(void *arg) {
    uint32_t task_id = (uint32_t)(uint64_t)arg;
    uart_puts("Task ");
    print_uint(task_id);
    uart_puts(" started.\n");

    for (unsigned int i = 0; i < 5; ++i) {  // Run for 5 iterations
        uart_puts("Task ");
        print_uint(task_id);
        uart_puts(" says: Hello! Count: ");
        print_uint(i);
        uart_puts("\n");
        // Simple delay loop
        for (volatile int k = 0; k < 2000000; ++k);  // Adjust delay as needed
    }
    uart_puts("Task ");
    print_uint(task_id);
    uart_puts(" is exiting.\n");
    task_exit();
    // Should not reach here
    uart_puts("Task ");
    print_uint(task_id);
    uart_puts(" ERROR: Reached after task_exit()!\n");
    while (1);
}

// Simple task function 2
void simple_task_2(void *arg) {
    uint32_t task_id = (uint32_t)(uint64_t)arg;
    uart_puts("Task ");
    print_uint(task_id);
    uart_puts(" started.\n");

    for (unsigned int i = 0; i < 7; ++i) {  // Run for 7 iterations
        uart_puts("Task ");
        print_uint(task_id);
        uart_puts(" says: World! Count: ");
        print_uint(i);
        uart_puts("\n");
        // Simple delay loop
        for (volatile int k = 0; k < 1500000; ++k);  // Adjust delay as needed
    }
    uart_puts("Task ");
    print_uint(task_id);
    uart_puts(" is exiting.\n");
    task_exit();
    // Should not reach here
    uart_puts("Task ");
    print_uint(task_id);
    uart_puts(" ERROR: Reached after task_exit()!\n");
    while (1);
}

// Idle task function
void idle_task_function(void *arg) {
    // Argument is not used for the idle task, but signature matches task_create
    (void)arg;

    uart_puts("Idle task started.\n");
    while (1) {
        // uart_puts("Idle task WFI...\n"); // Optional: for debugging, can be
        // very noisy
        __asm__ __volatile__("wfi");
        // When an interrupt occurs and is handled, execution will resume here
        // after the interrupt handler and scheduler (if a context switch
        // happened). If the scheduler picks another task, the idle task remains
        // suspended. If the scheduler picks the idle task again, it re-enters
        // wfi.
    }
}

void kernel_main(void) {
    uart_init();
    uart_puts("\n-----------------------------------\n");
    uart_puts("picOS Kernel (AArch64) Booting...\n");
    uart_puts("-----------------------------------\n");

    exceptions_init();
    gic_init();
    timer_init_periodic(1000000);  // 1 second timer (1MHz clock, 1M ticks)
    task_init_system();

    uart_puts("Creating idle task...\n");
    int idle_pid = task_create(idle_task_function, NULL, "IdleTask");
    if (idle_pid < 0) {
        uart_puts("FATAL: Failed to create idle task!\n");
        // Potentially halt or panic here
        while (1);
    } else {
        // Find the TCB for the idle task and store it.
        // task_create adds to ready_queue, we need to remove it and store it
        // separately. This is a bit clunky; task_create could be modified not
        // to auto-add to ready_queue or take a flag. For now, we find it and
        // remove it.

        // Find the idle task's TCB (it was just created and added to ready
        // queue) And remove it from the ready queue.
        tcb_t *temp_idle_tcb = NULL;
        if (ready_queue_head && ready_queue_head->pid == (uint32_t)idle_pid) {
            temp_idle_tcb = ready_queue_head;
            ready_queue_head = ready_queue_head->next_in_queue;
        } else if (ready_queue_head) {
            tcb_t *current = ready_queue_head;
            while (current->next_in_queue) {
                if (current->next_in_queue->pid == (uint32_t)idle_pid) {
                    temp_idle_tcb = current->next_in_queue;
                    current->next_in_queue =
                        temp_idle_tcb->next_in_queue;  // Unlink
                    break;
                }
                current = current->next_in_queue;
            }
        }

        if (temp_idle_tcb) {
            idle_task_tcb = temp_idle_tcb;        // Store the global pointer
            idle_task_tcb->next_in_queue = NULL;  // Ensure it's isolated
            idle_task_tcb->state =
                TASK_READY;  // It's ready, just not in the main queue
            uart_puts("Idle task created with PID: ");
            print_uint(idle_pid);
            uart_puts(" and set aside.\n");
        } else {
            uart_puts("FATAL: Could not find and set aside idle task TCB!\n");
            while (1);
        }
    }

    uart_puts("Creating tasks...\n");
    int pid1 = task_create(simple_task_1, (void *)1, "Task1");
    if (pid1 < 0) {
        uart_puts("Failed to create task 1\n");
    } else {
        uart_puts("Task 1 created with PID: ");
        print_uint(pid1);
        uart_puts("\n");
    }

    int pid2 = task_create(simple_task_2, (void *)2, "Task2");
    if (pid2 < 0) {
        uart_puts("Failed to create task 2\n");
    } else {
        uart_puts("Task 2 created with PID: ");
        print_uint(pid2);
        uart_puts("\n");
    }

    uart_puts(
        "All tasks created. Enabling interrupts and starting scheduler "
        "(conceptually).\n");
    enable_interrupts();

    uart_puts("Kernel entering idle loop (WFI). Tasks are ready.\n");
    while (1) {
        // The kernel_main's WFI is now less critical as the idle task will take
        // over when no other tasks are scheduled. However, before the first
        // schedule call, this WFI is still what the "implicit kernel task"
        // does.
        __asm__ __volatile__("wfi");
    }
}