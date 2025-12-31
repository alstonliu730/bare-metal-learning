#include <gpio.h>
#include <fb.h>
#include <uart.h>
#include <irq.h>
#include <gic.h>
#include <timer.h>
#include <common.h>
#include <mb.h>
#include <sched.h>

// Returns the current Exception Level
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

// Get ARM Memory
void print_arm_memory() {
    // Setting mbox array
    mbox[0] = 8 * 4; // Size in Bytes
    mbox[1] = MBOX_REQUEST; // REQUEST TAG

    mbox[2] = MBOX_TAG_ARM_MEM; // Tag to get the arm memory address
    mbox[3] = 8;
    mbox[4] = MBOX_REQUEST;
    mbox[5] = 0;            // Base Address in Bytes
    mbox[6] = 0;            // size in Bytes
    mbox[7] = MBOX_TAG_LAST;

    if(mbox_call(MBOX_CH_PROP) && mbox[1] == MBOX_SUCCESS) {
        uart_writeText("ARM Base Address: 0x");
        uart_writeHex(mbox[5]);
        uart_writeText(";\nARM Memory Size: 0x");
        uart_writeHex(mbox[6]);
        uart_writeText(";\nARM End Address: 0x");
        uart_writeHex((mbox[5] + mbox[6]));
        uart_writeText(";\n");
    }
}

// Get VC Memory
void print_vc_memory() {
    // Setting mbox array
    mbox[0] = 8 * 4; // Size in Bytes
    mbox[1] = MBOX_REQUEST; // REQUEST TAG

    mbox[2] = MBOX_TAG_VC_MEM; // Tag to get the arm memory address
    mbox[3] = 8;
    mbox[4] = MBOX_REQUEST;
    mbox[5] = 0;            // Base Address in Bytes
    mbox[6] = 0;            // size in Bytes
    mbox[7] = MBOX_TAG_LAST;
    if(mbox_call(MBOX_CH_PROP) && mbox[1] == MBOX_SUCCESS) {
        uart_writeText("VC Base Address: 0x");
        uart_writeHex(mbox[5]);
        uart_writeText(";\nVC Memory Size: 0x");
        uart_writeHex(mbox[6]);
        uart_writeText(";\nVC End Address: 0x");
        uart_writeHex((mbox[5] + mbox[6]));
        uart_writeText(";\n");
    }
}

void main() {
    // Enable LEDs
    led_init();

    // Enable IRQs
    irq_enable();
    
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
    timer_wait(500);

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

    // Print Information
    print_arm_memory();
    print_vc_memory();

    while(1) {
        // schedule();
    }
}