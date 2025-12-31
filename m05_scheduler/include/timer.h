#ifndef _TIMER_H
#define _TIMER_H

#include <common.h>

// System Timer Addresses
#define SYS_TIMER_BASE      (PERIPHERAL_BASE + 0x3000)
#define SYS_TIMER_CS        (SYS_TIMER_BASE + 0x00)
#define SYS_TIMER_CLO       (SYS_TIMER_BASE + 0x04)
#define SYS_TIMER_CHI       (SYS_TIMER_BASE + 0x08)
#define SYS_TIMER_C0        (SYS_TIMER_BASE + 0x0C)
#define SYS_TIMER_C1        (SYS_TIMER_BASE + 0x10)
#define SYS_TIMER_C2        (SYS_TIMER_BASE + 0x14)
#define SYS_TIMER_C3        (SYS_TIMER_BASE + 0x18)

// Timer 1 Delay (1 sec)
#define CLOCK_HZ            1000000

uint32_t get_timer32();
uint64_t get_timer64();
void timer_wait(uint32_t ms);
void timer_init();

void handle_timer1();

#endif /* _TIMER_H */