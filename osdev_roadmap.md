# How to Create Your Own Operating System (AArch64 Hobbyist Guide)

This document outlines the roadmap I am using to develop a hobby operating system from scratch, primarily targeting the AArch64 architecture (as picOS does). 

Completed items reflect the current status of the picOS project.

## Phase 0: Foundations & Setup

*   [x] **Understand Target Architecture Basics:**
    *   [x] AArch64 Execution Levels (EL0, EL1, EL2, EL3).
    *   [x] AArch64 Registers (General Purpose, System Registers).
    *   [x] Basic AArch64 Assembly instructions.
*   [x] **Set Up Development Environment:**
    *   [x] Install an AArch64 cross-compiler (e.g., `aarch64-linux-gnu-gcc`, `aarch64-elf-gcc`).
    *   [x] Install an emulator (e.g., QEMU with `qemu-system-aarch64`).
    *   [x] Choose a text editor/IDE (e.g., VS Code).
*   [x] **Establish a Build System:**
    *   [x] Create a `Makefile` for compiling assembly and C code.
    *   [x] Define rules for linking into an ELF executable.
    *   [x] Add rules for converting ELF to a raw binary (optional, but common).

## Phase 1: Bootstrapping & Bare Minimum Kernel

*   [x] **Write a Minimal Bootloader (Assembly - e.g., `boot.s`):**
    *   [x] Define an entry point (`_start`).
    *   [x] Perform essential early CPU setup for EL1:
        *   [x] Set up a stack pointer (`sp_el1`).
        *   [ ] (Optional early) Mask/unmask exceptions (DAIF register).
        *   [ ] (Optional early) Configure basic MMU/caches if not done later.
    *   [x] Branch to a C kernel entry point (`kernel_main`).
*   [x] **Create a Minimal C Kernel (`kernel.c`):**
    *   [x] Implement `kernel_main()`.
    *   [x] Basic "Hello World" using a hardware output method.
*   [x] **Implement Basic Output (e.g., UART/Serial):**
    *   [x] Understand memory-mapped I/O.
    *   [x] Write basic functions to initialize the UART (e.g., PL011 on QEMU `virt`).
    *   [x] Write functions to send a character and a string via UART.
    *   [x] Create helper print functions (e.g., `print_hex`, `print_uint`).

## Phase 2: CPU Internals & Exception Handling

*   [x] **Set Up Exception Vector Table (Assembly - e.g., `vectors.s`):**
    *   [x] Define handlers for different exception types from EL1 (Synchronous, IRQ, FIQ, SError).
    *   [x] Ensure correct alignment and structure of the vector table.
    *   [x] Load the vector table base address into `VBAR_EL1`.
*   [x] **Implement Synchronous Exception Handling:**
    *   [x] In assembly handler:
        *   [x] Save general-purpose registers and relevant system registers (`SPSR_EL1`, `ELR_EL1`).
        *   [x] Prepare arguments (e.g., pointer to saved registers/trap frame).
        *   [x] Branch to a C handler function (`c_sync_handler`).
    *   [x] In C handler (`c_sync_handler`):
        *   [x] Read `ESR_EL1` (Exception Syndrome Register) to determine the cause.
        *   [x] Read `FAR_EL1` (Fault Address Register) if applicable.
        *   [x] Implement basic decoding and reporting of common exceptions (e.g., SVC, Data Abort, Instruction Abort).
    *   [x] In assembly handler: Restore context and return from exception (`eret`).
*   [x] **Implement Interrupt (IRQ) Handling:**
    *   [x] In assembly handler:
        *   [x] Save context (similar to synchronous exceptions).
        *   [x] Branch to a C IRQ handler function (`c_irq_handler`).
    *   [x] Unmask IRQs in `PSTATE` (clear the I bit in `DAIF`).
    *   [x] **Initialize an Interrupt Controller (e.g., GICv2/v3):**
        *   [x] Initialize the GIC Distributor (`GICD_*` registers).
        *   [x] Initialize the GIC CPU Interface (`GICC_*` registers).
        *   [x] Enable interrupt forwarding to the CPU.
    *   [x] In C handler (`c_irq_handler`):
        *   [x] Acknowledge the interrupt (read GIC IAR - Interrupt Acknowledge Register).
        *   [x] Identify the interrupt source (IRQ ID).
        *   [x] Dispatch to a specific device interrupt handler.
        *   [x] Signal End Of Interrupt (write GIC EOIR - End Of Interrupt Register).
    *   [x] In assembly handler: Restore context and `eret`.
*   [x] **Implement a Timer Interrupt:**
    *   [x] Choose a timer (e.g., ARM Generic Timer - EL1 Physical Timer).
    *   [x] Configure the timer to generate periodic interrupts:
        *   [x] Set timer frequency (`CNTFRQ_EL0`).
        *   [x] Set compare value (`CNTP_CVAL_EL0` or `CNTV_CVAL_EL0`).
        *   [x] Enable the timer (`CNTP_CTL_EL0` or `CNTV_CTL_EL0`).
    *   [x] Enable the timer's IRQ in the GIC (e.g., IRQ ID 30 for EL1 Physical Timer).
    *   [x] Write a timer-specific handler function:
        *   [x] Re-arm the timer for the next interrupt.
        *   [x] Perform periodic tasks (e.g., increment a tick counter).
*   [x] **CPU Idling:**
    *   [x] Use `wfi` (Wait For Interrupt) instruction in the kernel's idle loop.

## Phase 3: Concurrency & Scheduling

*   [ ] **Define Task/Process Concept:**
    *   [ ] What constitutes a task in your OS.
