#include <gpio.h>
#include <fb.h>
#include <uart.h>
#include <mini_uart.h>
#include <irq.h>
#include <gic.h>
#include <timer.h>
#include <common.h>

static void __attribute__((noinline)) reset() {
    // Zero the .bss section
    extern unsigned int __bss_start, __bss_end;

    for(unsigned int* i = &__bss_start; i < &__bss_end; i++) {
        *i = 0;
    }
}

// First, figure out where you are
uint32_t get_el() {
    uint32_t el;
    asm volatile("mrs %0, CurrentEL" : "=r"(el));
    return (el >> 2) & 0x3;
}

// Check if interrupts are masked
uint32_t get_daif() {
    uint32_t daif;
    asm volatile("mrs %0, DAIF" : "=r"(daif));
    return daif;  // Bit 7 = IRQ mask
}

void main() {
    reset();
    led_init();
    
    // GIC Initialization
    led_on();
    gic_init();
    delay(1000);
    led_off();

    // Timer Initialization
    timer_init();

    // UART 0 Initialization
    led_on();
    uart_init();
    timer_wait(1000);

    uart_writeText("PL011 UART 0 Initialized\n");
    
    timer_wait(1000);
    led_off();

    // Frame Buffer Initialization
    led_on();
    fb_init();
    timer_wait(1000);
    uart_writeText("Frame Buffer Initialized\n");
    led_off();
    
    timer_wait(1000);
    
    while(1) {

    }
}