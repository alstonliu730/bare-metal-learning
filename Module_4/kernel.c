#include "io.h"
#include "fb.h"
#include "common.h"

static void __attribute__((noinline)) start() {
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
    start();
    led_init();
    led_on();
    delay(1000);
    led_off();

    led_on();
    uart_init();
    uart_writeText("UART Initialized\n");
    delay(1000);
    led_off();
    delay(1000);

    led_on();
    fb_init();
    delay(1000);
    led_off();
    uart_writeText("Frame Buffer Initialized\n");

    uint32_t curr_el = get_el();
    uint32_t curr_daif = get_daif();

    uart_writeText("Current El: ");
    uart_writeInt(curr_el);
    uart_writeText("\n");

    uart_writeText("Current DAIF: ");
    uart_writeInt(curr_daif);
    uart_writeText("\n");

    while(1) {
        uart_update();
    }
}