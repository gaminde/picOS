#include <gic.h>  // For TIMER_IRQ_ID
#include <stdint.h>
#include <timer.h>
#include <uart.h>  // For debug messages

// Reads the frequency of the system counter (CNTFRQ_EL0)
static uint32_t read_cntfrq(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(val));
    return (uint32_t)val;
}

// Writes to CNTP_TVAL_EL0 (Timer Value Register)
// This sets the interval for the next interrupt.
static void write_cntp_tval(uint32_t val) {
    __asm__ volatile("msr cntp_tval_el0, %0" ::"r"((uint64_t)val));
}

// Writes to CNTP_CTL_EL0 (Timer Control Register)
static void write_cntp_ctl(uint32_t val) {
    __asm__ volatile("msr cntp_ctl_el0, %0" ::"r"((uint64_t)val));
}

// Reads CNTP_CTL_EL0
// static uint32_t read_cntp_ctl(void) {
//     uint64_t val;
//     __asm__ volatile("mrs %0, cntp_ctl_el0" : "=r"(val));
//     return (uint32_t)val;
// }

static uint32_t timer_interval_ticks;

void timer_init(uint32_t freq_hz) {
    uart_puts("Initializing EL1 Physical Timer...\n");

    uint32_t cntfrq = read_cntfrq();
    if (cntfrq == 0) {
        uart_puts("Error: CNTFRQ_EL0 is 0. Cannot initialize timer.\n");
        // On QEMU, CNTFRQ_EL0 should be set. If not, there's a deeper issue
        // or the CPU model doesn't expose it as expected.
        // For 'virt' machine, it's typically 62.5 MHz (62500000).
        // Let's assume a default if it's zero for now, though this is a
        // workaround.
        uart_puts("Assuming CNTFRQ_EL0 = 62.5 MHz for QEMU virt.\n");
        cntfrq = 62500000;  // Default for QEMU virt if not readable
    }

    uart_puts("  System Counter Frequency (CNTFRQ_EL0) read.\n");
    // print_hex(cntfrq); uart_puts(" Hz\n");

    timer_interval_ticks = cntfrq / freq_hz;
    uart_puts("  Timer interval calculated.\n");
    // print_hex(timer_interval_ticks); uart_puts("\n");

    write_cntp_tval(timer_interval_ticks);  // Set initial interval
    write_cntp_ctl(CNTP_CTL_EL0_ENABLE);  // Enable timer, unmask its interrupt

    uart_puts("EL1 Physical Timer Initialized and Enabled.\n");
}

// This function will be called from the main IRQ handler
void handle_timer_irq(void) {
    // Re-arm the timer for the next interrupt
    write_cntp_tval(timer_interval_ticks);

    // The GIC interrupt status for the timer (PPI) is usually cleared by EOI.
    // For some timer implementations, you might need to clear a status bit in
    // CNTP_CTL_EL0, but for the generic timer, setting TVAL and EOI to GIC is
    // usually enough. uint32_t ctl = read_cntp_ctl(); if (ctl &
    // CNTP_CTL_EL0_ISTATUS) {
    //    write_cntp_ctl(ctl | CNTP_CTL_EL0_ISTATUS); // Clear ISTATUS (W1C -
    //    write 1 to clear)
    // }
    // For CNTP_CTL_EL0, ISTATUS is read-only. The interrupt is cleared by GIC
    // EOI and by the timer itself when the condition (CNTPCT_EL0 >=
    // CNTP_CVAL_EL0) is no longer met, which happens after TVAL is written (as
    // TVAL sets CVAL relative to current count).
}