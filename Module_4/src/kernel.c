#include <io.h>
#include <fb.h>
#include <irq.h>
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

    led_on();
    uart_init();
    uart_writeText("UART Initialized\n");
    delay(10000);
    led_off();

    led_on();
    fb_init();
    delay(10000);
    led_off();
    uart_writeText("Frame Buffer Initialized\n");

    uint32_t curr_el = get_el();
    uint32_t curr_daif = get_daif();

    uart_writeText("Running at EL");
    uart_writeInt(curr_el);
    uart_writeText("\n");

    uart_writeText("Current DAIF: 0x");
    uart_writeHex(curr_daif);
    uart_writeText("\n");

    uint64_t vbar;
    asm volatile("mrs %0, VBAR_EL1" : "=r"(vbar));
    uart_writeText("VBAR_EL1: 0x");
    uart_writeHex(vbar);
    uart_writeText("\n");

    uint64_t spsel;
    asm volatile("mrs %0, SPSel" : "=r"(spsel));
    uart_writeText("SPSel: ");
    uart_writeHex(spsel);
    uart_writeText(" (should be 1 for SP_ELx)\n");

    timer_init();
    
    uart_writeText("VC_IRQ_REGS: 0x");
    uint32_t VC_IRQ_EN = mmio_read(IRQ0_REGS->IRQ0_ENABLE_0);
    uart_writeHex(VC_IRQ_EN);
    uart_writeText("\nWaiting for interrupts...\n");

    while(1) {
        uint32_t cs = mmio_read(SYS_TIMER_CS);
        if (cs & 0x2) {  // Timer C1 matched
            uart_writeText("Timer C1 matched (polled)! CS: 0x");
            uart_writeHex(cs);
            uart_writeText("\n");
            
            // Clear and reset
            mmio_write(SYS_TIMER_CS, 0x2);
            uint32_t current = mmio_read(SYS_TIMER_CLO);
            mmio_write(SYS_TIMER_C1, current + 1000000);
        }
        uart_update();
    }
}