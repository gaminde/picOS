#ifndef GIC_H
#define GIC_H

#include <stdint.h>  // For uintptr_t, uint32_t, uint8_t

// Define GIC base addresses as uintptr_t
#define GICD_BASE ((uintptr_t)0x08000000)  // Distributor base address
#define GICC_BASE ((uintptr_t)0x08010000)  // CPU Interface base address

// GIC Distributor interface register OFFSETS (from GICD_BASE)
#define GICD_CTLR 0x000   // Distributor Control Register
#define GICD_TYPER 0x004  // Distributor Type Register
#define GICD_IIDR 0x008   // Distributor Implementer ID Register
#define GICD_ISENABLERn_OFFSET \
    0x100  // Interrupt Set-Enable Registers base offset
#define GICD_ICENABLERn_OFFSET \
    0x180  // Interrupt Clear-Enable Registers base offset
#define GICD_ISPENDRn_OFFSET \
    0x200  // Interrupt Set-Pending Registers base offset
#define GICD_ICPENDRn_OFFSET \
    0x280  // Interrupt Clear-Pending Registers base offset
#define GICD_IPRIORITYRn_OFFSET \
    0x400  // Interrupt Priority Registers base offset
#define GICD_ITARGETSRn_OFFSET \
    0x800  // Interrupt Processor Targets Registers base offset
#define GICD_ICFGRn_OFFSET \
    0xC00                // Interrupt Configuration Registers base offset
#define GICD_SGIR 0xF00  // Software Generated Interrupt Register

// GIC CPU interface register OFFSETS (from GICC_BASE)
#define GICC_CTLR 0x000   // CPU Interface Control Register
#define GICC_PMR 0x004    // Interrupt Priority Mask Register
#define GICC_BPR 0x008    // Binary Point Register
#define GICC_IAR 0x00C    // Interrupt Acknowledge Register
#define GICC_EOIR 0x010   // End of Interrupt Register
#define GICC_RPR 0x014    // Running Priority Register
#define GICC_HPPIR 0x018  // Highest Priority Pending Interrupt Register
#define GICC_IIDR 0x0FC   // CPU Interface Identification Register

// Function prototypes
void gic_init(void);
void gic_enable_interrupt(uint32_t int_id, uint8_t core_target_mask,
                          uint8_t priority);
uint32_t gic_read_iar(void);
void gic_write_eoir(uint32_t int_id);

#endif  // GIC_H