# picOS - A Hobby Operating System for AArch64

## About

picOS is a small, educational hobby operating system being developed for the AArch64 architecture. The primary goal of this project is to learn about low-level system programming, OS concepts, and the ARMv8-A architecture.

This project is built to run on QEMU's `virt` machine.

## Current Features

*   Basic bootloader (`boot.s`) to set up the stack and jump to C.
*   Minimal UART driver for serial output.
*   Helper functions for printing hexadecimal (`print_hex`) and unsigned decimal (`print_uint`) numbers.
*   Exception vector table setup for EL1.
*   Synchronous exception handling (e.g., for SVC calls) with full context saving/restoring and a C handler (`c_sync_handler`) that decodes `ESR_EL1`.
*   IRQ exception handling infrastructure with full context saving/restoring.
*   Generic Interrupt Controller (GICv2) initialized for the distributor and CPU interface.
*   IRQs unmasked in PSTATE, allowing the CPU to receive interrupts.
*   ARM Generic Timer (EL1 Physical Timer) initialized and configured to generate periodic interrupts.
*   C IRQ handler (`c_irq_handler`) processes timer interrupts:
    *   Acknowledges interrupts via GIC (reads IAR, writes EOIR).
    *   Calls a timer-specific handler (`handle_timer_irq`) to re-arm the timer.
    *   Prints an incrementing "Timer Tick" count to the console.
*   CPU idles using the `wfi` (Wait For Interrupt) instruction in the main kernel loop.
*   Organized project structure with `src/` for source files and `include/` for headers.
*   Makefile for building the project.

## Building picOS

1.  **Prerequisites:**
    *   An AArch64 cross-compiler toolchain (e.g., `aarch64-elf-gcc`, `aarch64-elf-as`, `aarch64-elf-ld`).
    *   `make` utility.
    *   QEMU for AArch64 (`qemu-system-aarch64`).

2.  **Clone the repository (if you haven't already):**
    ```sh
    git clone <your-repository-url>
    cd picOS
    ```

3.  **Build:**
    Run the `make` command in the root directory of the project:
    ```sh
    make
    ```
    This will produce a `boot.elf` file (the linked kernel).

## Running picOS in QEMU

To run picOS in QEMU:

```sh
make run
```

This command (defined in the `Makefile`) typically executes:
```sh
qemu-system-aarch64 -machine virt -cpu max -m 64M -nographic -kernel boot.elf
```

You should see output from the bootloader, kernel initialization messages, and then periodic "Timer Tick: N" messages.

## Project Structure

*   `src/`: Contains all assembly (`.s`) and C (`.c`) source files.
*   `include/`: Contains all header (`.h`) files.
*   `Makefile`: Defines how to build the project.
*   `linker.ld`: Linker script to define memory layout.

## Goals & Next Steps

The immediate TO-DO list for enabling timer interrupts is now complete!

Future major goals include:
*   Implement basic preemptive multitasking.
*   Further develop IRQ handling (e.g., for other devices).
*   Basic memory management (paging).
*   More device drivers.

---
My actual TO-DO list (for enabling timer interrupts):

- [x] Unmasking IRQs: Telling the CPU it's okay to be interrupted.
- [x] Initializing the Generic Interrupt Controller (GIC): The GIC is responsible for managing interrupts from various peripherals and routing them to the CPU.
- [x] Configuring and Enabling a Timer: We'll use the ARM Generic Timer, which is built into the CPU cores.
- [x] Updating the C IRQ Handler: To acknowledge the interrupt from the GIC, handle the timer event (re-arm, print tick count).

*(This section is now complete!)*

Next, I've decided to work on multitasking.  This will be a basic solution where each process "believes" it has the full address range to itself - we'll deal with Paging later - and, all "tasks" are essentially kernel threads sharing the same address space.

---
**Next Major Goal: Basic Preemptive Multitasking**

- [ ] **Define Task Control Block (TCB) Structure:**
    - [ ] Create `struct` for TCB (registers, stack pointer, state, ID).
- [ ] **Implement Simple Task List/Array:**
    - [ ] Array to hold TCBs.
- [ ] **Develop Task Creation/Initialization:**
    - [ ] `task_create()` function.
    - [ ] Allocate stack for new tasks.
    - [ ] Initialize saved context on the new task's stack (initial `ELR_EL1`, `SPSR_EL1`).
- [ ] **Implement Context Switching Mechanism:**
    - [ ] `save_context()` function.
    - [ ] `restore_context()` function (likely involves assembly).
- [ ] **Implement a Basic Scheduler:**
    - [ ] `schedule()` function (called by timer IRQ).
    - [ ] Round-robin task selection logic.
- [ ] **Integrate Scheduler with Timer Interrupt:**
    - [ ] Modify `c_irq_handler` to call `schedule()`.
- [ ] **Create and Run Initial Tasks:**
    - [ ] Create 2-3 simple tasks in `kernel_main`.
    - [ ] Start the multitasking.