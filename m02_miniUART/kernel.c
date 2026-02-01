#include "io.h"

static void __attribute__((noinline)) start() {
    // Zero the .bss section
    extern unsigned int __bss_start, __bss_end;

    for(unsigned int* i = &__bss_start; i < &__bss_end; i++) {
        *i = 0;
    }
} 

void main() {
    start(); // clear bss section
    led_init(); // initialize the led

    for (int i = 0; i < 3; i++) {
        led_on(); // turn on the led
        delay(15000);
        led_off(); // turn off the led
        delay(15000);
    }

    uart_init(); // initialize the uart
    
    delay(10000); // delay for hardware to sync

    uart_writeText("Hello from the kernel\n");

    while (1) uart_update();
}