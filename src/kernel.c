#include <gic.h>
#include <kernel.h>
#include <stdint.h>
#include <timer.h>
#include <uart.h>

#define TIMER_FREQUENCY_HZ 1  // Let's start with 1 interrupt per second

void kernel_main(void) {
    __asm__ volatile("msr daifclr, #2");  // Unmask IRQs

    gic_init();  // Initialize the GIC

    // Initialize the ARM Generic Timer to fire at TIMER_FREQUENCY_HZ
    timer_init(TIMER_FREQUENCY_HZ);

    // Enable the EL1 Physical Timer interrupt in the GIC (ID 30 for 'virt'
    // machine) Priority 0xA0 (lower value is higher priority, 0x00 is highest,
    // 0xFF is lowest) Target core 0 (mask 0x01)
    gic_enable_interrupt(TIMER_IRQ_ID, 0x01, 0xA0);

    uart_puts(
        "Hello from Kernel (C) - Timer Initialized, GIC Timer IRQ Enabled!\n");

    // Loop forever, waiting for interrupts
    while (1) {
        __asm__ volatile("wfi");  // Wait for an interrupt
    }
}