#ifndef __MLX_90640_H__
#define __MLX_90640_H__

#include <common.h>
#include <i2c.h>

//*********************************
// Note: Values in MLX90640's EEPROM calibration data
// uses the two's complement to store its value.
//*********************************

typedef enum {
    MLX_SUCCESS = 0,
    MLX_NACK,
    MLX_CORRUPT,
    MLX_UNKNOWN_ERR
} mlx_error;

#define MLX_REG_DLEN                2

// Default 0x33 but found 0x3B as the sensor
#define MLX_DEV_ADDR                0x3B 

// MLX User-Accessible Internal Registers
#define MLX_STATUS                  0x8000
#define MLX_CTRL1                   0x800D
#define MLX_CTRL2                   0x800E
#define MLX_I2C_CONFIG              0x800F
#define MLX_I2C_ADDR                0x8010

/**
 * Initialize the I2C Bus associated for the MLX90640.
 */
void mlx_i2cInit();

/**
 * Reads a number of words from a selected MLX90640 device memory starting from the given address.
 * Stores the data in the memory location given by the user.
 * 
 * @param dev           device address of the MLX90640 (default: 0x33)
 * @param start         start address from the MLX90640 memory to be read
 *                      RAM [0x0400 - 0x073F];  EEPROM [0x2400 - 0x273F]
 * @param nRead         Number of 16-bits words to be read
 * @param data          pointer to the memory location where the data is stored
 * 
 * @return              Returns a MLX Error Code
 */
mlx_error mlx_i2cRead(uint8_t dev, uint16_t start, uint16_t nRead, uint16_t *data);

/**
 * Write a number of words to a selected MLX90640 device. 
 * The function reads back the data after the write operation is done.
 * 
 * @param dev           device address of the MLX90640 (default: 0x33)
 * @param writeAddr     memory address in the MLX90640 to write to
 * @param nWrite        Number of 16-bits words to be write
 * @param data          pointer to the data to be written in the MLX90640 address
 * 
 * @return              Returns a MLX Error Code
 */
mlx_error mlx_i2cWrite(uint8_t dev, uint16_t writeAddr, uint16_t nWrite, uint16_t *data);

/**
 * Get the 16-bit value of the control register 1 in the internal regs.
 * 
 * @return Control Register 1 value
 */
uint16_t mlx_getCtrlReg1();

/**
 * Get the 16-bit value of the status register in the internal regs.
 * 
 * @return Status Register
 */
uint16_t mlx_getStatusReg();

/**
 * Get the 8-bit value of the i2c address in the regs
 * 
 * @return I2C Address
 */
uint16_t mlx_getI2CAddr();

#endif /* __MLX_90640_H__ */