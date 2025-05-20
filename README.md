# picOS - A Hobby Operating System for AArch64

## About

picOS is a small, educational hobby operating system being developed for the AArch64 architecture. The primary goal of this project is to learn about low-level system programming, OS concepts, and the ARMv8-A architecture.

This project is built to run on QEMU's `virt` machine.

## Current Features

*   Basic bootloader (`boot.s`) to set up the stack and jump to C.
*   Minimal UART driver for serial output.
*   Exception vector table setup.
*   Rudimentary synchronous exception handling (SVC calls) with context saving/restoring and a C handler.
*   Rudimentary IRQ exception handling infrastructure with context saving/restoring and a C handler stub.
*   Basic Generic Interrupt Controller (GICv2) initialization.
*   IRQs unmasked in PSTATE.
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
    This will produce a `boot.elf` file (the linked kernel) and `boot.bin` (a raw binary, though `boot.elf` is typically used with QEMU for kernels).

## Running picOS in QEMU

To run picOS in QEMU:

```sh
make run
```

This command (defined in the `Makefile`) typically executes:
```sh
qemu-system-aarch64 -machine virt -cpu max -m 64M -nographic -kernel boot.elf
```

You should see output from the bootloader and kernel on your terminal.

## Project Structure

*   `src/`: Contains all assembly (`.s`) and C (`.c`) source files.
*   `include/`: Contains all header (`.h`) files.
*   `Makefile`: Defines how to build the project.
*   `linker.ld`: Linker script to define memory layout.

## Goals & Next Steps

*   Implement ARM Generic Timer interrupts.
*   Further develop IRQ handling.
*   Basic memory management (paging).
*   Simple scheduler and process management.

---

My actual TO-DO list (for enabling timer interrupts):

- [ ] Unmasking IRQs: Telling the CPU it's okay to be interrupted.
- [ ] Initializing the Generic Interrupt Controller (GIC): The GIC is responsible for managing interrupts from various peripherals and routing them to the CPU.
- [ ] Configuring and Enabling a Timer: We'll use the ARM Generic Timer, which is built into the CPU cores.
- [ ] Updating the C IRQ Handler: To acknowledge the interrupt from the GIC and handle the timer event.