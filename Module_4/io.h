#ifndef IO_H
#define IO_H

#define PERIPHERAL_BASE         0xFE000000
#define GPIO_BASE               (PERIPHERAL_BASE + 0x200000)
#define GPFSEL0                 (GPIO_BASE + 0x00)
#define GPFSEL1                 (GPIO_BASE + 0x04)
#define GPFSEL2                 (GPIO_BASE + 0x08)
#define GPFSEL3                 (GPIO_BASE + 0x0C)
#define GPFSEL4                 (GPIO_BASE + 0x10)
#define GPSET0                  (GPIO_BASE + 0x1C)
#define GPCLR0                  (GPIO_BASE + 0x28)
#define GPPUPPDN0               (GPIO_BASE + 0xE4)

#define GPIO_MAX_PIN 53
#define GPIO_FUNCTION_OUT 1
#define GPIO_FUNCTION_ALT5 2
#define GPIO_FUNCTION_ALT3 7

#define AUX_BASE                (PERIPHERAL_BASE + 0x215000)
#define AUX_ENABLES             (AUX_BASE + 0x04)
#define AUX_MU_IO_REG           (AUX_BASE + 0x40)
#define AUX_MU_IER_REG          (AUX_BASE + 0x44)
#define AUX_MU_IIR_REG          (AUX_BASE + 0x48)
#define AUX_MU_LCR_REG          (AUX_BASE + 0x4C)
#define AUX_MU_MCR_REG          (AUX_BASE + 0x50)
#define AUX_MU_LSR_REG          (AUX_BASE + 0x54)
#define AUX_MU_CNTL_REG         (AUX_BASE + 0x60)
#define AUX_MU_STAT_REG         (AUX_BASE + 0x64)
#define AUX_MU_BAUD_REG         (AUX_BASE + 0x68)

#define AUX_UART_CLOCK          250000000   // 250MHz
#define UART_MAX_QUEUE          (16 * 1024)

#define AUX_MU_BAUD(baud) \
    ((AUX_UART_CLOCK / (8 * baud)) - 1)  

void uart_init();
void uart_update();

void mmio_write(long reg, unsigned int value);
unsigned int mmio_read(long reg);
// Writing function
unsigned int uart_writeByteReady();
void uart_writeByte(unsigned char ch);
void uart_writeByteBlocking(unsigned char ch);
void uart_writeText(char *text);
void uart_loadOutputBuffer();
void uart_writeInt(int num);

// Reading functions
unsigned char uart_readByte();
unsigned int uart_readByteReady();

// Debugging functions
void led_init();
void led_on();
void led_off();
void debug_buffer_contents();

extern unsigned char uart_output_buffer[UART_MAX_QUEUE];

// Suspend tasks for n amount of cycles
static inline void delay(volatile unsigned int count) {
    while (count--) asm("nop");
}

#endif /* IO_H*/