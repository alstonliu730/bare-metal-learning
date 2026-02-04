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

// Get Core Clock Speed
uint32_t get_core_clk() {
    // set mbox array
    mbox[0] = 8 * sizeof(uint32_t);
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_GETCLK;
    mbox[3] = 8;
    mbox[4] = MBOX_REQUEST;
    mbox[5] = MBOX_CLK_CORE;
    mbox[6] = 0;
    mbox[7] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP) && mbox[1] == MBOX_SUCCESS) {
        return mbox[6];
    }
    return 0;
}

// Get ARM Memory
static void print_arm_memory() {
    // Setting mbox array
    mbox[0] = 8 * sizeof(uint32_t); // Size in Bytes
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
    mbox[0] = 8 * sizeof(uint32_t); // Size in Bytes
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
    led_off();
    
    wait_ms(1000);
    
    // Memory Allocator 
    allocator_init();
    print_pool_boundaries();

    // DHT11 Temperature Readings
    int* dht_data = (int *) malloc(sizeof(int) * MAX_DHT_INPUT);
    
    // Clear screen without clearing previous output
    for(int i = 0; i < 16; i++) {
        uart_printf("\n");
    }
    
    while (1) {
        uart_printf("\033[2A");  // Cursor to home position
        
        if (read_dht11_data(dht_data) == 0) {
            uint32_t f_temp = (dht_data[2] << 8) | dht_data[3];
            f_temp *= 9;
            f_temp /= 5;
            f_temp += (32 << 8);
            uint32_t f_temp_frac = f_temp & BIT_MASK(7, 0);
            uint32_t f_temp_int = (f_temp & BIT_MASK(15, 8)) >> 8;
            uart_printf("Temp:  %d.%d C (%d.%d F)  \n", dht_data[2], dht_data[3], f_temp_int, f_temp_frac);
            uart_printf("Humid: %d.%d %%  \n", dht_data[0], dht_data[1]);
        } else {
            uart_printf("Read failed      \n");
            uart_printf("                 \n");
        }
        
        wait_ms(2000);  // DHT11 needs ~1-2 sec between reads
    }
}