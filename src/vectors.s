#include "common_macros.h" // For ALIGN_DIRECTIVE if used, or other constants

.global _exception_vector_table

// Macro to save general-purpose registers x0-x29 and lr (x30)
// Assumes SP is 16-byte aligned before this macro.
// Saves 31 registers * 8 bytes/reg = 248 bytes.
// Decrements SP by 256 bytes (16 pairs of registers) to maintain 16-byte alignment.
.macro save_gprs_lr
    stp x0,  x1,  [sp, #-16*16]! // SP is now SP - 256. Store x0, x1 at SP + 0
    stp x2,  x3,  [sp, #16*1]    // Store x2, x3 at SP + 16
    stp x4,  x5,  [sp, #16*2]
    stp x6,  x7,  [sp, #16*3]
    stp x8,  x9,  [sp, #16*4]
    stp x10, x11, [sp, #16*5]
    stp x12, x13, [sp, #16*6]
    stp x14, x15, [sp, #16*7]
    stp x16, x17, [sp, #16*8]
    stp x18, x19, [sp, #16*9]
    stp x20, x21, [sp, #16*10]
    stp x22, x23, [sp, #16*11]
    stp x24, x25, [sp, #16*12]
    stp x26, x27, [sp, #16*13]
    stp x28, x29, [sp, #16*14]
    str x30,      [sp, #16*15]    // Store lr (x30) at SP + 240
.endm

// Macro to restore general-purpose registers x0-x30
// SP should point to the base of saved GPRs (i.e., where x0 was stored).
// Increments SP by 256 bytes.
.macro restore_gprs_lr
    ldp x0,  x1,  [sp], #16*1 // Load x0, x1, SP is now SP + 16
    ldp x2,  x3,  [sp], #16*1
    ldp x4,  x5,  [sp], #16*1
    ldp x6,  x7,  [sp], #16*1
    ldp x8,  x9,  [sp], #16*1
    ldp x10, x11, [sp], #16*1
    ldp x12, x13, [sp], #16*1
    ldp x14, x15, [sp], #16*1
    ldp x16, x17, [sp], #16*1
    ldp x18, x19, [sp], #16*1
    ldp x20, x21, [sp], #16*1
    ldp x22, x23, [sp], #16*1
    ldp x24, x25, [sp], #16*1
    ldp x26, x27, [sp], #16*1
    ldp x28, x29, [sp], #16*1
    ldr x30,      [sp], #16*1    // Load lr (x30), SP is now SP + 16 (total SP + 256 from original base)
.endm

// Exception Vector Table
// Each entry is 128 bytes (0x80)
.align 11 // Align to 2^11 = 2048 bytes (0x800)
_exception_vector_table:
    // Branch to handler for Synchronous exception from Current EL using SP_EL0.
    .align 7 // Align each entry to 128 bytes
    b default_unhandled_exception // Or specific handler if SP_EL0 is used by kernel
    .fill (128 - (. - _exception_vector_table % 128)), 1, 0 // Pad to 128 bytes

    // Branch to handler for IRQ from Current EL using SP_EL0.
    .align 7
    b default_unhandled_exception
    .fill (2*128 - (. - _exception_vector_table % 256)), 1, 0

    // Branch to handler for FIQ from Current EL using SP_EL0.
    .align 7
    b default_unhandled_exception
    .fill (3*128 - (. - _exception_vector_table % 384)), 1, 0

    // Branch to handler for SError from Current EL using SP_EL0.
    .align 7
    b default_unhandled_exception
    .fill (4*128 - (. - _exception_vector_table % 512)), 1, 0

    // Branch to handler for Synchronous exception from Current EL using SP_EL1 (or SP_ELx).
    .align 7
    b sync_current_el_spx_handler
    .fill (5*128 - (. - _exception_vector_table % 640)), 1, 0

    // Branch to handler for IRQ from Current EL using SP_EL1 (or SP_ELx).
    .align 7
    b irq_current_el_spx_handler
    .fill (6*128 - (. - _exception_vector_table % 768)), 1, 0

    // Branch to handler for FIQ from Current EL using SP_EL1 (or SP_ELx).
    .align 7
    b fiq_current_el_spx_handler
    .fill (7*128 - (. - _exception_vector_table % 896)), 1, 0

    // Branch to handler for SError from Current EL using SP_EL1 (or SP_ELx).
    .align 7
    b serror_current_el_spx_handler
    .fill (8*128 - (. - _exception_vector_table % 1024)), 1, 0

    // Handlers for exceptions from Lower EL (AArch64).
    // ... (fill these similarly if you plan to handle exceptions from EL0 to EL1)
    // For now, just put unhandled for the rest
    .align 7; b default_unhandled_exception; .fill (9*128 - (. - _exception_vector_table % 1152)), 1, 0
    .align 7; b default_unhandled_exception; .fill (10*128 - (. - _exception_vector_table % 1280)), 1, 0
    .align 7; b default_unhandled_exception; .fill (11*128 - (. - _exception_vector_table % 1408)), 1, 0
    .align 7; b default_unhandled_exception; .fill (12*128 - (. - _exception_vector_table % 1536)), 1, 0
    .align 7; b default_unhandled_exception; .fill (13*128 - (. - _exception_vector_table % 1664)), 1, 0
    .align 7; b default_unhandled_exception; .fill (14*128 - (. - _exception_vector_table % 1792)), 1, 0
    .align 7; b default_unhandled_exception; .fill (15*128 - (. - _exception_vector_table % 1920)), 1, 0
    .align 7; b default_unhandled_exception; .fill (16*128 - (. - _exception_vector_table % 2048)), 1, 0


// Handlers for exceptions taken to EL1, using SP_EL1 (renamed from default_*)

// Synchronous exception handler from Current EL using SP_ELx (typically SP_EL1 for kernel)
sync_current_el_spx_handler:
    save_gprs_lr            // Save x0-x30. SP is now current_sp - 256. Stack: [GPRs]

    mrs x2, spsr_el1        // Get SPSR_EL1
    mrs x3, elr_el1         // Get ELR_EL1
    stp x2, x3, [sp, #-16]! // Push SPSR_EL1, ELR_EL1. SP is now current_sp - 256 - 16. Stack: [SPSR,ELR][GPRs]
                            // The order on stack is SPSR_EL1 (at SP+0), ELR_EL1 (at SP+8)

    mov x1, sp              // Arg1 for c_sync_handler: pointer to context (points to SPSR_EL1 on stack)
    mrs x0, esr_el1         // Arg0 for c_sync_handler: ESR_EL1
    bl c_sync_handler       // Call C handler: c_sync_handler(esr_el1, context_ptr)

    // Context might have been switched by scheduler if c_sync_handler called schedule() e.g. for SVC
    // SP might now point to a different task's stack which has SPSR,ELR,GPRs saved in the same layout.

    ldp x2, x3, [sp], #16   // Pop SPSR_EL1 into x2, ELR_EL1 into x3. SP is now current_sp - 256.
    msr spsr_el1, x2
    msr elr_el1, x3

    restore_gprs_lr         // Restore x0-x30. SP is restored to original value before save_gprs_lr.
    eret

// IRQ handler from Current EL using SP_ELx
irq_current_el_spx_handler:
    save_gprs_lr            // Save x0-x30. Stack: [GPRs]

    mrs x2, spsr_el1
    mrs x3, elr_el1
    stp x2, x3, [sp, #-16]! // Push SPSR_EL1, ELR_EL1. Stack: [SPSR,ELR][GPRs]

    mov x0, sp              // Arg0 for c_irq_handler: pointer to current context on stack
    bl c_irq_handler        // Call C handler: c_irq_handler(context_ptr)
                            // c_irq_handler will call schedule() if preemption is needed.
                            // It returns the SP of the next task to run in x0.

    mov sp, x0              // Set SP to the stack pointer of the next task to run.
                            // This SP points to the SPSR_EL1 of the next task's saved context.

    ldp x2, x3, [sp], #16   // Pop SPSR_EL1, ELR_EL1 from (potentially new) task's stack into x2, x3
                            // SP is incremented by 16, now points to the GPRs of new task.
    msr spsr_el1, x2
    msr elr_el1, x3

    restore_gprs_lr         // Restore GPRs from (potentially new) task's stack
                            // SP will be restored to new_task_stack_base + sizeof(context_state_t)
    eret

// FIQ handler from Current EL using SP_ELx
fiq_current_el_spx_handler:
    save_gprs_lr
    mrs x2, spsr_el1
    mrs x3, elr_el1
    stp x2, x3, [sp, #-16]!
    mov x0, sp
    bl minimal_fiq_print // Or a full c_fiq_handler if you implement one
    ldp x2, x3, [sp], #16
    msr spsr_el1, x2
    msr elr_el1, x3
    restore_gprs_lr
    eret

// SError handler from Current EL using SP_ELx
serror_current_el_spx_handler:
    save_gprs_lr
    mrs x2, spsr_el1
    mrs x3, elr_el1
    stp x2, x3, [sp, #-16]!
    mov x0, sp
    bl minimal_serror_print // Or a full c_serror_handler
    ldp x2, x3, [sp], #16
    msr spsr_el1, x2
    msr elr_el1, x3
    restore_gprs_lr
    eret

default_unhandled_exception:
    // Basic unhandled exception: print a message and halt.
    // This is very rudimentary.
    // We can't easily call uart_puts here without saving context and setting up C environment.
    // For now, just loop infinitely.
    // In a real scenario, you'd save minimal state and call a C panic function.
    mov x0, #0 // Indicate unhandled
    // Consider a call to a very simple assembly routine to print a character or string via UART
    // Or, if VBAR_EL1 is set up, this might be where a debugger would break.
loop_forever:
    b loop_forever

.end
