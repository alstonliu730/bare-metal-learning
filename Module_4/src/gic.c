#include <gic.h>
#include <io.h>
#include <irq.h>

#define ENABLE_GRP 0x03
#define P_MASK      0xFF
#define DISABLE 0

/**
 * Enable the Distributer Controller. 
 */
void gic_dist_init() {
    mmio_write(GICD_CTLR, ENABLE_GRP);
}

/**
 * Disable the Distributor Controller.
 */
void gic_dist_clr() {
    mmio_write(GICD_CTLR, DISABLE);
}

/**
 * Enable the CPU Interface Controller.
 */
void gic_cpu_init() {
    // Enable Group 0 & 1 Interrupts
    mmio_write(GICC_CTLR, ENABLE_GRP);

    // Set Priority mask to route interrupts through (Lower Value = Higher Priority Mask)
    mmio_write(GICC_PMR, P_MASK);

    // Set Binary Point for no priority grouping.
    mmio_write(GICC_BPR, DISABLE);
}

/**
 *  Enable the distributor to let the given interrupt signal pass.
 */
void enable_interrupt(uint32_t irq) {
    // Calculate where interrupt enable register is
    uint32_t n = irq / 32;
    uint32_t offset = irq % 32;
    long enableReg = GICD_SET_ENABLER + (4 * n);
    
    // DEBUGGING
    uart_writeText("Enabling IRQ #");
    uart_writeInt(irq);
    uart_writeText(" at 0x");
    uart_writeHex(enableReg);
    uart_writeText(".\n");

    // Enable interrupt id in the distributor
    mmio_write(enableReg, (1 << offset));
}

/**
 * Setting the priority value for the given IRQ number in the distributor.
 */
void set_irq_priority(uint32_t irq, uint32_t priority) {
    // Calculate where interrupt priority offset is
    uint32_t n = irq / 4;            // Which priority register
    uint32_t offset = (irq % 4) * 8; // bit shift by offset
    long priorityReg = GICD_PRIORITY + (4 *n);

    uint32_t val = mmio_read(priorityReg) | (priority << offset);
    // DEBUGGING
    uart_writeText("Setting IRQ #");
    uart_writeInt(irq);
    uart_writeText(" priority at 0x");
    uart_writeHex(priorityReg);
    uart_writeText(" with the value of 0x");
    uart_writeHex(priority);
    uart_writeText(", reg val: 0x");
    uart_writeHex(val);
    uart_writeText(".\n");

    // Setting priority for the given interrupt
    mmio_write(priorityReg, val);
}   

/**
 * Assign processor destination of interrupt (TEMP: Single-Core only routes to Core 0)
 * 
 * NOTE:
 * - In a uniprocessor implementation, all interrupts target the one processor, and 
 *      the GICD_ITARGETSRs are RAZ/WI (Read as Zero/ Write Ignored)
 */
void assign_target(uint32_t irq) {
    uint32_t reg_num = irq / 4; // Tells us which Target Register #
    uint32_t target = GICD_TARGET + (4 * reg_num);

    uint32_t offset = irq % 4; // Which byte in that register
    uint32_t bit_shift = offset * 8;

    uint32_t val = mmio_read(target) | (1 << bit_shift);

    // DEBUGGING
    uart_writeText("Setting IRQ #");
    uart_writeInt(irq);
    uart_writeText(" target at 0x");
    uart_writeHex(target);
    uart_writeText(" with the value of 0x");
    uart_writeHex(val);
    uart_writeText(".\n");

    // Setting target to Core 0
    mmio_write(target, val);
}

void set_configuration(uint32_t irq, gicd_cfg_flags_t flag) {
    uint32_t reg_num = irq / 16;
    uint32_t bit_pos = (irq % 16) * 2;
    uint32_t cfg_addr = GICD_ICFGR + (4 * reg_num);

    uint32_t val = mmio_read(cfg_addr) | (flag << bit_pos);

    // DEBUGGING
    uart_writeText("Configuring IRQ #");
    uart_writeInt(irq);
    uart_writeText(" target at 0x");
    uart_writeHex(cfg_addr);
    uart_writeText(" with the value of 0x");
    uart_writeHex(val);
    uart_writeText(".\n");

    mmio_write(cfg_addr, val);
}

/**
 * Writes to the End of Interrupt Register
 */
void clear_interrupt(uint32_t irq) {
    mmio_write(GICC_EOIR, irq);
}

/**
 * Function to setup a singular interrupt
 */
void setup_interrupt(uint32_t irq, uint32_t priority, gicd_cfg_flags_t flag) {
    enable_interrupt(irq);
    set_irq_priority(irq, priority);
    assign_target(irq);
    set_configuration(irq, flag);
}

/**
 * Initialization of the GIC-400
 */
void gic_init() {
    gic_dist_init();
    
    gic_cpu_init();

    setup_interrupt(SYS_TIMER_IRQ_1, 0xA0, edge_triggered);
}   