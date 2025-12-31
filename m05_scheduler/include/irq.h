#ifndef _IRQ_H
#define _IRQ_H

#include <common.h>
#define PACTL_CS            0xFE204E00
#define ARMC_BASE           (PERIPHERAL_BASE + 0xB000)

// IRQ0 Registers (Legacy)
typedef struct {
    volatile uint32_t IRQ0_PENDING0;
    volatile uint32_t IRQ0_PENDING1;
    volatile uint32_t IRQ0_PENDING2;
    volatile uint32_t res0;
    volatile uint32_t IRQ0_ENABLE_0;
    volatile uint32_t IRQ0_ENABLE_1;
    volatile uint32_t IRQ0_ENABLE_2;
    volatile uint32_t res1;
    volatile uint32_t IRQ0_DISABLE_0;
    volatile uint32_t IRQ0_DISABLE_1;
    volatile uint32_t IRQ0_DISABLE_2;
} armc_irq0_regs;

#define IRQ0_REGS ((armc_irq0_regs *) (ARMC_BASE + 0x200))

#define IRQ_STATUS0         (ARMC_BASE + 0x230)
#define IRQ_STATUS1         (ARMC_BASE + 0x234)
#define IRQ_STATUS2         (ARMC_BASE + 0x238)

// Interrupt IRQ IDs
#define VC_IRQ_BASE_ID         0x60
#define SYS_TIMER_IRQ_0     (VC_IRQ_BASE_ID + 0x00)
#define SYS_TIMER_IRQ_1     (VC_IRQ_BASE_ID + 0x01)
#define SYS_TIMER_IRQ_2     (VC_IRQ_BASE_ID + 0x02)
#define SYS_TIMER_IRQ_3     (VC_IRQ_BASE_ID + 0x03)

#define PL011_UART_IRQ      (VC_IRQ_BASE_ID + 0x39)

// Functions
void exception_report(uint64_t type, uint64_t esr_reg, uint64_t elr, uint64_t spsr);
void irq_el1h_handler();

void irq_enable();
void irq_disable();
void irq_barrier();

#endif /* _IRQ_H */