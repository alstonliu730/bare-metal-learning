#ifndef IO_H
#define IO_H

#define PERIPHERAL_BASE     0xFE000000
#define GPFSEL0             (PERIPHERAL_BASE + 0x200000)
#define GPSET0              (PERIPHERAL_BASE + 0x20001C)
#define GPCLR0              (PERIPHERAL_BASE + 0x200028)
#define GPPUPPDN0           (PERIPHERAL_BASE + 0x2000E4)

#define GPIO_MAX_PIN 53
#define GPIO_FUNCTION_OUT 1
#define GPIO_FUNCTION_ALT5 2
#define GPIO_FUNCTION_ALT3 7

#define AUX_BASE            (PERIPHERAL_BASE + 0x215000)
#define AUX_ENABLES         (AUX_BASE + 0x04)
#define AUX_MU_IO_REG       (AUX_BASE + 0x40)
#define AUX_MU_IER_REG      (AUX_BASE + 0x44)
#define AUX_MU_IIR_REG      (AUX_BASE + 0x48)
#define AUX_MU_LCR_REG      (AUX_BASE + 0x4C)
#define AUX_MU_MCR_REG      (AUX_BASE + 0x50)
#define AUX_MU_LSR_REG      (AUX_BASE + 0x54)
#define AUX_MU_CNTL_REG     (AUX_BASE + 0x60)
#define AUX_MU_BAUD_REG     (AUX_BASE + 0x68)

#define AUX_UART_CLOCK      500000000   // 500MHz
#define UART_MAX_QUEUE      (16 * 1024)

#define AUX_MU_BAUD(baud) \
    ((AUX_UART_CLOCK / (baud * 8)) - 1)                   

void uart_init();
void uart_writeText(char *text);

#endif /* IO_H*/