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

// External declaration for uart_puts (defined in boot.s)
// The linker will resolve this.
.extern uart_puts

default_sync_handler:
    adr x0, sync_msg  // Load address of the message for this handler
    bl uart_puts      // Call uart_puts
handler_loop_sync:
    b handler_loop_sync // Infinite loop

default_irq_handler:
    adr x0, irq_msg
    bl uart_puts
handler_loop_irq:
    b handler_loop_irq

default_fiq_handler:
    adr x0, fiq_msg
    bl uart_puts
handler_loop_fiq:
    b handler_loop_fiq

default_serror_handler:
    adr x0, serror_msg
    bl uart_puts
handler_loop_serror:
    b handler_loop_serror

// Message strings for the handlers
// These will be placed in the .rodata section by the assembler/linker
.section .rodata
sync_msg:   .asciz "Default Synchronous Handler Reached!\n"
irq_msg:    .asciz "Default IRQ Handler Reached!\n"
fiq_msg:    .asciz "Default FIQ Handler Reached!\n"
serror_msg: .asciz "Default SError Handler Reached!\n"
