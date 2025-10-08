#include <irq.h>
#include <io.h>

/**
 * Reports which interrupt is set and the exception information on the invalid entry.
 */
void exception_report(uint64_t type, uint64_t esr, uint64_t elr ,uint64_t spsr) {
    uart_writeText("\nType: ");
    uart_writeHex(type);

    uart_writeText("\nESR REG:");
    uart_writeHex(esr);

    uart_writeText("\nELR REG: ");
    uart_writeHex(elr);

    uart_writeText("\nSPSR REG: ");
    uart_writeHex(spsr);
}

/**
 * Interrupt Controller that controls the behavior of each interrupt source.
 */
void irq_el1h_handler() {
    uint32_t irq_pending = mmio_read(IRQ0_REGS->IRQ0_PENDING0);
    
    // DEBUGGING
    uart_writeText("IRQ! Pending: 0x");
    uart_writeHex(irq_pending);
    uart_writeText("\n");

    // SYSTEM TIMER 1                                                                                                                            
    if (irq_pending & SYS_TIMER_IRQ_1) {
        // Clear the Timer Interrupt to CS Register
        mmio_write(SYS_TIMER_CS, SYS_TIMER_IRQ_1);

        // Handle Timer Tick
        uart_writeText("Timer Fired!\n");

        // Set Next Compare Value
        uint32_t curr = mmio_read(SYS_TIMER_CLO);
        mmio_write(SYS_TIMER_C1, curr + SYS_TIMER_1_CMP);
    }
}