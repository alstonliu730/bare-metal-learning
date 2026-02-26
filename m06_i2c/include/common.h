#ifndef _COMMON_H
#define _COMMON_H

#include <stdint.h>
#include <stddef.h>

// Peripheral Base
#define PERIPHERAL_BASE         0xFE000000

// Bit manipulation techniques
#define LSHIFT(val, n)              ((uint64_t)(val) << (n))
#define RSHIFT(val, n)              ((uint64_t)(val) >> (n))
#define BIT(n)                      (1ULL << (n))
#define CLR_BIT(n)                  (~BIT((n)))
#define BIT_MASK(msb, lsb)          ((BIT((msb) - (lsb) + 1) - 1) << lsb)
#define CLR_MASK(msb, lsb)          (~ BIT_MASK((msb), (lsb)))
#define BITS(val, msb, lsb)         (((val) & BIT_MASK(msb, lsb)) >> lsb)

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

// Float absolute value
static inline double fabs(double x) {
    uint64_t i;
    double res;
    // copy bits from double to an integer to allow bitwise op
    memcpy(&i, &x, sizeof(i));

    // Mask out the sign bit
    i &= 0x7FFFFFFFFFFFFFFF;

    // copy bits back to a double
    memcpy(&res, &i, sizeof(x));
    return res;
}

#endif /* _COMMON_H */