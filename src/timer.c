#include "timer.h"

#include <stdint.h>

#include "common_macros.h"  // For TIMER_IRQ_ID
#include "gic.h"            // For gic_enable_interrupt
#include "mmio.h"
#include "uart.h"  // For uart_puts, print_uint, print_hex

static uint64_t TIMER_INTERVAL_TICKS = 0;

// Remove 'static' to match declaration in timer.h
uint64_t read_cntp_ctl_el0(void) {
    uint64_t val;
    __asm__ __volatile__("mrs %0, cntp_ctl_el0" : "=r"(val));
    return val;
}

// Remove 'static' to match declaration in timer.h
void write_cntp_tval_el0(uint64_t val) {
    __asm__ __volatile__("msr cntp_tval_el0, %0" ::"r"(val));
}

void timer_init(uint32_t interval_ms) {
    uint64_t cntfrq;
    uint64_t ticks;

    __asm__ __volatile__("mrs %0, cntfrq_el0" : "=r"(cntfrq));
    uart_puts("  System Counter Frequency (CNTFRQ_EL0) read: ");
    print_uint(cntfrq);
    uart_puts(" Hz\n");

    ticks = (cntfrq * interval_ms) / 1000;
    TIMER_INTERVAL_TICKS = ticks;

    uart_puts("  Timer interval calculated: ");
    print_uint(ticks);
    uart_puts(" ticks for ");
    print_uint(interval_ms);
    uart_puts(" ms\n");

    write_cntp_tval_el0(ticks);

    __asm__ __volatile__("msr cntp_ctl_el0, %0" ::"r"((uint64_t)0x1));
    uint64_t ctl_val_check = read_cntp_ctl_el0();
    uart_puts("EL1 Physical Timer Initialized and Enabled. CNTP_CTL_EL0: 0x");
    print_hex(ctl_val_check);
    uart_puts("\n");

    uart_puts("Enabling timer IRQ in GIC...\n");
    gic_enable_interrupt(TIMER_IRQ_ID, 0x01, 0xA0);
    uart_puts("Timer IRQ enabling attempt complete.\n");
}

void handle_timer_irq(void) { write_cntp_tval_el0(TIMER_INTERVAL_TICKS); }