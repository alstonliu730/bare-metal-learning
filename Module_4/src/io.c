#include <io.h>
#include <common.h>

#define PULL_NONE 0
#define PULL_UP   1
#define PULL_DOWN 2

#define INT_BUF_SIZE 10
#define HEX_BUF_SIZE 18

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
uint32_t gpio_call(unsigned int pin, unsigned int value,
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

// ------------ GPIO functions ------------
// Enabling the GPIO pin to a high state
uint32_t gpio_set (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPSET0, 1, GPIO_MAX_PIN);
}

// Disabling the GPIO pin to a low state
uint32_t gpio_clear (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPCLR0, 1, GPIO_MAX_PIN);
}

// Set the pull-up/pull-down resistor for the GPIO pin
uint32_t gpio_pull (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPPUPPDN0, 2, GPIO_MAX_PIN);
}

// Defines the GPIO pin's operation mode. 
// See Section 5.3 in BCM2711 ARM Peripherals
uint32_t gpio_function (unsigned int pin, unsigned int value) {
    return gpio_call(pin, value, GPFSEL0, 3, GPIO_MAX_PIN);
}

// Set the GPIO pin to use Alternate function 5
void gpio_useAlt5 (unsigned int pin) {
    gpio_pull(pin, PULL_NONE);
    gpio_function(pin, GPIO_FUNCTION_ALT5);
}

// ------------ UART functions ------------
// Create UART output buffer
unsigned char uart_output_buffer[UART_MAX_QUEUE];
uint32_t uart_output_buffer_write;
uint32_t uart_output_buffer_read;

// UART register addresses
/**
 * Initialize the UART for communication.
 */
void uart_init() {
    gpio_useAlt5(14); // TXD1
    gpio_useAlt5(15); // RXD1

    // Clear buffer pointers
    uart_output_buffer_write = 0;
    uart_output_buffer_read = 0;

    mmio_write(AUX_ENABLES, 1); // Enable UART
    mmio_write(AUX_MU_CNTL_REG, 0); // Disable transmitter and receiver
    mmio_write(AUX_MU_IER_REG, 0); // Disable interrupts
    mmio_write(AUX_MU_LCR_REG, 3); // Set 8 data bits
    mmio_write(AUX_MU_MCR_REG, 0); // Disable modem control
    mmio_write(AUX_MU_IIR_REG, 0xC6); // Disable interrupts
    //mmio_write(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200)); // Set baud rate
    
    mmio_write(AUX_MU_CNTL_REG, 3); // Enable transmitter and receiver
}

/**
 * Checks if the UART FIFO buffer is empty.
 */
uint32_t uart_writeByteReady() {
    return mmio_read(AUX_MU_LSR_REG) & 0x20; // Check if the transmitter is ready
}

/**
 * Checks if the UART FIFO buffer is ready to read.
 */
uint32_t uart_readByteReady() {
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
uint32_t uart_bufferEmpty() {
    return uart_output_buffer_write == uart_output_buffer_read;
}

/**
 * Write out the UART output buffer until it's empty or the UART is not ready.
 */
void uart_loadOutputBuffer() {
    while (!uart_bufferEmpty()) {
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
 * Prints out the integer to UART in base 10 digits.
 */
void uart_writeInt(int num) {
    char buf[INT_BUF_SIZE];
    int i = 0;
    int isNeg = 0;    
    
    // Check if int is a negative
    if (num == 0) {
        isNeg = 1;
        num = -num;
    }

    if (num == 0) {
        buf[i++] = 0;
    } else {
        // Convert digits to chars in reverse order
        while (num > 0) {
            buf[i++]= (num % 10) + '0';
            num /= 10;
        }
    }

    if (isNeg) {
        buf[i++] = '-';
    }

    // Prints characters in order
    while(i > 0) {
        uart_writeByte(buf[--i]);
    }
}

/**
 * Prints out the integer to UART in hexadecimal format.
 */
void uart_writeHex(long num) {
    char buf[HEX_BUF_SIZE];
    int i = 0;
    int isNeg = 0;

     // Check if int is a negative
    if (num == 0) {
        isNeg = 1;
        num = -num;
    }

    if (num == 0) {
        buf[i++] = 0;
    } else {
        // Convert digits to chars in reverse order
        while (num > 0) {
            int c = num & 0xF;
            buf[i++]= HEX_STR(c);
            num = num >> 4;
        }
    }

    if (isNeg) {
        buf[i++] = '-';
    }

    // Prints characters in order
    while(i > 0) {
        uart_writeByte(buf[--i]);
    }
}

/**
 * Read a byte from the input register.
 */
unsigned char uart_readByte() {
    while (!uart_readByteReady());
    return (unsigned char) mmio_read(AUX_MU_IO_REG);
}

/**
 * Write a string to the UART output while handling line endings.
 */
void uart_writeText(char *text) {
    while (*text) {
        if (*text == '\n') {
            uart_writeByte('\r'); // Send carriage return before line feed
        } 
        uart_writeByte(*text++); // Send character
    }

    uart_loadOutputBuffer();
}

void uart_update() {
    if (uart_readByteReady()) {
        unsigned char ch = uart_readByte();
        // if the user press enter then we can write back
        if (ch == '\r' || ch == '\n') {
            uart_writeText("\n");
        }
        else uart_writeByte(ch);
    }
}

void debug_buffer_contents() {
    uart_writeByteBlocking('B');
    uart_writeByteBlocking('u');
    uart_writeByteBlocking('f');
    uart_writeByteBlocking(':');
    
    for (int i = 0; i < 4; i++) {
        unsigned char val = uart_output_buffer[i];
        
        // Print hex value
        char hex1 = (val >> 4) & 0xF;
        char hex2 = val & 0xF;
        
        uart_writeByteBlocking(hex1 < 10 ? '0' + hex1 : 'A' + hex1 - 10);
        uart_writeByteBlocking(hex2 < 10 ? '0' + hex2 : 'A' + hex2 - 10);
        uart_writeByteBlocking(' ');
    }
    uart_writeByteBlocking('\r');
    uart_writeByteBlocking('\n');
}

// LED Functions
void led_init() {
    gpio_function(42, GPIO_FUNCTION_OUT); // Built-in LED (if available)
}

void led_on() {
    gpio_set(42, 1);
}

void led_off() {
    gpio_clear(42, 1);
}