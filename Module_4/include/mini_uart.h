#ifndef MINI_UART_H
#define MINI_UART_H

#include <common.h>
#define MU_HEX_STR(h) ((h < 10) ? '0' + h : 'A' + h - 10)

#define MU_INT_BUF_SIZE            22
#define MU_HEX_BUF_SIZE            18
// ------------------------- Mini UART -------------------------
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

#define AUX_MU_BAUD(baud) \
    ((AUX_UART_CLOCK / (8 * baud)) - 1)
#define AUX_MU_MAX_QUEUE        (16 * 1024)

// ------------------------- FUNCTIONS -------------------------
void mUart_init();
void mUart_update();

void mUart_writeInt(long num);
void mUart_writeHex(uint64_t num);
void mUart_writeByte(unsigned char ch);
void mUart_writeByteBlocking(unsigned char ch);
void mUart_writeText(char *text);
void mUart_loadOutputBuffer();

// UART Status functions
unsigned char mUart_readByte();
uint32_t mUart_writeByteReady();
uint32_t mUart_readByteReady();
uint32_t mUart_bufferEmpty();

#endif /* MINI_UART_H */