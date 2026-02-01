#include <common.h>
#include <stdint.h>

// Sets the value as c at the given destination address
void *memset(void *dest, int c, size_t n) {
    uint8_t *p = (uint8_t*)dest;
    uint8_t value = (uint8_t)c;
    
    for (size_t i = 0; i < n; i++) {
        p[i] = value;
    }
    
    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = (uint8_t*)dest;
    const uint8_t *s = (const uint8_t*)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}