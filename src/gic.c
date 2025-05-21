#include <gic.h>
#include <uart.h>  // For debug messages, if needed

void gic_init(void) {
    uart_puts("Initializing GIC...\n");

    // --- Initialize Distributor (GICD) ---
    // Disable Distributor before configuration
    *GICD_CTLR = 0;

    // For a uniprocessor system or to target only CPU0 for now:
    // Set all SPIs (Shared Peripheral Interrupts, IDs 32-1019) to target CPU0.
    // GICD_ITARGETSRn registers: each byte targets a CPU. 0x01 for CPU0.
    // We'll skip detailed SPI configuration for now, as we'll start with a PPI
    // (timer). PPIs (0-31) are private to each CPU and don't use
    // GICD_ITARGETSRn.

    // Set default priority for all interrupts (e.g., 0xA0 - lower numeric value
    // is higher priority) There are 255 priority registers, each handling 4
    // interrupts. (1020 interrupts / 4 interrupts_per_reg = 255 registers) For
    // simplicity, we can initialize a range or just what we need later. Let's
    // set a common low priority (0xA0) for a few interrupt lines as an example.
    // for (int i = 0; i < 32; ++i) { // Example: first 32 interrupts (PPIs and
    // some SPIs)
    //     uint32_t reg_offset = i / 4;
    //     uint32_t bit_shift = (i % 4) * 8;
    //     GICD_IPRIORITYRn[reg_offset] &= ~((uint32_t)0xFF << bit_shift);
    //     GICD_IPRIORITYRn[reg_offset] |= ((uint32_t)0xA0 << bit_shift);
    // }
    // A simpler approach for now: priorities will be set when enabling specific
    // interrupts.

    // Ensure all interrupts are initially disabled (cleared in GICD_ICENABLERn)
    // There are up to 32 GICD_ICENABLERn registers (for up to 1020 interrupts)
    // for (int i = 0; i < 32; ++i) {
    //    GICD_ICENABLERn[i] = 0xFFFFFFFF; // Disable all interrupts in this
    //    block
    // }
    // This is generally the default state, but explicit clearing can be good.

    // Enable Distributor (Group 1 non-secure interrupts)
    *GICD_CTLR = 1;  // Enable Group 1

    // --- Initialize CPU Interface (GICC) ---
    // Set priority mask register to allow all priorities (lowest priority
    // threshold) Any interrupt with a higher priority (lower numerical value)
    // than PMR will be signaled.
    *GICC_PMR = 0xFF;

    // Enable CPU Interface for Group 1 non-secure interrupts
    // Bit 0: Enable Group 1 interrupts
    *GICC_CTLR = 1;

    uart_puts("GIC Initialized.\n");
}

// Function to enable a specific interrupt ID in the GIC
void gic_enable_interrupt(uint32_t int_id, uint8_t core_target_mask,
                          uint8_t priority) {
    // Set interrupt priority (lower value = higher priority)
    // Each GICD_IPRIORITYRn register holds 4 interrupt priorities (8 bits each)
    uint32_t prio_reg_offset = int_id / 4;
    uint32_t prio_bit_shift = (int_id % 4) * 8;
    GICD_IPRIORITYRn[prio_reg_offset] &=
        ~((uint32_t)0xFF << prio_bit_shift);  // Clear existing priority
    GICD_IPRIORITYRn[prio_reg_offset] |=
        ((uint32_t)priority << prio_bit_shift);  // Set new priority

    // Set interrupt target (for SPIs, IDs 32 and above)
    // PPIs (0-31) are hardwired to the local CPU interface.
    if (int_id >= 32) {
        uint32_t target_reg_offset = int_id / 4;
        uint32_t target_bit_shift = (int_id % 4) * 8;
        GICD_ITARGETSRn[target_reg_offset] &=
            ~((uint32_t)0xFF << target_bit_shift);
        GICD_ITARGETSRn[target_reg_offset] |=
            ((uint32_t)core_target_mask << target_bit_shift);
    }

    // Enable the interrupt
    // Each GICD_ISENABLERn register controls 32 interrupts
    GICD_ISENABLERn[int_id / 32] = (1U << (int_id % 32));
}

// Function to read the Interrupt Acknowledge Register (IAR)
uint32_t gic_read_iar(void) { return *GICC_IAR; }

// Function to write to the End Of Interrupt Register (EOIR)
void gic_write_eoir(uint32_t int_id) { *GICC_EOIR = int_id; }