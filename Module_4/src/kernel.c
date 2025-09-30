#include "../include/io.h"
#include "../include/fb.h"
#include "../include/timer.h"
#include "../include/common.h"

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

uint32_t get_timer32() {
    return mmio_read(SYS_TIMER_CLO);
}

uint64_t get_timer64() {
    uint64_t timer_hi = mmio_read(SYS_TIMER_CHI);
    uint32_t res = mmio_read(SYS_TIMER_CLO);

    res |= (timer_hi << 32);
    return res;
}

void wait(int i) {
    uint32_t start = get_timer32();
    uint32_t curr = get_timer32();
    while (curr - start < i) {
        // debugging
        uart_writeHex(curr);
        uart_writeText("\n");
        curr = get_timer32();
    }
}

void main() {
    reset();
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

    wait(100000);

    while(1) {
        uart_update();
    }
}