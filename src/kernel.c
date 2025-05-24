#include "kernel.h"

#include "exceptions.h"
#include "gic.h"
#include "task.h"
#include "timer.h"  // Make sure this is included
#include "uart.h"

// Simple task function 1
void simple_task_1(void *arg) {
    uint32_t task_id = (uint32_t)(uint64_t)arg;  // Retrieve the argument
    uint64_t counter = 0;

    uart_puts("Task ");
    print_uint(task_id);
    uart_puts(" started.\n");

    while (1) {
        uart_puts("Task ");
        print_uint(task_id);
        uart_puts(" says: Hello! Count: ");
        print_uint(counter++);
        uart_puts("\n");

        // Simple delay loop (very approximate)
        for (volatile int i = 0; i < 10000000; ++i) {
            // Do nothing, just burn CPU cycles
        }
        // In a real scenario, tasks would yield or block on events.
    }
}

// Simple task function 2
void simple_task_2(void *arg) {
    uint32_t task_id = (uint32_t)(uint64_t)arg;  // Retrieve the argument
    uint64_t counter = 0;

    uart_puts("Task ");
    print_uint(task_id);
    uart_puts(" started.\n");

    while (1) {
        uart_puts("Task ");
        print_uint(task_id);
        uart_puts(" says: World! Count: ");
        print_uint(counter++);
        uart_puts("\n");

        // Simple delay loop
        for (volatile int i = 0; i < 15000000; ++i) {
            // Do nothing
        }
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
    enable_interrupts();  // Enable interrupts globally (for timer)

    // At this point, the tasks are in the ready queue.
    // The scheduler isn't running yet, so they won't execute.
    // The first timer interrupt will trigger c_irq_handler.
    // We need to modify c_irq_handler to call schedule().
    // And schedule() needs to pick a task and set it as current_task.
    // The very first context switch will happen when schedule() is first called
    // and it switches from the implicit kernel_main "task" to one of the
    // created tasks.

    // For now, kernel_main will just idle. The timer interrupts will fire.
    // Once schedule() is implemented and called, we should see task output.
    uart_puts("Kernel entering idle loop (WFI). Tasks are ready.\n");
    while (1) {
        __asm__ __volatile__("wfi");  // Wait for interrupt
    }
}