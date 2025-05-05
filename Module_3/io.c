// GPIO
#include "io.h"

// Write a value to a memory-mapped I/O register
void mmio_write(long reg, unsigned int value) {
    *(volatile unsigned int *)(reg) = value;
}

// Read a value from a memory-mapped I/O register
unsigned int mmio_read(long reg) {
    return *(volatile unsigned int *)(reg);
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
    unsigned int reg = base + (pin / num_field) * 4;
    unsigned int shift = (pin % num_field) * field_size;

    unsigned int current_value = mmio_read(reg);
    current_value &= ~(field_mask << shift);
    current_value |= (value << shift);
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
/**
 * Initialize the UART for communication.
 */
void uart_init() {
    mmio_write(AUX_ENABLES, 1); // Enable UART1
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

/** * Checks if the UART Register is ready to read data.
 */
unsigned int uart_readByteReady() {
    return mmio_read(AUX_MU_LSR_REG) & 0x01; // Check if data is available
}

/**
 * Checks if the UART Register is ready to write data.
 */
unsigned int uart_writeByteReady() {
    return mmio_read(AUX_MU_LSR_REG) & 0x20; // Check if the transmitter is ready
}

/**
 * Write a byte to the UART transmitter blockingly.
 */
void uart_writeToMem(unsigned char ch) {
    while (!uart_writeByteReady()); // Wait for the transmitter to be ready
    mmio_write(AUX_MU_IO_REG, (unsigned int)ch); // Write the byte
}

/**
 * Read a byte from the UART receiver blockingly.
 */
void uart_readFromMem(unsigned char *ch) {
    while (!uart_readByteReady()); // Wait for data to be available
    *ch = (unsigned char)mmio_read(AUX_MU_IO_REG); // Read the byte
}

// UART FIFO Buffer (Circular Queue)
unsigned char uart_out[UART_MAX_QUEUE];     // UART output buffer
unsigned int uart_out_write = 0;            // Write index
unsigned int uart_out_read = 0;             // Read index

/**
 * Check if the UART output queue is empty.
 */
unsigned int uart_fifoEmpty() {
    return uart_out_read == uart_out_write;
}

/**
 * Write a byte to the UART output queue
 */
void uart_fifoWrite(unsigned char ch) {
    unsigned int next_write = (uart_out_write + 1) % UART_MAX_QUEUE;
    
    while (next_write == uart_out_read) uart_fifoToMem();

    uart_out[uart_out_write] = ch; // Write to the queue
    uart_out_write = next_write; // Update write index
}

/**
 * Write the bytes from the UART output queue to the UART register
 */
void uart_fifoToMem() {
    // Checks if the queue is not empty and the register is ready to write
    while (!uart_fifoEmpty() && uart_writeByteReady()) {
        uart_writeToMem(uart_out[uart_out_read]); // Write to UART
        uart_out_read = (uart_out_read + 1) % UART_MAX_QUEUE; // Update read index
    }
}

/**
 * Write a string to the UART Output Buffer.
 */
void uart_writeText(char *text) {
    while (*text) {
        if (*text == '\n') {
            uart_fifoWrite('\r'); // Send carriage return before line feed
        } 
        uart_fifoWrite(*text++); // Send character
    }
}

/**
 * Update the UART by transferring data from the queue to the UART register.
 * Then read any incoming data from the UART and write it to the output queue.
 */
void uart_update() {
    uart_fifoToMem(); // Transfer data from the queue to the UART

    if (uart_readByteReady()) {
        unsigned char ch;
        uart_readFromMem(&ch); // Read from UART
        uart_fifoWrite(ch); // Write to the output queue
    }
}
