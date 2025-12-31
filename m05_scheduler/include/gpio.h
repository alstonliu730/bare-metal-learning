#ifndef _IO_H
#define _IO_H

#include <common.h>

#define GPIO_BASE               (PERIPHERAL_BASE + 0x200000)
#define GPFSEL0                 (GPIO_BASE + 0x00)
#define GPFSEL1                 (GPIO_BASE + 0x04)
#define GPFSEL2                 (GPIO_BASE + 0x08)
#define GPFSEL3                 (GPIO_BASE + 0x0C)
#define GPFSEL4                 (GPIO_BASE + 0x10)
#define GPSET0                  (GPIO_BASE + 0x1C)
#define GPSET1                  (GPIO_BASE + 0x20)
#define GPCLR0                  (GPIO_BASE + 0x28)
#define GPCLR1                  (GPIO_BASE + 0x2C)
#define GPLEV0                  (GPIO_BASE + 0x34)
#define GPLEV1                  (GPIO_BASE + 0x38)
#define GPPUPPDN0               (GPIO_BASE + 0xE4)

#define GPIO_MAX_PIN 53

#define GPIO_FUNCTION_IN    0
#define GPIO_FUNCTION_OUT   1
#define GPIO_FUNCTION_ALT0  0b100
#define GPIO_FUNCTION_ALT1  0b101
#define GPIO_FUNCTION_ALT2  0b110
#define GPIO_FUNCTION_ALT3  0b111
#define GPIO_FUNCTION_ALT4  0b011
#define GPIO_FUNCTION_ALT5  0b010

// Read/Write functions
void mmio_write(uintptr_t reg, uint32_t value);
uint32_t mmio_read(uintptr_t reg);

// ------------ GPIO ------------
uint8_t gpio_call(unsigned int pin, unsigned int value,
                     unsigned int base, unsigned int field_size, unsigned int field_max);

uint8_t gpio_set(unsigned int pin, unsigned int value);
uint8_t gpio_clear(unsigned int pin, unsigned int value);
uint8_t gpio_pull(unsigned int pin, unsigned int value);
uint8_t gpio_function(unsigned int pin, unsigned int value);
uint8_t gpio_useAlt0(unsigned int pin);
uint8_t gpio_useAlt3(unsigned int pin);
uint8_t gpio_useAlt5(unsigned int pin);

// LED functions
void led_init();
void led_on();
void led_off();
void led_toggle();

// Suspend tasks for n amount of cycles
static inline void delay(volatile uint32_t count) {
    while (count--) asm("nop");
}

#endif /* _IO_H*/