#ifndef TIMER_H
#define TIMER_H

#include <common.h>

uint32_t get_timer32();
uint64_t get_timer64();
void wait(int i);
void timer_init();

#endif /* TIMER_H */