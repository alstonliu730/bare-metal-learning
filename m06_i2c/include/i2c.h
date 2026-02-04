#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>
#include <common.h>

/**
 * TODO: 
 * - Different Operating Modes: Blocking, ISR, or Later DMA
 * - Set Speed function
 * - brainstorm ideas for functions:
 *  - set slave address()
 *  - start write transmission
 *  - start read transmission
 *  - i2c_read
 */

// I2C Error Code
typedef enum {
    I2C_SUCCESS,
    I2C_INV_INPUT,
    I2C_ACK_ERR,
    I2C_DATA_LOSS,
    I2C_CLK_TIMEOUT,
    I2C_NOT_ENABLED
} i2c_status;

// I2C Register Structures
typedef struct {
    uint32_t control;
    uint32_t status;
    uint32_t dlen;
    uint32_t addr;
    uint32_t fifo;
    uint32_t div;
    uint32_t del;
    uint32_t clkt;
} i2c_reg_t;

// Broadcom Serial Controller Addresses
#define BSC0_ADDR                   (PERIPHERAL_BASE + 0x205000)
#define BSC1_ADDR                   (PERIPHERAL_BASE + 0x804000)
#define BSC3_ADDR                   (PERIPHERAL_BASE + 0x205600)
#define BSC4_ADDR                   (PERIPHERAL_BASE + 0x205800)
#define BSC5_ADDR                   (PERIPHERAL_BASE + 0x205a80)
#define BSC6_ADDR                   (PERIPHERAL_BASE + 0x205c00)

#define I2C_REG(addr)               ((volatile i2c_reg_t *) (addr))

// Control Register Mask
#define C_ENABLE                    BIT(15)
#define C_DISABLE                   CLR_BIT(15)
#define C_INTR_EN                   BIT(10)
#define C_INTR_DE                   CLR_BIT(10)
#define C_INTT_EN                   BIT(9)
#define C_INTT_DE                   CLR_BIT(9)
#define C_INTD_EN                   BIT(8)
#define C_INTD_DE                   CLR_BIT(8)
#define C_START                     BIT(7)
#define C_CLEAR                     BIT_MASK(5,4)
#define C_READ                      BIT(0)
#define C_WRITE                     CLR_BIT(0)

// Status Register Mask
#define S_CLKTOUT                   BIT(9)
#define S_ERR                       BIT(8)
#define S_RXF                       BIT(7)
#define S_TXE                       BIT(6)
#define S_RXD                       BIT(5)
#define S_TXD                       BIT(4)
#define S_RXR                       BIT(3)
#define S_TXW                       BIT(2)
#define S_DONE                      BIT(1)
#define S_TA                        BIT(0)

// Data Length Register Mask
#define I2C_DLEN                    BIT_MASK(15, 0)

// Slave Address Register Mask
#define I2C_SLV_ADDR                BIT_MASK(6, 0)

// FIFO Register Mask
#define I2C_FIFO                    BIT_MASK(7, 0)

// Clock divider Register Mask
#define I2C_CDIV                    BIT_MASK(15, 0)

// Data Delay Register Mask
#define I2C_FEDL                    BIT_MASK(31, 16)
#define I2C_REDL                    BIT_MASK(15, 0)

// Clock stretch timeout Register Mask
#define I2C_TOUT                 BIT_MASK(15, 0)

// Constants
#define DEFAULT_CDIV                0x05DC
#define READ_REG_DLEN               1
#define WRITE_REG_DLEN              2

// Function Declarations
/**
 * Initializes the master device for the given i2c bus address
 * Initiates the gpio pins for SDA & SCL
 * 
 * @param base          i2c bus base address
 * @param sda_pin       GPIO number for SDA line
 * @param scl_pin       GPIO number for SCL line
 */
void i2c_init(volatile i2c_reg_t* base, uint8_t sda_pin, uint8_t scl_pin);

/**
 * Polling method for receiving data from the slave drive. Waits until the end of transmission.
 * 
 * @param bus           i2c bus base address
 * @param dev           device address to the slave drive
 * @param reg           register address in the i2c device
 * @param size          number of bytes per transfer
 * @param buf           address to the buffer
 * 
 * @return              An i2c status code
 */
i2c_status i2c_blocking_recv(volatile i2c_reg_t* bus, uint8_t dev, uint8_t reg, uint32_t size, uint8_t* buf);

/**
 * Polling method for sending data to the slave drive. Waits until the end of transmission.
 * 
 * @param bus           i2c bus base address
 * @param dev           device address to the slave drive
 * @param reg           register address in the i2c device
 * @param size          number of bytes in the buffer
 * @param buf           address to the buffer
 * 
 * @return              An i2c status code
 */
i2c_status i2c_blocking_send(volatile i2c_reg_t* bus, uint8_t dev, uint8_t reg, uint32_t size, uint8_t* buf);

#endif /* __I2C_H__*/