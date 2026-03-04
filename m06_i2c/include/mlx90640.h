#ifndef __MLX_90640_H__
#define __MLX_90640_H__

#include <common.h>
#include <i2c.h>

//*********************************
// Note: Values in MLX90640's EEPROM calibration data
// uses the two's complement to store its value.
//*********************************

// MLX90640 Status Error Codes
typedef enum {
    MLX_SUCCESS = 0,
    MLX_NACK,
    MLX_CORRUPT,
    MLX_UNKNOWN_ERR
} mlx_error;

// MLX90640 Calibration Params
typedef struct mlx90640{
    int16_t kVdd;
    int16_t vdd25;
    float KvPTAT;
    float KtPTAT;
    uint16_t vPTAT25;
    float alphaPTAT;
    int16_t gainEE;
    float tgc;
    float cpKv;
    float cpKta;
    uint8_t resolutionEE;
    uint8_t calibrationModeEE;
    float KsTa;
    float KsTo[5];
    int16_t ct[5];
    uint16_t alpha[768];    
    uint8_t alphaScale;
    int16_t offset[768];    
    int8_t kta[768];
    uint8_t ktaScale;    
    int8_t kv[768];
    uint8_t kvScale;
    float cpAlpha[2];
    int16_t cpOffset[2];
    float ilChessC[3]; 
    uint16_t brokenPixels[5];
    uint16_t outlierPixels[5];  
} mlx_param;

// Register Length in Bytes
#define MLX_REG_DLEN                2

// EEPROM Dump Length in Words
#define MLX_EEPROM_LEN              832
#define MLX_PIXEL_LEN               832
#define MLX_FRAME_DATA_LEN          834
#define MLX_TOTAL_PIX               768

// Alpha Scale
#define SCALEALPHA                  0.000001f

// Default 0x33 but found 0x3B as the sensor
#define MLX_DEV_ADDR                0x3B 

// MLX User-Accessible Internal Registers
#define MLX_STATUS                  0x8000
#define MLX_CTRL1                   0x800D
#define MLX_CTRL2                   0x800E
#define MLX_I2C_CONFIG              0x800F
#define MLX_I2C_ADDR                0x8010

// MLX Frame Data Range
#define MLX_FRAME_ADDR_START        0x0400
#define MLX_FRAME_ADDR_END          0x07FF

// MLX EEPROM Start address
#define MLX_EEPROM_ADDR_START       0x2400

// MLX Refresh Rate
#define FPS0_5HZ                    0x00
#define FPS1HZ                      0x01
#define FPS2HZ                      0x02
#define FPS4HZ                      0x03
#define FPS8HZ                      0x04
#define FPS16HZ                     0x05
#define FPS32HZ                     0x06
#define FPS64HZ                     0x07

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
 * Write a number of words to a selected MLX90640 device. The function reads back the data after the write operation is done.
 * This function is responsible for adding the register address to the beginning of the data array.
 * 
 * @param dev           device address of the MLX90640 (default: 0x33)
 * @param writeAddr     memory address in the MLX90640 to write to
 * @param nWrite        Number of 16-bits words to be write
 * @param data          pointer to the data to be written in the MLX90640 address
 * 
 * @return              Returns a MLX Error Code
 */
mlx_error mlx_i2cWrite(uint8_t dev, uint16_t writeAddr, uint16_t nWrite, const uint16_t *data);

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

// ===================================================================
/**
 * Reads 832 words (16-bit) from the EEPROM Calibration Data
 * 
 * @param eeData Array to stores EEPROM data
 * 
 * @return Returns a MLX Error Code
 */
mlx_error mlx_dumpParamEE(uint16_t* eeData);

/**
 * Extract parameters from the EEPROM data.
 * 
 * @param eeData Array that stores the EEPROM data
 * @param calibration_data      address to the parameter struct
 * 
 * @return Returns an int status
 */
int mlx_extractParam(uint16_t* eeData, mlx_param* calibration_data);

/**
 * Reads the data from the RAM portion of the MLX90640 sensor.
 * This reads around 832 16-bit words from the RAM and stores it into the FrameData parameter.
 * However, users need to make sure to allocate 834 16-bit words to store the metadata of the status
 * register and control register.
 * 
 * @param frameData The data read from the MLX90640
 * 
 * @return MLX Error Status
 */
mlx_error mlx_getFrameData(uint16_t* frameData);

#endif /* __MLX_90640_H__ */