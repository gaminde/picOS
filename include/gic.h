#ifndef GIC_H
#define GIC_H

#include <stdint.h>

// GICv2 Base Addresses for QEMU 'virt' machine
#define GICD_BASE ((volatile uint32_t *)0x08000000)  // Distributor
#define GICC_BASE ((volatile uint32_t *)0x08010000)  // CPU Interface

// GIC Distributor Registers (offsets from GICD_BASE, divided by 4 for uint32_t
// array access)
#define GICD_CTLR (GICD_BASE + 0x000 / 4)   // Distributor Control Register
#define GICD_TYPER (GICD_BASE + 0x004 / 4)  // Distributor Type Register
#define GICD_ISENABLERn \
    (GICD_BASE + 0x100 / 4)  // Interrupt Set-Enable Registers (n=0-31)
#define GICD_ICENABLERn \
    (GICD_BASE + 0x180 / 4)  // Interrupt Clear-Enable Registers
#define GICD_IPRIORITYRn \
    (GICD_BASE + 0x400 / 4)  // Interrupt Priority Registers (n=0-254)
#define GICD_ITARGETSRn \
    (GICD_BASE + 0x800 / 4)  // Interrupt Processor Targets Registers

// GIC CPU Interface Registers (offsets from GICC_BASE, divided by 4 for
// uint32_t array access)
#define GICC_CTLR (GICC_BASE + 0x000 / 4)  // CPU Interface Control Register
#define GICC_PMR (GICC_BASE + 0x004 / 4)   // Interrupt Priority Mask Register
#define GICC_IAR (GICC_BASE + 0x00C / 4)   // Interrupt Acknowledge Register
#define GICC_EOIR (GICC_BASE + 0x010 / 4)  // End of Interrupt Register

// Interrupt ID for EL1 Physical Timer (PPI) - We'll use this later
#define TIMER_IRQ_ID 30  // Common for ARM Generic Timer (EL1 Physical Timer)

void gic_init(void);
void gic_enable_interrupt(uint32_t int_id, uint8_t core_target_mask,
                          uint8_t priority);
uint32_t gic_read_iar(void);
void gic_write_eoir(uint32_t int_id);

#endif  // GIC_H