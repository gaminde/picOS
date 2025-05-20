.section ".text.vectors" // Place vectors in a specific subsection of .text
.align 11 // Align to 2^11 = 2048 bytes, as VBAR_EL1 requires

.global exception_vectors_start

// Exception Vector Table (16 entries, each 0x80 bytes)
// Each entry is a branch to a handler.

exception_vectors_start:
    // Current EL with SP0 (Stack Pointer 0)
    b default_sync_handler      // Synchronous (SVC, instruction/data aborts from current EL)
    .align 7                    // Align to 0x80 byte boundary for next vector
    b default_irq_handler       // IRQ (Interrupt Request)
    .align 7
    b default_fiq_handler       // FIQ (Fast Interrupt Request)
    .align 7
    b default_serror_handler    // SError (System Error)
    .align 7

    // Current EL with SPx (Stack Pointer x, i.e., EL1's own stack)
    b default_sync_handler      // Synchronous
    .align 7
    b default_irq_handler       // IRQ
    .align 7
    b default_fiq_handler       // FIQ
    .align 7
    b default_serror_handler    // SError
    .align 7

    // Lower EL using AArch64 (e.g., EL0)
    b default_sync_handler      // Synchronous
    .align 7
    b default_irq_handler       // IRQ
    .align 7
    b default_fiq_handler       // FIQ
    .align 7
    b default_serror_handler    // SError
    .align 7

    // Lower EL using AArch32 (Not typically used in pure AArch64 OS)
    b default_sync_handler      // Synchronous
    .align 7
    b default_irq_handler       // IRQ
    .align 7
    b default_fiq_handler       // FIQ
    .align 7
    b default_serror_handler    // SError
    .align 7

// Default Handlers
.global default_sync_handler
.global default_irq_handler
.global default_fiq_handler
.global default_serror_handler

.extern c_sync_handler

// Default Synchronous Exception Handler
// This handler will now save full context and call a C function.
default_sync_handler:
    // Allocate a 272-byte frame on the stack.
    // This frame will hold 31 GPRs (x0-x30), SPSR_EL1, and ELR_EL1.
    // (31 GPRs + SPSR_EL1 + ELR_EL1) * 8 bytes/register = 33 * 8 = 264 bytes.
    // 272 bytes provides 16-byte alignment for the frame (17 * 16 = 272).
    sub sp, sp, #272

    // Save general-purpose registers x0-x29 (as pairs)
    stp x0, x1, [sp, #(0 * 16)]
    stp x2, x3, [sp, #(1 * 16)]
    stp x4, x5, [sp, #(2 * 16)]
    stp x6, x7, [sp, #(3 * 16)]
    stp x8, x9, [sp, #(4 * 16)]
    stp x10, x11, [sp, #(5 * 16)]
    stp x12, x13, [sp, #(6 * 16)]
    stp x14, x15, [sp, #(7 * 16)]
    stp x16, x17, [sp, #(8 * 16)]
    stp x18, x19, [sp, #(9 * 16)]
    stp x20, x21, [sp, #(10 * 16)]
    stp x22, x23, [sp, #(11 * 16)]
    stp x24, x25, [sp, #(12 * 16)]
    stp x26, x27, [sp, #(13 * 16)]
    stp x28, x29, [sp, #(14 * 16)]  // Registers x0-x29 stored up to offset 224 + 16 = 240

    // Save x30 (Link Register of the interrupted context)
    str x30, [sp, #(15 * 16)]       // Offset 240

    // Save SPSR_EL1 and ELR_EL1
    mrs x2, SPSR_EL1                // Move SPSR_EL1 to scratch register x2
    mrs x3, ELR_EL1                 // Move ELR_EL1 to scratch register x3
    stp x2, x3, [sp, #(15 * 16 + 8)] // Store SPSR_EL1 and ELR_EL1 at offset 248

    // Pass current SP (pointer to the base of the saved context frame) to C handler
    mov x0, sp
    bl c_sync_handler

    // Restore SPSR_EL1 and ELR_EL1
    ldp x2, x3, [sp, #(15 * 16 + 8)] // Load SPSR_EL1 into x2, ELR_EL1 into x3
    msr SPSR_EL1, x2
    msr ELR_EL1, x3

    // Restore x30
    ldr x30, [sp, #(15 * 16)]

    // Restore general-purpose registers x0-x29
    ldp x28, x29, [sp, #(14 * 16)]
    ldp x26, x27, [sp, #(13 * 16)]
    ldp x24, x25, [sp, #(12 * 16)]
    ldp x22, x23, [sp, #(11 * 16)]
    ldp x20, x21, [sp, #(10 * 16)]
    ldp x18, x19, [sp, #(9 * 16)]
    ldp x16, x17, [sp, #(8 * 16)]
    ldp x14, x15, [sp, #(7 * 16)]
    ldp x12, x13, [sp, #(6 * 16)]
    ldp x10, x11, [sp, #(5 * 16)]
    ldp x8, x9, [sp, #(4 * 16)]
    ldp x6, x7, [sp, #(3 * 16)]
    ldp x4, x5, [sp, #(2 * 16)]
    ldp x2, x3, [sp, #(1 * 16)]
    ldp x0, x1, [sp, #(0 * 16)]

    // Deallocate stack frame
    add sp, sp, #272
    eret                    // Return from exception

// Default IRQ Handler - For now, just a loop.
// We would apply similar context saving if calling C.
default_irq_handler:
    // adr x0, default_irq_msg // Old message
    // bl uart_puts            // Old print
    b .                     // Infinite loop

// Default FIQ Handler - For now, just a loop.
default_fiq_handler:
    b .

// Default SError Handler - For now, just a loop.
default_serror_handler:
    b .
