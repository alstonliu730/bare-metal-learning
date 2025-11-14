#ifndef UART_H
#define UART_H

#include <common.h>

#define HEX_STR(h) ((h < 10) ? '0' + h : 'A' + h - 10)

#define INT_BUF_SIZE            10
#define HEX_BUF_SIZE            18
#define DESIRED_BAUD            115200
#define UART_MAX_QUEUE          (16 * 1024)

// ------------------------- PL011 UART -------------------------
#define UART0_BASE              0xFE201000
#define UART2_BASE              0xFE201400
#define UART3_BASE              0xFE201600
#define UART4_BASE              0xFE201800
#define UART5_BASE              0xFE201A00

#define UART0_MASK              (1 << 20)
#define UART2_MASK              (1 << 19)
#define UART3_MASK              (1 << 18)
#define UART4_MASK              (1 << 17)
#define UART5_MASK              (1 << 16)

#define UART_RX_BIT             (1 << 4)
#define UART_TX_BIT             (1 << 5)
#define UART_RT_BIT             (1 << 6)

// ------------------------- UART 0 Addresses-------------------------
#define UART0_DR                (UART0_BASE + 0x00)
#define UART0_RSRECR            (UART0_BASE + 0x04)
#define UART0_FR                (UART0_BASE + 0x18)
#define UART0_IBRD              (UART0_BASE + 0x24)
#define UART0_FBRD              (UART0_BASE + 0x28)
#define UART0_LCRH              (UART0_BASE + 0x2C)
#define UART0_CR                (UART0_BASE + 0x30)
#define UART0_IFLS              (UART0_BASE + 0x34)
#define UART0_IMSC              (UART0_BASE + 0x38)
#define UART0_RIS               (UART0_BASE + 0x3C)
#define UART0_MIS               (UART0_BASE + 0x40)
#define UART0_ICR               (UART0_BASE + 0x44)
#define UART0_DMACR             (UART0_BASE + 0x48)
#define UART0_ITCR              (UART0_BASE + 0x80)
#define UART0_ITIP              (UART0_BASE + 0x84)    
#define UART0_ITOP              (UART0_BASE + 0x88)
#define UART0_TDR               (UART0_BASE + 0x8C)

#define UART0_BUSY              (mmio_read(UART0_FR) & (1 << 3))
#define UART0_RXFE              (mmio_read(UART0_FR) & (1 << 4))
#define UART0_TXFF              (mmio_read(UART0_FR) & (1 << 5))
#define UART0_RXFF              (mmio_read(UART0_FR) & (1 << 6))
#define UART0_TXFE              (mmio_read(UART0_FR) & (1 << 7))

typedef enum {
    sel0 = 0, // 1/8
    sel1 = 1, // 1/4
    sel2 = 2, // 1/2
    sel3 = 3, // 3/4
    sel4 = 4, // 7/8
} fifo_level_t;

// ------------------------- UART Functions -------------------------
// UART Write Functions
void uart_writeByte(unsigned char ch);
void uart_writeInt(int num);
void uart_writeHex(long num);

// UART 0
void uart_writeText(char *text);
void uart_loadOutputBuffer();

// UART 0 Interrupt
void uart_init();
void uart_handler();
void uart_tx_handler();
void uart_rx_handler();
void uart_rt_handler();
void set_fifo_level(fifo_level_t rx_sel, fifo_level_t tx_sel);

void dump_debug_info();
#endif