#include <uart.h>
#include <gpio.h>
#include <mb.h>
#include <irq.h>
#include <gic.h>

#define DEFAULT_UART_CLK        7372800
#define VC_UART_IRQ             0x39

// Create UART output buffer
static volatile unsigned char uart_output_buffer[UART_MAX_QUEUE];
static volatile uint32_t uart_output_buffer_write;
static volatile uint32_t uart_output_buffer_read;
typedef struct {
    uint32_t total_written;
    uint32_t total_transmitted;
    uint32_t startTX_calls;
    uint32_t interrupt_calls;
    uint32_t buffer_write_pos;
    uint32_t buffer_read_pos;
} debug_info_t;

// Create debugging information
volatile debug_info_t debug_info __attribute__((section(".data"))) = {0};

void dump_debug_info() {
    uart_writeText("Written: 0x");
    uart_writeHex(debug_info.total_written);
    uart_writeText("\nTransmitted: 0x");
    uart_writeHex(debug_info.total_transmitted);
    uart_writeText("\nStartTX: 0x");
    uart_writeHex(debug_info.startTX_calls);
    uart_writeText("\nInterrupt Calls: 0x");
    uart_writeHex(debug_info.interrupt_calls);
    uart_writeText("\nBuffer Write Pos: 0x");
    uart_writeHex(uart_output_buffer_write);
    uart_writeText("\nBuffer Read Pos: 0x");
    uart_writeHex(uart_output_buffer_read);
}

// Mailbox Request to get the UART Clock
static uint32_t __attribute__((unused)) get_uart_clock() {
    // Setting mbox array
    mbox[0] = 8 * 4;        // Size in bytes
    mbox[1] = MBOX_REQUEST; // Request Tag

    mbox[2] = MBOX_TAG_GETCLK;  // Tag for get clock rate
    mbox[3] = 8;                // tag size
    mbox[4] = MBOX_REQUEST;
    mbox[5] = MBOX_CLK_UART;    // clock id
    mbox[6] = 0;
    mbox[7] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP) && mbox[1] == MBOX_SUCCESS && mbox[5] == MBOX_CLK_UART) {
        return mbox[6]; // returns the clock rate
    } else {
        return 0;
    }
}

// Mailbox Request to set the UART clock to a given frequency
static uint32_t set_uart_clk(uint32_t frq) {
    // Setting mbox array
    mbox[0] = 9*4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_SETCLK;
    mbox[3] = 12;
    mbox[4] = MBOX_REQUEST;
    mbox[5] = MBOX_CLK_UART;
    mbox[6] = frq;
    mbox[7] = 1;
    mbox[8] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP) && mbox[1] == MBOX_SUCCESS && mbox[5] == MBOX_CLK_UART) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Return if the buffer is empty.
 */
uint32_t uart_bufferEmpty() {
    return uart_output_buffer_write == uart_output_buffer_read;
}

/**
 * Starts the TX Transmission Interrupt
 */
void uart_startTX() {
    if (uart_bufferEmpty()) {
        // Do nothing if the buffer is empty
        return;
    }

    // Prime the TX FIFO with initial data if it's empty
    if (UART0_TXFE) {  // If TX FIFO is empty
        while (!uart_bufferEmpty() && !UART0_TXFF) {
            mmio_write(UART0_DR, uart_output_buffer[uart_output_buffer_read]);
            uart_output_buffer_read = (uart_output_buffer_read + 1) % UART_MAX_QUEUE;
            debug_info.total_transmitted++;
        }
    }
    
    // Enable TX interrupt only if there's still data in the buffer
    if (!uart_bufferEmpty()) {
        uint32_t mask_val = mmio_read(UART0_IMSC) | UART_TX_BIT;
        mmio_write(UART0_IMSC, mask_val);
    }
}

/**
 * Write a byte to the UART output buffer.
 * If the FIFO is full, then write to the output buffer until the TX interrupt is asserted.
 */
