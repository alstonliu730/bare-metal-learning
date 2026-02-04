#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <stddef.h>

// Peripheral Base
#define PERIPHERAL_BASE         0xFE000000

// Bit manipulation techniques
#define LSHIFT(val, n)           ((uint64_t)(val) << (n))
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

// Write a value to a memory-mapped I/O register
static inline void mmio_write(uintptr_t reg, uint32_t value) {
    *(volatile unsigned int *)reg = value;
}

// Read a value from a memory-mapped I/O register
static inline uint32_t mmio_read(uintptr_t reg) {
    return *(volatile unsigned int *)reg;
}

// Core Speed
extern uint64_t core_clock;

#endif /* _COMMON_H */