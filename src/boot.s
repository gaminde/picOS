.global _start

// Functions from other files we will call or use
.extern kernel_main
.extern uart_init // From uart.s
.extern uart_puts // From uart.s

.section .rodata
    boot_msg: .asciz "picOS Bootloader Initialized.\n"

.text
_start:
    // 1. Set up stack pointer
    // Ensure this stack address is valid and won't collide with kernel/BSS/heap
    // For QEMU virt machine, RAM starts at 0x40000000.
    // Let's put the initial stack for boot.s at the end of the first 1MB of RAM, growing downwards.
    // 0x40000000 (RAM start) + 0x100000 (1MB) = 0x40100000
    // This is just an initial stack for boot.s and early kernel_main.
    // The kernel might set up its own stack later, or per-task stacks.
    ldr x30, =0x40100000 // Example stack top for bootloader
    mov sp, x30

    // 2. Initialize UART (VBAR_EL1 setup is now done in C by exceptions_init)
    bl uart_init

    // 3. Print a boot message
    adr x0, boot_msg
    bl uart_puts

    // Optional: Clear BSS (if your linker script defines _bss_start and _bss_end)
    // ldr x1, =_bss_start
    // ldr x2, =_bss_end
    // clear_bss_loop:
    //     cmp x1, x2
    //     b.ge bss_cleared
    //     str xzr, [x1], #8 // Zero out 8 bytes and increment pointer
    //     b clear_bss_loop
    // bss_cleared:

    // 4. Jump to kernel_main in C
    bl kernel_main

// Infinite loop if kernel_main returns (it shouldn't)
end_loop:
    b end_loop

.global enable_interrupts
enable_interrupts:
    msr daifclr, #2 // Clear IRQ mask bit
    ret