void uart_writeByte(unsigned char ch) {
    unsigned int next_write = (uart_output_buffer_write + 1) % UART_MAX_QUEUE;
    
    if (next_write == uart_output_buffer_read) {
        // if the buffer is full attempt to transmit immediately
        uart_startTX();
    } 

    // DEBUG
    debug_info.total_written++;

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
 * Write a string to the UART output while handling line endings.
 */
void uart_writeText(char *text) {
    while (*text) {
        if (*text == '\n') {
            uart_writeByte('\r'); // Send carriage return before line feed
        } 
        uart_writeByte(*text++); // Send character
    }

    uart_startTX();
}

/* ------------------------------------- PL011 UART ------------------------------------- */

// Setting FIFO fill level
void set_fifo_level(fifo_level_t rx_sel, fifo_level_t tx_sel) {
    uint32_t val = ((rx_sel & 0x7) << 3) | (tx_sel & 0x7);
    mmio_write(UART0_IFLS, val);
}

/**
 * Initialization of PL011 UART0:
 *  - Disable it first, clear FIFO
 *  - Set FIFO levels
 *  - Setting baud rate divisor
 *  - Set Appropriate GPIO alt function
 *  - Enable FIFO and set word length
 *  - Enable TX, RX and UART
 */
void uart_init() {
    // Disable UART first
    mmio_write(UART0_CR, 0);
    delay(1000);
    
    // Flush FIFO by setting FEN to 0
    mmio_write(UART0_LCRH, (3 << 5));

    // Set FIFO Levels
    set_fifo_level(/*RX SELECT = */sel1, /*TX SELECT = */sel2);
    
    uart_output_buffer_write = 0;
    uart_output_buffer_read = 0;

    // Setting UART CLK Rate
    set_uart_clk(DEFAULT_UART_CLK);

    // set the baud rate on the current clock
    mmio_write(UART0_IBRD, 0x4);    // write to the integer baud rate divisor (16-bit)
    mmio_write(UART0_FBRD, 0);      // write to the fractional baud rate divisor (6-bit)
    
    // Set GPIO function as alternate function 0
    gpio_useAlt0(14);
    gpio_useAlt0(15);

    // Enable the FIFO & set the word length
    uint32_t lcr_val = (1 << 4) | (3 << 5);
    mmio_write(UART0_LCRH, lcr_val);

    // Enabling only the receive interrupt
    mmio_write(UART0_IMSC, (1 << 4));

    // Enable TX and RX and UART again
    uint32_t cr_val = (1 << 9) | (1 << 8) | 1;
    mmio_write(UART0_CR, cr_val);
}

/**
 * When the IRQ line is asserted for the UART this handles it for all UART.
 */
void uart_handler() {
    // Check which IRQ was set (reading bits 16 - 20)
    uint32_t uart_id = mmio_read(PACTL_CS) & (0x1F << 16);

    // UART 0
    if (uart_id & UART0_MASK) {
        // Check which line was asserted
        uint32_t mis = mmio_read(UART0_MIS);

        if (mis & UART_RX_BIT) {
            uart_rx_handler();
        }

        if (mis & UART_TX_BIT) {
            uart_tx_handler();
        }

        if (mis & UART_RT_BIT) {
            uart_rt_handler();
        }
    }
}

/**
 * Handles the UART 0 Transmitter Interrupt:
 *  - Checks circular buffer for data
 *  - Refills Transmit FIFO
 */ 
void uart_tx_handler() {
    // Check if the buffer is not empty and TX FIFO is not full
    do {
        // write the next char in the buffer
        if (!UART0_TXFF) {
            mmio_write(UART0_DR, uart_output_buffer[uart_output_buffer_read]);
            uart_output_buffer_read = (uart_output_buffer_read + 1) % UART_MAX_QUEUE;
        }
    } while (!uart_bufferEmpty());

    // Clear the Transmit Interrupt
    mmio_write(UART0_ICR, UART_TX_BIT);

    // Suspend the Transmit Interrupt
    uint32_t mask_val = mmio_read(UART0_IMSC) & ~UART_TX_BIT;
    mmio_write(UART0_IMSC, mask_val);
}

/**
 * Handles the UART 0 Receiver Interrupt.
 */
void uart_rx_handler() {
    // Check if the Receive FIFO is not empty and UART is not busy transmitting
    while(!UART0_RXFE) {
        char c = (char) (mmio_read(UART0_DR) & 0xFF); // reads the first 8 bits of the Data Reg

        // For immediate echo, try direct write if FIFO has space
        if (!UART0_TXFF) {
            mmio_write(UART0_DR, c);  // Direct echo
        } else {
            uart_writeByte(c);  // Use buffer only if FIFO full
            uart_startTX();
        }
    }

    // clear the interrupt
    mmio_write(UART0_ICR, UART_RX_BIT);
}

/**
 * Handles the UART 0 Receive Timeout Interrupt
 */
void uart_rt_handler() {
    // Read leftover data in FIFO
    while(!UART0_RXFE) {
        char c = (char) (mmio_read(UART0_DR) & 0xFF); // reads the first 8 bits of the Data Reg

        // For immediate echo, try direct write if FIFO has space
        if (!UART0_TXFF) {
            mmio_write(UART0_DR, c);  // Direct echo
        } else {
            uart_writeByte(c);  // Use buffer only if FIFO full
            uart_startTX();
        }
    }

    // clear the interrupt
    mmio_write(UART0_ICR, UART_RT_BIT);
}
