#include <stdint.h>

#include "common_macros.h"  // For KERNEL_TIMER_INTERVAL_MS
#include "exceptions.h"     // For exceptions_init
#include "gic.h"            // For gic_init
#include "task.h"           // For task_init_system
#include "timer.h"          // For timer_init
#include "uart.h"

// External function to enable interrupts (defined in boot.S or similar)
extern void enable_interrupts(void);
// External function to disable interrupts (defined in boot.S or similar)
// extern void disable_interrupts(void); // If you have one

void kernel_main(void) {
    uart_init();
    uart_puts("picOS Kernel Initializing...\n");

    exceptions_init();  // Initialize exception vector table
    gic_init();         // Initialize GIC
    timer_init(
        KERNEL_TIMER_INTERVAL_MS);  // Initialize timer, e.g., for 10ms interval
    task_init_system();   // Initialize tasking system (placeholders for now)
    enable_interrupts();  // Clears PSTATE.I bit, enabling IRQs/FIQs

    // uart_puts("Entering diagnostic loop to check timer ISTATUS and GIC
    // state...\n"); // <<< REMOVE THIS LINE uint32_t loop_count = 0; // <<<
    // REMOVE THIS LINE const uint32_t max_loops = 5; // <<< REMOVE THIS LINE

    // while(loop_count < max_loops) { // <<< REMOVE THIS BLOCK
    //     uint64_t ctl_val = read_cntp_ctl_el0(); // <<< REMOVE THIS BLOCK if
    //     (ctl_val & 0x4) { // Check ISTATUS bit (bit 2) // <<< REMOVE THIS
    //     BLOCK
    //         uart_puts("--- Timer ISTATUS IS SET! CNTP_CTL_EL0: 0x");
    //         print_hex(ctl_val); uart_puts(" ---\n"); // <<< REMOVE THIS BLOCK

    //         uint32_t gicd_isenabler0_val = mmio_read(GICD_BASE +
    //         GICD_ISENABLERn_OFFSET); // <<< REMOVE THIS BLOCK uart_puts("
    //         GICD_ISENABLER0: 0x"); print_hex(gicd_isenabler0_val);     // <<<
    //         REMOVE THIS BLOCK if (gicd_isenabler0_val & (1U << TIMER_IRQ_ID))
    //         {                       // <<< REMOVE THIS BLOCK
    //             uart_puts(" (IRQ 30 ENABLED in GICD)\n"); // <<< REMOVE THIS
    //             BLOCK
    //         } else { // <<< REMOVE THIS BLOCK
    //             uart_puts(" (IRQ 30 NOT enabled in GICD) <<-- PROBLEM!\n");
    //             // <<< REMOVE THIS BLOCK
    //         } // <<< REMOVE THIS BLOCK

    //         uint32_t gicd_ispendr0_val = mmio_read(GICD_BASE +
    //         GICD_ISPENDRn_OFFSET); // <<< REMOVE THIS BLOCK uart_puts("
    //         GICD_ISPENDR0: 0x"); print_hex(gicd_ispendr0_val);         // <<<
    //         REMOVE THIS BLOCK if (gicd_ispendr0_val & (1U << TIMER_IRQ_ID)) {
    //         // <<< REMOVE THIS BLOCK
    //             uart_puts(" (IRQ 30 PENDING in GICD)\n"); // <<< REMOVE THIS
    //             BLOCK
    //         } else { // <<< REMOVE THIS BLOCK
    //             uart_puts(" (IRQ 30 NOT pending in GICD)\n"); // <<< REMOVE
    //             THIS BLOCK
    //         } // <<< REMOVE THIS BLOCK

    //         uint32_t gicc_hppir_val = mmio_read(GICC_BASE + GICC_HPPIR); //
    //         <<< REMOVE THIS BLOCK uart_puts("  GICC_HPPIR: 0x");
    //         print_hex(gicc_hppir_val);               // <<< REMOVE THIS BLOCK
    //         uart_puts(" (IRQ ID: "); print_uint(gicc_hppir_val & 0x3FF);
    //         uart_puts(")\n"); // <<< REMOVE THIS BLOCK
    //     } else { // <<< REMOVE THIS BLOCK
    //         // uart_puts("--- Timer ISTATUS is NOT set. CNTP_CTL_EL0: 0x");
    //         print_hex(ctl_val); uart_puts(" ---\n"); // <<< REMOVE THIS BLOCK
    //     } // <<< REMOVE THIS BLOCK for(volatile int i=0; i<1000000; ++i); //
    //     Simple delay                      // <<< REMOVE THIS BLOCK
    //     loop_count++; // <<< REMOVE THIS BLOCK
    // } // <<< REMOVE THIS BLOCK uart_puts("Diagnostic checks complete.\n"); //
    // <<< REMOVE THIS LINE

    uart_puts("Entering WFI loop...\n");
    while (1) {
        __asm__ __volatile__("wfi");  // Wait For Interrupt
    }
}