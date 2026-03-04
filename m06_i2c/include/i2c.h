#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>
#include <common.h>

// I2C Error Code
typedef enum {
    I2C_SUCCESS,
    I2C_ACK_ERR,
    I2C_INV_INPUT,
    I2C_DATA_LOSS,
    I2C_CLK_TIMEOUT,
    I2C_NOT_ENABLED
} i2c_status;

// I2C Addressing Mode
typedef enum {
    I2C_7BIT = 0,
    I2C_10BIT
} i2c_mode;

// I2C Register Clock Speed
typedef enum {
    DEF_CLK_MODE = 100000,
    FAST_CLK_MODE = 400000
} i2c_clk_mode;

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

// Offset Values
#define I2C_C_OFFSET                0x00
#define I2C_S_OFFSET                0x04
#define I2C_DLEN_OFFSET             0x08
#define I2C_A_OFFSET                0x0C
#define I2C_FIFO_OFFSET             0x10
#define I2C_DIV_OFFSET              0x14
#define I2C_DEL_OFFSET              0x18
#define I2C_CLKT_OFFSET             0x1C

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
#define I2C_TOUT                    BIT_MASK(15, 0)

// Constants
#define DEFAULT_I2C_TIMEOUT         0x40    
#define WRITE_REG_DLEN              2
#define MAX_I2C_DEV_ADDR            LSHIFT(1, 7) - 1

#define DEFAULT_BEGIN               0x08
#define DEFAULT_END                 0x77

// Function Declarations
/**
 * Initializes the master device for the given i2c bus address
 * Initiates the gpio pins for SDA & SCL
 * 
 * @param bus           i2c bus base address
 * @param sda_pin       GPIO number for SDA line
 * @param scl_pin       GPIO number for SCL line
 */
void i2c_init(volatile i2c_reg_t* bus, uint8_t sda_pin, uint8_t scl_pin);

/**
 * Sets the device address for the given bus.
 * 
 * @param bus           i2c bus base address
 * @param dev           device address
 * @param mode          i2c 7-bit or 10-bit addressing mode
 */
void i2c_setDevAddr(volatile i2c_reg_t* bus, uint16_t dev, i2c_mode mode);

/**
 * Resets the I2C bus by clearing the FIFO & clearing the DONE status.
 * 
 * @param bus           i2c bus base address
 */
void i2c_reset(volatile i2c_reg_t* bus);

/**
 * Set the clock speed on the I2C Master by setting the Divisor Register.
 * 
 * @param bus           i2c bus base address
 * @param clkSpeed      i2c clock speed
 */
void i2c_setClock(volatile i2c_reg_t* bus, i2c_clk_mode clkSpeed);

/**
 * Pings the device in the current bus by just sending the device address.
 * 
 * @param bus           i2c bus base address
 * @param dev           device address
 * @param mode          i2c 7-bit or 10-bit address modes
 * 
 * @return              An i2c status code
 */
i2c_status i2c_ping(volatile i2c_reg_t* bus, uint16_t dev, i2c_mode mode);

/**
 * Detects i2c devices in the given bus by listing the device address:
 * - "--" = i2c device probed but not answered
 * - "UU" = i2c device probed but its in use
 * - "XX" = i2c device responded back (where X is a hexadecimal number)
 * 
 * @param bus           i2c bus base address
 * @param first         beginning range of the scan
 * @param last          end range of the scan (exclusive)
 */
void i2c_detect(volatile i2c_reg_t* bus, uint8_t first, uint8_t last);

/**
 * Polling method for receiving data from the slave drive. Waits until the end of transmission.
 * 
 * @param bus           i2c bus base address
 * @param dev           device address to the slave drive
 * @param writeBuf      register address in the i2c device
 * @param nWrite        Number of bytes to write
 * @param readBuf           address to the buffer
 * @param size          number of bytes per transfer
 * 
 * @return              An i2c status code
 */
i2c_status i2c_writeReadRepeat(volatile i2c_reg_t* bus, uint8_t dev, 
    const void* writeBuf, const uint32_t nWrite, void* readBuf, const uint32_t size);

/**
 * Polling method for sending data to the slave drive. Waits until the end of transmission.
 * Caller is responsible for allocating memory for the buffer and providing the correct amount of data that 
 * will be written using bytes as the measurement.
 * The buffer and size will need to include the register address depending on the i2c device communication protocol.
 * For example, the MLX90640 requires a 16-bit register address to access the internal register map.
 * 
 * @param base  i2c bus base address
 * @param dev   device address to the slave drive
 * @param buf   address to the buffer
 * @param size  number of bytes in the buffer
 * 
 * @return An i2c status code
 */
i2c_status i2c_blocking_send(volatile i2c_reg_t* bus, uint8_t dev, const uint32_t size, const void* buf);

#endif /* __I2C_H__*/