#include "gic.h"

#include <stddef.h>
#include <stdint.h>

#include "common_macros.h"  // For TIMER_IRQ_ID if used directly, or GIC constants
#include "mmio.h"
#include "uart.h"  // For uart_puts, print_hex

// Remove the local #defines for GIC registers, they are now in gic.h

void gic_init(void) {
    uart_puts("Initializing GIC...\n");
    uintptr_t gicd_base = GICD_BASE;
    uintptr_t gicc_base = GICC_BASE;
    uint32_t num_irqs_to_configure;

    // Disable GIC Distributor before configuration
    mmio_write(gicd_base + GICD_CTLR, 0x00);

    // Discover the number of interrupt lines supported.
    uint32_t typer_val = mmio_read(gicd_base + GICD_TYPER);
    uint32_t num_irq_groups = (typer_val & 0x1F);
    num_irqs_to_configure = (num_irq_groups + 1) * 32;

    if (num_irqs_to_configure > 1020) {  // Cap at GICv2 spec max
        num_irqs_to_configure = 1020;
    }
    uart_puts("GICD_TYPER reports ");
    print_uint(num_irqs_to_configure);
    uart_puts(" IRQs\n");

    // Disable all interrupts, clear pending status
    uart_puts(
        "Disabling all IRQs via GICD_ICENABLERn...\n");  // <<< ADDED PRINT
    for (uint32_t i = 0; i < (num_irqs_to_configure / 32); ++i) {
        mmio_write(gicd_base + GICD_ICENABLERn_OFFSET + i * 4,
                   0xFFFFFFFF);  // Write to ICENABLER to disable

        // Diagnostic: Verify by reading ISENABLER immediately after trying to
        // clear it
        uint32_t isenabler_val =
            mmio_read(gicd_base + GICD_ISENABLERn_OFFSET + i * 4);
        uart_puts("  GICD_ISENABLER");
        print_uint(i);
        uart_puts(" after disable op: 0x");
        print_hex(isenabler_val);
        uart_puts("\n");  // <<< ADDED PRINT

        mmio_write(gicd_base + GICD_ICPENDRn_OFFSET + i * 4,
                   0xFFFFFFFF);  // Clear pending
    }

    // Set a default priority (e.g., 0xA0) for all SPIs (Shared Peripheral
    // Interrupts, ID 32-1019) And target all SPIs to CPU0 by default.
    for (uint32_t irq_id_base = 32; irq_id_base < num_irqs_to_configure;
         irq_id_base += 4) {
        mmio_write(gicd_base + GICD_IPRIORITYRn_OFFSET + (irq_id_base / 4) * 4,
                   0xA0A0A0A0);
        mmio_write(gicd_base + GICD_ITARGETSRn_OFFSET + (irq_id_base / 4) * 4,
                   0x01010101);
    }

    // Enable GIC Distributor (Enable Group 1 Non-secure)
    mmio_write(gicd_base + GICD_CTLR, 0x01);  // Enable Group 1 non-secure
    uart_puts("GIC Distributor initialized. GICD_CTLR: 0x1\n");

    // Diagnostic: Read GICD_ICFGR1 for IRQ 30 configuration
    // GICD_ICFGR1 is for IRQs 16-31. Offset is GICD_ICFGRn_OFFSET + 1*4
    uint32_t icfgr1_val = mmio_read(gicd_base + GICD_ICFGRn_OFFSET + 4);
    uart_puts("GICD_ICFGR1 (IRQs 16-31 config): 0x");
    print_hex(icfgr1_val);
    uart_puts("\n");
    // IRQ 30 is (30 % 16) = 14th IRQ in this register. Config is 2 bits:
    // [2*14+1 : 2*14] = [29:28]
    uint32_t irq30_config_bits = (icfgr1_val >> 28) & 0x3;
    uart_puts("  IRQ 30 raw config bits [29:28] from ICFGR1: 0x");
    print_hex(irq30_config_bits);
    // For GICv2 PPIs: bit [2m+1] is RAZ/WI. Bit [2m] is RO. 0=level, 1=edge.
    // So we expect bit 29 to be 0, and bit 28 to be 0 for level-sensitive.
    // Expected 0b00.
    if ((irq30_config_bits & 0x1) == 0)
        uart_puts(" (Level-sensitive)\n");
    else
        uart_puts(" (Edge-triggered)\n");

    // Diagnostic: Read GICD_ITARGETSR7 for IRQ 30 target
    // GICD_ITARGETSR7 is for IRQs 28-31. Offset is GICD_ITARGETSRn_OFFSET + (7
    // * 4)
    uint32_t itargetsr7_val =
        mmio_read(gicd_base + GICD_ITARGETSRn_OFFSET + (7 * 4));
    uart_puts("GICD_ITARGETSR7 (IRQs 28-31 target): 0x");
    print_hex(itargetsr7_val);
    uart_puts("\n");
    // IRQ 30 is (30 % 4) = 2nd byte in this register (0-indexed). This is bits
    // [23:16].
    uint32_t irq30_target_byte = (itargetsr7_val >> 16) & 0xFF;
    uart_puts("  IRQ 30 target byte from ITARGETSR7: 0x");
    print_hex(irq30_target_byte);
    uart_puts(" (Expected 0x01 for CPU0)\n");

    // Initialize GIC CPU Interface
    mmio_write(gicc_base + GICC_PMR, 0xF0);
    mmio_write(gicc_base + GICC_BPR, 0x00);
    mmio_write(gicc_base + GICC_CTLR, 0x01);

    uart_puts(
        "GIC CPU Interface initialized. GICC_CTLR: 0x1, GICC_PMR: 0xF0, "
        "GICC_BPR: 0x0\n");
    uart_puts("GIC Initialized.\n");
}

