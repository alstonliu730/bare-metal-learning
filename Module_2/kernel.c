#include "io.h"

void main() {
    led_init();
    led_on();

    for(int i = 0; i < 15000; i++) asm volatile("nop");

    uart_init();
    led_off();

    for(int i = 0; i < 15000; i++) asm volatile("nop");
    uart_writeText("Hello from the kernel\n");
    
    while (1) uart_update();
}