.section ".text.vectors"
    .align 11 // Align to 2048 bytes (2^11)

    .globl _exception_vector_table
    .globl default_sync_handler // Or specific sync handler
    .globl irq_handler          // Or specific irq handler
    .globl default_fiq_handler   // Or specific fiq handler
    .globl default_serror_handler // Or specific serror handler

    .extern c_sync_handler
    .extern c_irq_handler      // <<< MAKE SURE THIS IS PRESENT
    .extern minimal_fiq_print  // Or c_fiq_handler if you have one
    .extern minimal_serror_print // Or c_serror_handler if you have one

_exception_vector_table:
    // Current EL with SP0
    b default_sync_handler      // Synchronous (offset 0x000)
    .align 7                    // Align to 128 bytes (2^7)
    b irq_handler               // IRQ (offset 0x080) <--- CHANGE THIS
    .align 7
    b default_fiq_handler       // FIQ (offset 0x100)
    .align 7
    b default_serror_handler    // SError (offset 0x180)
    .align 7

    // Current EL with SPx (EL1)
    b default_sync_handler      // Synchronous (offset 0x200)
    .align 7
    b irq_handler               // IRQ (offset 0x280) <--- AND THIS (this is the one we are likely using)
    .align 7
    b default_fiq_handler       // FIQ (offset 0x300)
    .align 7
    b default_serror_handler    // SError (offset 0x380)
    .align 7

    // Lower EL using AArch64
    b default_sync_handler      // Synchronous (offset 0x400)
    .align 7
    b irq_handler               // IRQ (offset 0x480) <--- AND THIS
    .align 7
    b default_fiq_handler       // FIQ (offset 0x500)
    .align 7
    b default_serror_handler    // SError (offset 0x580)
    .align 7

    // Lower EL using AArch32
    b default_sync_handler      // Synchronous (offset 0x600)
    .align 7
    b irq_handler               // IRQ (offset 0x680) <--- AND THIS
    .align 7
    b default_fiq_handler       // FIQ (offset 0x700)
    .align 7
    b default_serror_handler    // SError (offset 0x780)
    .align 7


// Define a macro for saving all general-purpose registers and LR
.macro save_all_regs
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x8, x9, [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, x19, [sp, #-16]!
    stp x20, x21, [sp, #-16]!
    stp x22, x23, [sp, #-16]!
    stp x24, x25, [sp, #-16]!
    stp x26, x27, [sp, #-16]!
    stp x28, x29, [sp, #-16]!
    str x30, [sp, #-16]!  // Save LR (x30)
.endm

// Define a macro for restoring all general-purpose registers and LR
.macro restore_all_regs
    ldr x30, [sp], #16    // Restore LR (x30)
    ldp x28, x29, [sp], #16
    ldp x26, x27, [sp], #16
    ldp x24, x25, [sp], #16
    ldp x22, x23, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x18, x19, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8, x9, [sp], #16
    ldp x6, x7, [sp], #16
    ldp x4, x5, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16
.endm

// IRQ Handler (example for Current EL with SP_EL0)
// This is one of the 16 entries in your vector table.
// Ensure your vector table entry for "IRQ from Current EL using SP_EL0"
// (offset 0x280 from VBAR_EL1) branches to this irq_handler.
// Or, if you have a common stub that then branches here, that's fine too.

irq_handler:
    save_all_regs       // Save general-purpose registers x0-x30 (lr). SP is adjusted.

    mov x0, sp          // Pass current stack pointer (base of saved regs) as context_state_t*
                        // c_irq_handler's ctx argument is currently unused for register access.
    bl c_irq_handler    // Branch to the C handler

    restore_all_regs    // Restore general-purpose registers. SP is adjusted back.
    eret                // Return from exception

// Default Synchronous Exception Handler
default_sync_handler:
    // For now, just call the minimal print.
    // A full sync handler would save context, read ESR_EL1, ELR_EL1, and call c_sync_handler.
    save_all_regs
    mrs x0, esr_el1     // Arg1 for c_sync_handler
    mrs x1, elr_el1     // Arg2 for c_sync_handler
    mov x2, sp          // Arg3 for c_sync_handler (pointer to saved context)
    bl c_sync_handler
    restore_all_regs
    eret

// Default FIQ Handler
default_fiq_handler:
    save_all_regs
    // mov x0, sp // If c_fiq_handler takes context
    bl minimal_fiq_print // Or bl c_fiq_handler
    restore_all_regs
    eret

// Default SError Handler
default_serror_handler:
    save_all_regs
    // mov x0, sp // If c_serror_handler takes context
    bl minimal_serror_print // Or bl c_serror_handler
    restore_all_regs
    eret