*   [ ] **Design Task Control Block (TCB) / Process Control Block (PCB):**
    *   [ ] `struct` to store task state:
        *   [ ] Saved registers (GPRs, `SP_ELx`, `ELR_EL1`, `SPSR_EL1`).
        *   [ ] Task ID (PID).
        *   [ ] Task state (e.g., `RUNNING`, `READY`, `BLOCKED`, `ZOMBIE`).
        *   [ ] Pointer to task's stack.
        *   [ ] Pointer to task's page table (if per-task virtual memory - to be added in Phase 4).
        *   [ ] (Optional) Priority, scheduling information, IPC resources.
*   [ ] **Implement Task Creation & Initialization:**
    *   [ ] Function to allocate TCB and stack.
    *   [ ] Initialize the saved context on the new task's stack to start execution at a specific function.
*   [ ] **Implement Context Switching:**
    *   [ ] Assembly routines to save the current task's context to its TCB.
    *   [ ] Assembly routines to restore the next task's context from its TCB.
*   [ ] **Develop a Scheduler:**
    *   [ ] Data structure for ready tasks (e.g., run queue, linked list).
    *   [ ] Scheduling algorithm (e.g., round-robin, priority-based).
    *   [ ] `schedule()` function to select the next task to run.
*   [ ] **Integrate Scheduler with Timer Interrupt:**
    *   [ ] Call `schedule()` from the timer interrupt handler to enable preemption.
*   [ ] **Implement Basic Synchronization Primitives (Optional for early stages):**
    *   [ ] Spinlocks.
    *   [ ] Semaphores/Mutexes (requires blocking/unblocking tasks).
*   [ ] **Task Termination and Cleanup.**

## Phase 4: Memory Management

*   [ ] **Understand Physical Memory Layout:**
    *   [ ] How RAM is mapped by the hardware/emulator.
    *   [ ] Location of kernel code/data, peripherals.
*   [ ] **Implement a Physical Memory Manager (Frame Allocator):**
    *   [ ] Data structure to track free/used physical memory pages (e.g., bitmap, linked list).
    *   [ ] Functions to allocate and free physical page frames.
*   [ ] **Implement Virtual Memory (Paging):**
    *   [ ] Understand AArch64 MMU, translation table format (e.g., 4KB granule, multi-level).
    *   [ ] Define page table structures (L0, L1, L2, L3 tables).
    *   [ ] Write functions to create and populate page table entries (PTEs).
    *   [ ] Set memory attributes (Normal, Device, Cacheability, Permissions) using `MAIR_EL1`.
    *   [ ] Configure `TCR_EL1` (Translation Control Register).
    *   [ ] Set `TTBR0_EL1` and/or `TTBR1_EL1` (Translation Table Base Registers).
    *   [ ] **Enable the MMU:** Set the M bit in `SCTLR_EL1`.
    *   [ ] Create initial page tables for the kernel (identity mapping or higher-half kernel).
    *   [ ] (Later, after multitasking is stable) Create separate page tables for each task.
*   [ ] **Handle Page Faults:**
    *   [ ] Modify synchronous exception handler to identify Data Aborts and Instruction Aborts caused by MMU.
    *   [ ] Implement page fault resolution (e.g., for demand paging, copy-on-write - advanced).
*   [ ] **Kernel Heap:**
    *   [ ] Implement `kmalloc`/`kfree` for dynamic memory allocation within the kernel.

## Phase 5: Device Drivers & I/O

*   [ ] **Develop a Simple Device Driver Model (Optional but good practice).**
*   [ ] **UART Input Driver:**
    *   [ ] Configure UART for receive interrupts.
    *   [ ] Write ISR to read characters, handle buffering, line editing (basic).
*   [ ] **Keyboard Driver (if not using UART for all input).**
*   [ ] **Block Device Driver (e.g., VirtIO Block for QEMU):**
    *   [ ] Understand VirtIO interface.
    *   [ ] Implement functions to read/write blocks.
    *   [ ] Handle interrupts from the block device.
*   [ ] **Implement a Simple Filesystem (e.g., FAT16/FAT32 reader, or custom):**
    *   [ ] Parse filesystem structures.
    *   [ ] Read directories and files.
    *   [ ] (Later) Write files.

## Phase 6: User Space & System Calls

*   [ ] **Separate Kernel Space and User Space:**
    *   [ ] Define distinct virtual address ranges.
    *   [ ] Use page tables to enforce protection.
*   [ ] **Implement System Call Interface:**
    *   [ ] Use `SVC` instruction from user mode to trigger a synchronous exception.
    *   [ ] In `c_sync_handler`, identify SVC, dispatch to system call handler.
    *   [ ] Define system call numbers and handlers (e.g., `read`, `write`, `fork`, `exec`).
*   [ ] **Load and Run User Programs:**
    *   [ ] Define an executable file format (e.g., simple ELF subset, custom format).
    *   [ ] Write a loader to read user programs from a filesystem/initramfs into memory.
    *   [ ] Set up user mode stack, page tables.
    *   [ ] Transition to EL0 to run the user program.
*   [ ] **Create a Basic Shell/Command-Line Interface.**

## Phase 7: Advanced Topics & Refinements

*   [ ] **Multicore Support (SMP - Symmetric Multiprocessing):**
    *   [ ] Boot secondary CPU cores.
    *   [ ] Per-core data structures.
    *   [ ] Locking for shared resources.
    *   [ ] Inter-Processor Interrupts (IPIs).
*   [ ] **Networking Stack (e.g., VirtIO Net).**
*   [ ] **More Advanced Scheduling Algorithms.**
*   [ ] **Security Enhancements.**
*   [ ] **GUI (Highly Advanced).**
*   [ ] **Power Management.**

And there you have it.  A lot of work and some tasks can be their own project.

Good Luck!!