void gic_enable_interrupt(uint32_t int_id, uint8_t core_target_mask,
                          uint8_t priority) {
    uintptr_t gicd_base = GICD_BASE;
    uint32_t prio_reg_addr;
    uint32_t current_prio_val;
    uint32_t prio_byte_offset_in_reg;

    // 1. Set Interrupt Priority (GICD_IPRIORITYRn)
    // Each register holds 4 priority fields (8-bits each)
    prio_reg_addr = gicd_base + GICD_IPRIORITYRn_OFFSET + (int_id / 4) * 4;
    prio_byte_offset_in_reg = (int_id % 4);  // Byte offset: 0, 1, 2, or 3

    current_prio_val = mmio_read(prio_reg_addr);
    current_prio_val &=
        ~(0xFF << (prio_byte_offset_in_reg * 8));  // Clear existing priority
    current_prio_val |= ((uint32_t)priority
                         << (prio_byte_offset_in_reg * 8));  // Set new priority
    mmio_write(prio_reg_addr, current_prio_val);

    // 2. Set Interrupt Processor Target (GICD_ITARGETSRn) - Only for SPIs (ID
    // >= 32)
    if (int_id >= 32) {
        uint32_t target_reg_addr;
        uint32_t current_target_val;
        uint32_t target_byte_offset_in_reg;

        target_reg_addr = gicd_base + GICD_ITARGETSRn_OFFSET + (int_id / 4) * 4;
        target_byte_offset_in_reg = (int_id % 4);  // Byte offset

        current_target_val = mmio_read(target_reg_addr);
        current_target_val &= ~(0xFF << (target_byte_offset_in_reg * 8));
        current_target_val |=
            ((uint32_t)core_target_mask << (target_byte_offset_in_reg * 8));
        mmio_write(target_reg_addr, current_target_val);
    }

    // 3. Enable Interrupt (GICD_ISENABLERn)
    // Each register controls 32 interrupts
    uint32_t enable_reg_addr =
        gicd_base + GICD_ISENABLERn_OFFSET + (int_id / 32) * 4;
    uint32_t enable_bit = (1U << (int_id % 32));

    mmio_write(enable_reg_addr, enable_bit);

    // Diagnostic: Read back the enable register immediately
    uint32_t isenabler_val_after_write = mmio_read(enable_reg_addr);
    uart_puts("  In gic_enable_interrupt for IRQ ");
    print_uint(int_id);
    uart_puts(": Wrote 0x");
    print_hex(enable_bit);
    uart_puts(" to GICD_ISENABLERn (addr 0x");
    print_hex(enable_reg_addr);
    uart_puts(")\n");
    uart_puts("  Read back GICD_ISENABLERn: 0x");
    print_hex(isenabler_val_after_write);
    if (isenabler_val_after_write & enable_bit) {
        uart_puts(" (Bit successfully set)\n");
    } else {
        uart_puts(" (BIT NOT SET - WRITE FAILED?)\n");
    }

    // Original print statement for enabling IRQ
    uart_puts("Enabled IRQ: ");
    print_uint(int_id);
    uart_puts(" Priority: 0x");
    print_hex(priority);
    if (int_id >= 32) {
        uart_puts(" Target: 0x");
        print_hex(core_target_mask);
    }
    uart_puts("\n");
}

uint32_t gic_read_iar(void) {
    uintptr_t gicc_base = GICC_BASE;
    return mmio_read(gicc_base +
                     GICC_IAR);  // Read Interrupt Acknowledge Register
}

void gic_write_eoir(uint32_t int_id) {
    uintptr_t gicc_base = GICC_BASE;
    mmio_write(gicc_base + GICC_EOIR,
               int_id);  // Write to End Of Interrupt Register
}

// Make sure this is the end of the file or before any other file-level
// static/global definitions if any.