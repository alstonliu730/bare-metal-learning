#include <timer.h>
#include <irq.h>
#include <gpio.h>
#include <gic.h>
#include <sched.h>

const uint32_t interval = 200000;

uint32_t get_timer32() {
    return mmio_read(SYS_TIMER_CLO);
}

uint64_t get_timer64() {
    return (uint64_t) ((((uint64_t) mmio_read(SYS_TIMER_CHI)) << 32) | mmio_read(SYS_TIMER_CLO));
}

void wait_ms(uint64_t ms) {
    uint64_t start = get_timer64();
    uint64_t curr = start;
    
    while (curr - start < (ms * 1000)) {
        curr = get_timer64();
    }
}

void wait_us(uint64_t us) {
    uint64_t start = get_timer64();
    uint64_t curr = start;
    
    while (curr - start < us) {
        curr = get_timer64();
    }
}

/**
 * Initialization of the System Timer 1 Interrupt via the GIC.
 */
void timer1_init() {
    // set compare value to 1 sec delay
    uint32_t curr = mmio_read(SYS_TIMER_CLO);
    mmio_write(SYS_TIMER_C1, curr + CLOCK_HZ);

    mmio_write(IRQ0_REGS->IRQ0_ENABLE_0, 0x2);  // enable timer 1 bit
}

/**
 * Initialization of the System Timer 2 Interrupt 
 */
void timer2_init() {
    // Set compare value to 200 msec
    uint32_t curr = mmio_read(SYS_TIMER_CLO);
    mmio_write(SYS_TIMER_C2, curr + interval);

    mmio_write(IRQ0_REGS->IRQ0_ENABLE_0, 0x3); // enable timer 2 bit
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

void handle_timer2() {
    // Set Next Compare Value
    uint32_t curr = mmio_read(SYS_TIMER_CLO);
    mmio_write(SYS_TIMER_C2, curr + interval);

    timer_tick(); // update scheduler

    // Clear the Timer Interrupt to CS Register
    mmio_write(SYS_TIMER_CS, 0x3);
}