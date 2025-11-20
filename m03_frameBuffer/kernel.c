#include "io.h"
#include "fb.h"

static void __attribute__((noinline)) start() {
    // Zero the .bss section
    extern unsigned int __bss_start, __bss_end;

    for(unsigned int* i = &__bss_start; i < &__bss_end; i++) {
        *i = 0;
    }
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

    // Test drawing characters on screen
    drawChar('O', 1000, 500, 0x05);
    drawChar('H', 1000, 508, 0x04);

    // Test drawing lines of text on screen
    drawString("Hello World!", 1400, 600, 0x04);

    // Test drawing shapes on screen
    drawLine(100, 500, 350, 700, 0x0C);

    while(1) {
        uart_update();
    }
}