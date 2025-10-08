#ifndef IRQ_H
#define IRQ_H

#include <common.h>

#define ARMC_BASE           (PERIPHERAL_BASE + 0xB000)

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

#define SYS_TIMER_IRQ_0     1
#define SYS_TIMER_IRQ_1     2
#define SYS_TIMER_IRQ_2     4
#define SYS_TIMER_IRQ_3     8
#define AUX_IRQ             (1 << 29)

#define SYS_TIMER_BASE      (PERIPHERAL_BASE + 0x3000)
#define SYS_TIMER_CS        (SYS_TIMER_BASE + 0x00)
#define SYS_TIMER_CLO       (SYS_TIMER_BASE + 0x04)
#define SYS_TIMER_CHI       (SYS_TIMER_BASE + 0x08)
#define SYS_TIMER_C0        (SYS_TIMER_BASE + 0x0C)
#define SYS_TIMER_C1        (SYS_TIMER_BASE + 0x10)
#define SYS_TIMER_C2        (SYS_TIMER_BASE + 0x14)
#define SYS_TIMER_C3        (SYS_TIMER_BASE + 0x18)

#define SYS_TIMER_1_CMP     1000000

void exception_report(uint64_t type, uint64_t esr_reg, uint64_t elr ,uint64_t spsr);
void irq_el1h_handler();
#endif /* IRQ_H */