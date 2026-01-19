#include <gpio.h>
#include <fb.h>
#include <uart.h>
#include <irq.h>
#include <gic.h>
#include <timer.h>
#include <common.h>
#include <mb.h>
#include <sched.h>
#include <mmu.h>
#include <malloc.h>
#include <dht-11.h>

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
static void print_arm_memory() {
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
        uart_printf("ARM Base Address: %p\n", mbox[5]);
        uart_printf("ARM Memory Size: %p\n", mbox[6]);
        uart_printf("ARM End Address: %x\n", (mbox[5] + mbox[6]));
    }
}

// Get VC Memory
static void print_vc_memory() {
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
        uart_printf("VC Base Address: %p\n", mbox[5]);
        uart_printf("VC Memory Size: %p\n", mbox[6]);
        uart_printf("VC End Address: %x\n", (mbox[5] + mbox[6]));
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
    timer1_init();

    // UART 0 Initialization
    led_on();
    uart_init();
    wait_ms(500);
    uart_printf("\n========================================\n");
    uart_printf("Raspberry Pi 4 Bare-Metal Kernel\n");
    uart_printf("========================================\n");

    uart_printf("PL011 UART 0 Initialized...\n");
    
    wait_ms(1000);
    led_off();
    
    // MMU initialization
    led_on();
    mmu_init();
    wait_ms(500);
    led_off();

    // Frame Buffer Initialization
    led_on();
    wait_ms(1000);
    fb_init();
    uart_printf("Frame Buffer Initialized...\n");
    led_off();
    
    wait_ms(1000);
    
    // Memory Allocator 
    allocator_init();
    print_pool_boundaries();

    // DHT11 Temperature Readings
    int* dht_data = (int *) malloc(sizeof(int) * MAX_DHT_INPUT);
    uart_printf("DHT11 Data: \n");
    while(1) { 
        read_dht11_data(dht_data);
        
        // print it out
        uart_printf("\033[2A");
        uart_printf("Temperature: %d.%d C\n", dht_data[2], dht_data[3]);
        uart_printf("Humidity: %d.%d %%\n", dht_data[0], dht_data[1]);

         wait_ms(2000);  // DHT11 needs ~1-2 sec between reads
    }
}