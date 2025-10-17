#include <irq.h>
#include <gic.h>
#include <io.h>
#include <timer.h>

#define ENABLE 1
#define DISABLE 0

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

    uart_writeText("\n");
    return;
}

/**
 * Interrupt Controller that controls the behavior of each interrupt source.
 */
void irq_el1h_handler() {
    uint32_t irq_ack = mmio_read(GICC_IAR);
    uint32_t irq_num = irq_ack & 0x2FF;
    // DEBUGGING
    uart_writeText("IRQ Received! Acknowledge Value: 0x");
    uart_writeHex(irq_ack);
    uart_writeText("\n");

    // SYSTEM TIMER 1                                                                                                                             
    switch (irq_num) {
        case SYS_TIMER_IRQ_1: {
            handle_timer1(); // Timer 1 Interrupt
            break;
        }
        case 1023: // Spurious
            return;
        default:
            uart_writeText("ERROR: Unknown Interrupt");
            break;
    }

    clear_interrupt(irq_ack);
}