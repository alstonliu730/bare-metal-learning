#include <i2c.h>
#include <mlx90640.h>
#include <uart.h>
#include <gpio.h>
#include <common.h>
#include <timer.h>

/**
 * Initialize the I2C Bus associated for the MLX90640.
 */
inline void mlx_i2cInit() {
    i2c_init(I2C_REG(BSC1_ADDR), GPIO_SDA1, GPIO_SCL1);
}

/**
 * Reads a number of words from a selected MLX90640 device memory starting from the given address.
 * Stores the data in the memory location given by the user.
 * 
 * @param dev           address of the MLX90640 (default: 0x33)
 * @param start         start address from the MLX90640 memory to be read
 *                      RAM [0x0400 - 0x073F];  EEPROM [0x2400 - 0x273F]
 * @param nRead         Number of 16-bits words to be read
 * @param data          pointer to the memory location where the data is stored
 * 
 * @return              Returns a MLX Error Code
 */
mlx_error mlx_i2cRead(uint8_t dev, uint16_t start, uint16_t nRead, uint16_t *data) {
    i2c_status err;

    uint8_t reg[2] = { (uint8_t)(start >> 8), (uint8_t)(start & 0xFF) };
    
    err = i2c_writeReadRepeat(I2C_REG(BSC1_ADDR), dev, (void *)reg, MLX_REG_DLEN, (void *)data, (nRead << 1));

    // Translate Error Code
    switch (err) {
        case I2C_SUCCESS:
            return MLX_SUCCESS;
        case I2C_ACK_ERR:
            return MLX_NACK;
        case I2C_DATA_LOSS:
            return MLX_CORRUPT;
        default:
            return MLX_UNKNOWN_ERR;
    }
}

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
mlx_error mlx_i2cWrite(uint8_t dev, uint16_t writeAddr, uint16_t nWrite, uint16_t *data) {
    i2c_status code = i2c_blocking_send(I2C_REG(BSC1_ADDR), dev, writeAddr, (nWrite << 1), (uint8_t *)data);

    // Translate Error Code
    switch (code) {
        case I2C_SUCCESS:
            return MLX_SUCCESS;
        case I2C_ACK_ERR:
            return MLX_NACK;
        case I2C_DATA_LOSS:
            return MLX_CORRUPT;
        default:
            return MLX_UNKNOWN_ERR;
    }
}

/**
 * Get the 16-bit value of the control register 1 in the internal regs.
 * 
 * @return Control Register 1 value
 */
uint16_t mlx_getCtrlReg1() {
    uint16_t res = 0;

    // use i2c to receive control reg
    mlx_error stat = mlx_i2cRead(MLX_DEV_ADDR, MLX_CTRL1, 1, &res);
    if (stat != MLX_SUCCESS) {
        uart_printf("MLX90640: getCtrlReg1 received an error: %d\n", (int) stat);
    }

    // return the result
    return res;
}

/**
 * Get the 16-bit value of the status register in the internal regs.
 * 
 * @return Status Register Value
 */
uint16_t mlx_getStatusReg() {
    uint16_t res = 0;

    // use i2c to receive status reg
    mlx_error stat = mlx_i2cRead(MLX_DEV_ADDR, MLX_STATUS, 1, &res);
    if (stat != MLX_SUCCESS) {
        uart_printf("MLX90640: getStatusReg received an error: %d\n", (int) stat);
    }

    // return the result
    return res;
}

/**
 * Get the 8-bit value of the i2c address in the regs
 * 
 * @return I2C Address
 */
uint16_t mlx_getI2CAddr() {
    uint16_t res = 0;

    // use i2c to receive status reg
    mlx_error stat = mlx_i2cRead(MLX_DEV_ADDR, MLX_I2C_ADDR, 1, &res);
    if (stat != MLX_SUCCESS) {
        uart_printf("MLX90640: getStatusReg received an error: %d\n", (int) stat);
    }

    // return the result
    return res;
}

/**
 * Reads 832 words (16-bit) from the EEPROM Calibration Data
 * 
 * @param eeData The array to store the calibration data in
 * 
 * @return Returns a MLX Error Code
 */
mlx_error mlx_dumpParamEE(uint16_t* eeData) {
    // Clear data from array
    memset(eeData, 0, MLX_EEPROM_LEN * sizeof(uint16_t));

    uint8_t reg[2] = { (uint8_t)(MLX_EEPROM_ADDR_START >> 8), (uint8_t)(MLX_EEPROM_ADDR_START & 0xFF) };

    // Read data from the sensor (blocking)
    i2c_status err = i2c_writeReadRepeat(I2C_REG(BSC1_ADDR), MLX_DEV_ADDR, reg, MLX_REG_DLEN, (void *) eeData, (MLX_EEPROM_LEN << 1));

    switch (err) {
        case I2C_SUCCESS:
            return MLX_SUCCESS;
        case I2C_ACK_ERR:
            return MLX_NACK;
        case I2C_DATA_LOSS:
            return MLX_CORRUPT;
        default:
            return MLX_UNKNOWN_ERR;
    }
}

/**
 * Calculate VDD value from EEPROM Data and set it to the parameters
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
void ExtractVDDParam(uint16_t* eeData, mlx_param* calibration_data) {
    // Calculate kVdd value & set it to the calibration data
    int16_t kVdd = (eeData[MLX_EE_IDX(33)] & 0xFF00) >> 8;

    if (kVdd > 127) { kVdd -= 256; }

    kVdd <<= 5; // shift kVdd by 5 bits
    calibration_data->kVdd = kVdd;

    // Calculate vdd25 & set it to the calibration data
    int16_t vdd25 = (eeData[MLX_EE_IDX(33)] & 0x00FF);
    vdd25 = ((vdd25 - 256) >> 5) - (1 << 13);

    calibration_data->vdd25 = vdd25;
}

/**
 * Calculate PTAT value from EEPROM Data and set it to the parameters
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
void ExtractPTATParam(uint16_t* eeData, mlx_param* calibration_data) {
    // Calculate KvPTAT
    float KvPTAT = (float) ((eeData[MLX_EE_IDX(32)] & 0xFC00) >> 10);
    if (KvPTAT > 31.00) { KvPTAT -= 64.00; }

    KvPTAT /= LSHIFT(1, 12);
    calibration_data->KvPTAT = KvPTAT;

    // Calculate KtPTAT
    float KtPTAT = (eeData[MLX_EE_IDX(32)] & 0x03FF);
    if (KtPTAT > 511) { KtPTAT -= 1024; } 

    KtPTAT /= LSHIFT(1, 3);
    calibration_data->KtPTAT;
    
    // Calculate vPTAT25
    int16_t vPTAT25 = eeData[MLX_EE_IDX(31)];

    calibration_data->vPTAT25 = vPTAT25;

    // Calculate alpha PTAT
    float alphaPTAT = ((eeData[MLX_EE_IDX(10)] & 0xF000) >> 12) / 4 + 8;
    calibration_data->alphaPTAT = alphaPTAT;
}




