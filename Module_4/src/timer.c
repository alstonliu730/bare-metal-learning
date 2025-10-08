#include <timer.h>
#include <irq.h>
#include <io.h>

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
        curr = get_timer32();
    }
}

void timer_init() {
    mmio_write(IRQ0_REGS->IRQ0_ENABLE_0, 0x2);

    uint32_t curr = mmio_read(SYS_TIMER_CLO);
    mmio_write(SYS_TIMER_C1, curr + SYS_TIMER_1_CMP);

    uart_writeText("Timer initialized\n");
}