// GPIO
#include "io.h"

#define PULL_NONE 0

// Write a value to a memory-mapped I/O register
void mmio_write(long reg, unsigned int value) {
    *(volatile unsigned int *)reg = value;
}

// Read a value from a memory-mapped I/O register
unsigned int mmio_read(long reg) {
    return *(volatile unsigned int *)reg;
}

/**
 * Set the function of a GPIO pin.
 */
unsigned int gpio_call(unsigned int pin, unsigned int value,
                     unsigned int base, unsigned int field_size, unsigned int field_max) {
    unsigned int field_mask = (1 << field_size) - 1;
    
    if (pin > field_max) return 0;
    if (value > field_mask) return 0;

    unsigned int num_field = 32 / field_size;
    unsigned int reg = base + ((pin / num_field) * 4);
    unsigned int shift = (pin % num_field) * field_size;

    unsigned int current_value = mmio_read(reg);
    current_value &= ~(field_mask << shift);
    current_value |= value << shift;
    mmio_write(reg, current_value);

    return 1;
}

// GPIO functions

// Enabling the GPIO pin to a high state
unsigned int gpio_set (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPSET0, 1, GPIO_MAX_PIN);
}

// Disabling the GPIO pin to a low state
unsigned int gpio_clear (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPCLR0, 1, GPIO_MAX_PIN);
}

// Set the pull-up/pull-down resistor for the GPIO pin
unsigned int gpio_pull (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPPUPPDN0, 2, GPIO_MAX_PIN);
}

// Defines the GPIO pin's operation mode. See Section 5.3 in BCM2711 ARM Peripherals
unsigned int gpio_function (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPFSEL0, 3, GPIO_MAX_PIN);
}

// Set the GPIO pin to use Alternate function 5
void gpio_useAlt5 (unsigned int pin) {
    gpio_pull(pin, PULL_NONE);
    gpio_function(pin, GPIO_FUNCTION_ALT5);
}

// UART functions

// Create UART output buffer
unsigned char uart_output_buffer[UART_MAX_QUEUE] __attribute__((aligned(16)));
unsigned int uart_output_buffer_write = 0;
unsigned int uart_output_buffer_read = 0;

// UART register addresses
/**
 * Initialize the UART for communication.
 */
void uart_init() {
    mmio_write(AUX_ENABLES, 1); // Enable UART
    mmio_write(AUX_MU_IER_REG, 0); // Disable interrupts
    mmio_write(AUX_MU_CNTL_REG, 0); // Disable transmitter and receiver
    mmio_write(AUX_MU_LCR_REG, 3); // Set 8 data bits
    mmio_write(AUX_MU_MCR_REG, 0); // Disable modem control
    mmio_write(AUX_MU_IIR_REG, 0xC6); // Disable interrupts
    mmio_write(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200)); // Set baud rate

    gpio_useAlt5(14); // TXD1
    gpio_useAlt5(15); // RXD1

    mmio_write(AUX_MU_CNTL_REG, 3); // Enable transmitter and receiver
}

/**
 * Checks if the UART FIFO buffer is empty.
 */
unsigned int uart_writeByteReady() {
    return mmio_read(AUX_MU_LSR_REG) & 0x20; // Check if the transmitter is ready
}

/**
 * Checks if the UART FIFO buffer is ready to read.
 */
unsigned int uart_readByteReady() {
    return mmio_read(AUX_MU_LSR_REG) & 0x01;
}

/**
 * Write a byte to the UART transmitter blockingly.
 */
void uart_writeByteBlocking(unsigned char ch) {
    while (!uart_writeByteReady());
    mmio_write(AUX_MU_IO_REG, (unsigned int)ch);
}

/**
 * Return if the buffer is empty.
 */
unsigned int uart_bufferEmpty() {
    return uart_output_buffer_write == uart_output_buffer_read;
}

/**
 * Write out the UART output buffer until it's empty or the UART is not ready.
 */
void uart_loadOutputBuffer() {
    while (!uart_bufferEmpty() && uart_writeByteReady()) {
        uart_writeByteBlocking(uart_output_buffer[uart_output_buffer_read]);
        uart_output_buffer_read = (uart_output_buffer_read + 1) % UART_MAX_QUEUE;
    }
}

/**
 * Write a byte to the UART output buffer.
 * This function will block until there is space in the buffer.
 */
void uart_writeByte(unsigned char ch) {
    unsigned int next_write = (uart_output_buffer_write + 1) % UART_MAX_QUEUE;

    while (next_write == uart_output_buffer_read) {
        // Wait until there is space in the buffer
        uart_loadOutputBuffer();
    }

    uart_output_buffer[uart_output_buffer_write] = ch;
    uart_output_buffer_write = next_write;
}

/**
 * Write a string to the UART output while handling line endings.
 */
void uart_writeText(char *text) {
    while (*text && *text != '\0') {
        if (*text == '\n') {
            uart_writeByte('\r'); // Send carriage return before line feed
        } 
        uart_writeByte(*text++); // Send character
    }
}

