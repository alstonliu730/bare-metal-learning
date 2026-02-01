#include <gpio.h>
#include <common.h>

#define PULL_NONE 0
#define PULL_UP   1
#define PULL_DOWN 2

#define LED_PIN   42
// Write a value to a memory-mapped I/O register
void mmio_write(long reg, unsigned int value) {
    *(volatile unsigned int *)reg = value;
}

// Read a value from a memory-mapped I/O register
uint32_t mmio_read(long reg) {
    return *(volatile unsigned int *)reg;
}

/**
 * Set the function of a GPIO pin.
 */
uint8_t gpio_call(unsigned int pin, unsigned int value,
                     unsigned int base, unsigned int field_size, unsigned int field_max) {
    unsigned int field_mask = (1 << field_size) - 1;
    
    if (pin > field_max) return 0;
    if (value > field_mask) return 0;

    unsigned int num_field = 32 / field_size;
    unsigned int reg = base + ((pin / num_field) * 4);
    unsigned int shift = (pin % num_field) * field_size;

    unsigned int current_value = mmio_read(reg);
    current_value &= ~(field_mask << shift); // Clears the current value
    current_value |= value << shift;    // Add in the desired value
    mmio_write(reg, current_value);

    return 1;
}

// ------------ GPIO functions ------------
// Enabling the GPIO pin to a high state
uint8_t gpio_set (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPSET0, 1, GPIO_MAX_PIN);
}

// Disabling the GPIO pin to a low state
uint8_t gpio_clear (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPCLR0, 1, GPIO_MAX_PIN);
}

// Set the pull-up/pull-down resistor for the GPIO pin
uint8_t gpio_pull (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPPUPPDN0, 2, GPIO_MAX_PIN);
}

// Defines the GPIO pin's operation mode. 
// See Section 5.3 in BCM2711 ARM Peripherals
uint8_t gpio_function (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPFSEL0, 3, GPIO_MAX_PIN);
}

// Set the GPIO pin to use Alternate function 0
uint8_t gpio_useAlt0 (unsigned int pin) {
    return (gpio_pull(pin, PULL_NONE) && gpio_function(pin, GPIO_FUNCTION_ALT0));
}
// Set the GPIO pin to use Alternate function 3
uint8_t gpio_useAlt3 (unsigned int pin) {
    return (gpio_pull(pin, PULL_NONE) && gpio_function(pin, GPIO_FUNCTION_ALT3));
}

// Set the GPIO pin to use Alternate function 5
uint8_t gpio_useAlt5 (unsigned int pin) {
    return (gpio_pull(pin, PULL_NONE) && gpio_function(pin, GPIO_FUNCTION_ALT5));
}

// LED Functions
void led_init() {
    gpio_function(LED_PIN, GPIO_FUNCTION_OUT); // Built-in LED (if available)
}

void led_on() {
    gpio_set(LED_PIN, 1);
}

void led_off() {
    gpio_clear(LED_PIN, 1);
}

void led_toggle() {
    // Turns off the LED if the LED is on
    if (mmio_read(GPLEV1) & (1 << (LED_PIN - 32))) {
        led_off();
    } else {
        led_on();
    }
}
