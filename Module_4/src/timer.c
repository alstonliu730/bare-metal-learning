#include <timer.h>
#include <irq.h>
#include <gpio.h>
#include <gic.h>

uint32_t get_timer32() {
    return mmio_read(SYS_TIMER_CLO);
}

uint64_t get_timer64() {
    uint64_t timer_hi = mmio_read(SYS_TIMER_CHI);
    uint32_t res = mmio_read(SYS_TIMER_CLO);

    res |= (timer_hi << 32);
    return res;
}

void timer_wait(int ms) {
    uint32_t start = get_timer32();
    uint32_t curr = start;
    
    while (curr - start < (ms * 1000)) {
        curr = get_timer32();
    }
}

/**
 * Initialization of the System Timer Interrupt via the GIC.
 */
void timer_init() {
     // set compare value to 1 sec delay
    uint32_t curr = mmio_read(SYS_TIMER_CLO);
    mmio_write(SYS_TIMER_C1, curr + CLOCK_HZ);

    mmio_write(IRQ0_REGS->IRQ0_ENABLE_0, 0x2);  // enable timer 1 bit
}

/**
 * Handle Timer 1 interrupt by setting a delay of 1 second.
 */
void handle_timer1() {
    // Set Next Compare Value
    uint32_t curr = mmio_read(SYS_TIMER_CLO);
    mmio_write(SYS_TIMER_C1, curr + CLOCK_HZ);

    // Clear the Timer Interrupt to CS Register
    mmio_write(SYS_TIMER_CS, 0x2);
}