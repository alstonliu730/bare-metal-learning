#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <stddef.h>

#define PERIPHERAL_BASE         0xFE000000

// Bit manipulation techniques
#define SHIFT(val, n)           ((uint64_t)(val) << (n))
#define BIT(n)                  (1ULL << (n))
#define CLR_BIT(n)              (~BIT((n)))
#define BIT_MASK(msb, lsb)      ((BIT((msb) - (lsb) + 1) - 1) << lsb)
#define CLR_MASK(msb, lsb)      (~ BIT_MASK((msb), (lsb)))

// sets the value in the given address
void *memset(void *dest, int c, size_t n);
// copy the values from the source to the destination
void *memcpy(void *dest, const void *src, size_t n);

// Suspend tasks for n amount of cycles
static inline void delay(volatile uint32_t count) {
    while (count--) asm("nop");
}
#endif /* _COMMON_H */