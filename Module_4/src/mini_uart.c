#include <mini_uart.h>
#include <gpio.h>
#include <common.h>

// Create UART output buffer
static unsigned char mUart_output_buffer[AUX_MU_MAX_QUEUE];
static uint32_t mUart_output_buffer_write;
static uint32_t mUart_output_buffer_read;

/**
 * Return if the buffer is empty.
 */
uint32_t mUart_bufferEmpty() {
    return mUart_output_buffer_write == mUart_output_buffer_read;
}

/**
 * Initialize the UART for communication.
 */
void mUart_init() {
    gpio_useAlt5(14); // TXD1
    gpio_useAlt5(15); // RXD1

    // Clear buffer pointers
    mUart_output_buffer_write = 0;
    mUart_output_buffer_read = 0;

    mmio_write(AUX_ENABLES, 1); // Enable UART
    mmio_write(AUX_MU_CNTL_REG, 0); // Disable transmitter and receiver
    mmio_write(AUX_MU_IER_REG, 0); // Disable interrupts
    mmio_write(AUX_MU_LCR_REG, 0x03); // Set 8 data bits
    mmio_write(AUX_MU_MCR_REG, 0); // Disable modem control
    mmio_write(AUX_MU_IIR_REG, 0xC6); // Clear RX & TX FIFOs
    // mmio_write(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200)); // Set baud rate
    
    mmio_write(AUX_MU_CNTL_REG, 3); // Enable transmitter and receiver
}

/**
 * Checks if the UART FIFO buffer is empty.
 */
uint32_t mUart_writeByteReady() {
    return mmio_read(AUX_MU_LSR_REG) & 0x20; // Check if Transmit FIFO has room for 1 byte
}

/**
 * Checks if the UART FIFO buffer is ready to read.
 */
uint32_t mUart_readByteReady() {
    return mmio_read(AUX_MU_LSR_REG) & 0x01; // Check if Receive FIFO is ready
}

/**
 * Read a byte from the input register.
 */
unsigned char mUart_readByte() {
    while (!mUart_readByteReady());
    return (unsigned char) mmio_read(AUX_MU_IO_REG);
}

/**
 * Write a byte to the UART output buffer.
 * This function will block until there is space in the buffer.
 */
void mUart_writeByte(unsigned char ch) {
    unsigned int next_write = (mUart_output_buffer_write + 1) % AUX_MU_MAX_QUEUE;
    
    while (next_write == mUart_output_buffer_read) {
        // Wait until there is space in the buffer
        mUart_loadOutputBuffer();
    }

    mUart_output_buffer[mUart_output_buffer_write] = ch;
    mUart_output_buffer_write = next_write;
}

/**
 * Write a byte to the UART transmitter blockingly.
 */
void mUart_writeByteBlocking(unsigned char ch) {
    while (!mUart_writeByteReady());
    mmio_write(AUX_MU_IO_REG, (unsigned int)ch);
}

/**
 * Write a string to mini UART.
 */
void mUart_writeText(char* text) {
    while (*text) {
        if (*text == '\n') {
            mUart_writeByte('\r'); // Send carriage return before line feed
        } 
        mUart_writeByte(*text++); // Send character
    }
    
    mUart_loadOutputBuffer();
}

void mUart_writeInt(long num) {
    char buf[MU_INT_BUF_SIZE];
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
        mUart_writeByte(buf[--i]);
    }
}

void mUart_writeHex(uint64_t num) {
    char buf[MU_HEX_BUF_SIZE];
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
            buf[i++]= MU_HEX_STR(c);
            num = num >> 4;
        }
    }

    if (isNeg) {
        buf[i++] = '-';
    }

    // Prints characters in order
    while(i > 0) {
        mUart_writeByte(buf[--i]);
    }
}

/**
 * Write out the UART 1 output buffer until it's empty or the UART is not ready.
 */
void mUart_loadOutputBuffer() {
    while (!mUart_bufferEmpty()) {
        mUart_writeByteBlocking(mUart_output_buffer[mUart_output_buffer_read]);
        mUart_output_buffer_read = (mUart_output_buffer_read + 1) % AUX_MU_MAX_QUEUE;
    }
}

/**
 * Polling Bytes for mini UART
 */
void mUart_update() {
    if (mUart_readByteReady()) {
        unsigned char ch = mUart_readByte();
        // if the user press enter then we can write back
        if (ch == '\r' || ch == '\n') {
            mUart_writeText("\n");
        }
        else mUart_writeByte(ch);
    }
}