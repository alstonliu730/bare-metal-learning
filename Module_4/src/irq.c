#include <irq.h>
#include <gic.h>
#include <gpio.h>
#include <timer.h>
#include <uart.h>
#include <mini_uart.h>

#define ENABLE 1
#define DISABLE 0

/**
 * Reports which interrupt is set and the exception information on the invalid entry.
 */
void exception_report(uint64_t type, uint64_t esr, uint64_t elr ,uint64_t spsr) {
    // mUart_writeText("\nType: ");
    // mUart_writeHex(type);

    // mUart_writeText("\nESR REG:");
    // mUart_writeHex(esr);

    // mUart_writeText("\nELR REG: ");
    // mUart_writeHex(elr);

    // mUart_writeText("\nSPSR REG: ");
    // mUart_writeHex(spsr);

    // mUart_writeText("\n");
    return;
}

/**
 * Interrupt Controller that controls the behavior of each interrupt source.
 */
void irq_el1h_handler() {
    uint32_t irq_ack = mmio_read(GICC_IAR);
    uint32_t irq_num = irq_ack & 0x3FF; // first 10 bits

    // SYSTEM TIMER 1                                                                                                                             
    switch (irq_num) {
        case SYS_TIMER_IRQ_1: {
            handle_timer1(); // Timer 1 Interrupt
            break;
        }
        case PL011_UART_IRQ: {
            // OR of all UART IRQ asserts
            handle_pl011_uart();
            break;
        }
        case 1023: // Spurious
            return;
        default:
            // uart_writeText("ERROR: Unknown Interrupt ID");
            // uart_writeInt(irq_num);
            // uart_writeText("\n");
            break;
    }

    clear_interrupt(irq_ack);
}