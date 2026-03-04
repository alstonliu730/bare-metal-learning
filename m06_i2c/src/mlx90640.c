#include <i2c.h>
#include <mlx90640.h>
#include <uart.h>
#include <gpio.h>
#include <common.h>
#include <timer.h>

// Extraction Declarations
static void ExtractVDDParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractPTATParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractGainParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractKsTaParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractKsToParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractTGCParam(uint16_t *eeData, mlx_param* calibration_data);
static void ExtractResolutionParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractAlphaParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractOffsetParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractKtaPixelParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractKvPixelParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractCPParam(uint16_t* eeData, mlx_param* calibration_data);
static void ExtractCILCParam(uint16_t* eeData, mlx_param* calibration_data);
static int ExtractDeviatingPixels(uint16_t* eeData, mlx_param* calibration_data);

static int checkAdjPixel(uint16_t pix1, uint16_t pix2);
static int checkEEDataValid(uint16_t* eeData);

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

    // set write buffer as the register address in big endian
    uint8_t reg[2] = { (uint8_t)(start >> 8), (uint8_t)(start & 0xFF) };
    
    err = i2c_writeReadRepeat(I2C_REG(BSC1_ADDR), dev, (void *)reg, MLX_REG_DLEN, (void *)data, (nRead << 1));

    // reverse the byte order in each 16-bit word back to little endian
    for(int i = 0; i < nRead; i++) {
        data[i] = __builtin_bswap16(data[i]);
    }

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
mlx_error mlx_i2cWrite(uint8_t dev, uint16_t writeAddr, uint16_t nWrite, uint16_t* data) {
    i2c_status err;

    // Create new array to insert the register address
    uint16_t* writeData = (uint16_t *) malloc(sizeof(uint16_t) * (nWrite + 1));

    // add register address to the write buffer in Big Endian
    writeData[0] = __builtin_bswap16(writeAddr);

    // Reverse bytes to big endian before sending the data
    for(int i = 1; i < (nWrite + 1); i++) {
        writeData[i] = __builtin_bswap16(data[i]);
    }

    // send the i2c send command
    err = i2c_blocking_send(I2C_REG(BSC1_ADDR), dev, ((nWrite + 1) << 1), (uint8_t *)writeData);
    
    // Free the memory from the writeData 
    free(writeData);

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
 * @param eeData Array to stores EEPROM data
 * 
 * @return Returns a MLX Error Code
 */
mlx_error mlx_dumpParamEE(uint16_t* eeData) {
    // Clear data from array
    memset(eeData, 0, MLX_EEPROM_LEN * sizeof(uint16_t));

    return mlx_i2cRead(MLX_DEV_ADDR, 0x2400, MLX_EEPROM_LEN, eeData);
}

/**
 * Extract parameters from the EEPROM data.
 * 
 * @param eeData Array that stores the EEPROM data
 * @param calibration_data      address to the parameter struct
 * 
 * @return Returns an int status
 */
int mlx_extractParam(uint16_t* eeData, mlx_param* calibration_data) {
    int err = checkEEDataValid(eeData);
    if (err != 0) {
        return err;
    }

    ExtractVDDParam(eeData, calibration_data);
    ExtractPTATParam(eeData, calibration_data);
    ExtractGainParam(eeData, calibration_data);
    ExtractTGCParam(eeData, calibration_data);
    ExtractResolutionParam(eeData, calibration_data);
    ExtractKsTaParam(eeData, calibration_data);
    ExtractKsToParam(eeData, calibration_data);
    ExtractCPParam(eeData, calibration_data);

    ExtractAlphaParam(eeData, calibration_data);
    ExtractOffsetParam(eeData, calibration_data);
    ExtractKtaPixelParam(eeData, calibration_data);
    ExtractKvPixelParam(eeData, calibration_data);
    ExtractCILCParam(eeData, calibration_data);
    
    return ExtractDeviatingPixels(eeData, calibration_data);
}
/**
 * Calculate VDD value from EEPROM Data and set it to the parameters
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractVDDParam(uint16_t* eeData, mlx_param* calibration_data) {
    // Calculate kVdd value & set it to the calibration data
    int16_t kVdd = (eeData[0x33] & 0xFF00) >> 8;

    if (kVdd > 127) { kVdd -= 256; }

    kVdd <<= 5; // shift kVdd by 5 bits
    calibration_data->kVdd = kVdd;

    // Calculate vdd25 & set it to the calibration data
    int16_t vdd25 = (eeData[0x33] & 0x00FF);
    vdd25 = ((vdd25 - 256) >> 5) - (1 << 13);

    calibration_data->vdd25 = vdd25;
}

/**
 * Calculate PTAT value from EEPROM Data and set it to the parameters
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractPTATParam(uint16_t* eeData, mlx_param* calibration_data) {
    // Calculate KvPTAT
    float KvPTAT = (float) ((eeData[0x32] & 0xFC00) >> 10);
    if (KvPTAT > 31.00) { KvPTAT -= 64.00; }

    KvPTAT /= LSHIFT(1, 12);
    calibration_data->KvPTAT = KvPTAT;

    // Calculate KtPTAT
    float KtPTAT = (eeData[0x32] & 0x03FF);
    if (KtPTAT > 511) { KtPTAT -= 1024; } 

    KtPTAT /= LSHIFT(1, 3);
    calibration_data->KtPTAT = KtPTAT;
    
    // Calculate vPTAT25
    int16_t vPTAT25 = eeData[0x31];

    calibration_data->vPTAT25 = vPTAT25;

    // Calculate alpha PTAT
    float alphaPTAT = (float)((eeData[0x10] & 0xF000) >> 14) + 8.0f;
    calibration_data->alphaPTAT = alphaPTAT;
}

/**
 * Calculate Gain value from EEPROM Data and set it to the parameters
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractGainParam(uint16_t* eeData, mlx_param* calibration_data) {
    int16_t gainEE = eeData[0x30];
    
    if ((uint32_t)gainEE > 32767) { gainEE -= 65536; }

    calibration_data->gainEE = gainEE;
}

/**
 * Calculate KsTa value from EEPROM Data and set it to the parameters
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractKsTaParam(uint16_t* eeData, mlx_param* calibration_data) {
    int16_t KsTaEE = (eeData[0x3C] & 0xFF00) >> 8;
    if (KsTaEE > 127) { KsTaEE -= 256; }

    calibration_data->KsTa = (float) KsTaEE / (LSHIFT(1, 13));
}

/**
 * Calculate TGC value from EEPROM Data and set it to the parameters
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
void ExtractTGCParam(uint16_t *eeData, mlx_param* calibration_data) {
    float tgc = eeData[0x3C] & 0x00FF;
    if(tgc > 127) { tgc = tgc - 256; }

    tgc /= 32.0f;
    
    calibration_data->tgc = tgc;
}


/**
 * Calculate Resolution Calibration value from EEPROM Data and set it to the parameters
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractResolutionParam(uint16_t* eeData, mlx_param* calibration_data) {
    uint8_t resEE = (eeData[0x38] & 0x3000) >> 12;

    calibration_data->resolutionEE = resEE;
}

/**
 * Calculate KsTo & Corner Temperature value from EEPROM Data and set it to the parameters.
 * - KsTo Range 1-4 = Sensitivity changes with object temp. across 4 ranges (CT1-CT4)
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractKsToParam(uint16_t* eeData, mlx_param* calibration_data) {
    int32_t KsToScale;
    int8_t step;

    step = ((eeData[0x3F] & 0x3000) >> 12) * 10;

    // Calculate the Corner Temperatures
    calibration_data->ct[0] = -40; // hard-coded in Celcius
    calibration_data->ct[1] = 0;
    calibration_data->ct[2] = ((eeData[0x3F] & 0x00F0) >> 4) * step;
    calibration_data->ct[3] = ((eeData[0x3F] & 0x0F00) >> 8) * step + calibration_data->ct[2];
    calibration_data->ct[4] = 400; // hard-coded

    // Calculate the KsTo Scale
    KsToScale = (eeData[0x3F] & 0x000F) + 8;
    KsToScale = LSHIFT(1, KsToScale);

    // Calculate KsTo1
    calibration_data->KsTo[0] = (eeData[0x3D] & 0x00FF);
    if (calibration_data->KsTo[0] > 127) { calibration_data->KsTo[0] -= 256; }
    calibration_data->KsTo[0] /= KsToScale;

    // Calculate KsTo2
    calibration_data->KsTo[1] = (eeData[0x3D] & 0xFF00) >> 8;
    if (calibration_data->KsTo[1] > 127) { calibration_data->KsTo[1] -= 256; }
    calibration_data->KsTo[1] /= KsToScale;
    
    // Calculate KsTo3
    calibration_data->KsTo[2] = (eeData[0x3E] & 0x00FF);
    if (calibration_data->KsTo[2] > 127) { calibration_data->KsTo[2] -= 256; }
    calibration_data->KsTo[2] /= KsToScale;

    // Calculate KsTo4
    calibration_data->KsTo[3] = (eeData[0x3E] & 0xFF00) >> 8;
    if (calibration_data->KsTo[3] > 127) { calibration_data->KsTo[3] -= 256; }
    calibration_data->KsTo[3] /= KsToScale;

    // Hardcode the KsTo5
    calibration_data->KsTo[4] = -0.0002;
}

/**
 * Calculate Alpha Coefficient value from EEPROM Data and set it to the parameters.
 * Check 11.1.4, 11.1.11, 11.1.12 in the MLX90640 Datasheet
 * 
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractAlphaParam(uint16_t* eeData, mlx_param* calibration_data) {
    int accRow[24];
    int accCol[32];
    int p = 0;
    float alphaTemp[768]; // store coefficient temporary

    // Set Alpha Scale and Reference values
    int alphaRef = eeData[0x21];
    uint8_t alphaScale = ((eeData[0x20] & 0xF000) >> 12) + 30;
    uint8_t accRowScale = (eeData[0x20] & 0x0F00) >> 8;
    uint8_t accColScale = (eeData[0x20] & 0x00F0) >> 4;
    uint8_t accRemScale = eeData[0x20] & 0x000F;

    // Set the ACC Row values
    for (int i = 0; i < 6; i++) {
        p = i * 4; // set the pointer to the associated row
        accRow[p + 0] = (eeData[0x22 + i] & 0x000F);
        if (accRow[p + 0] > 7) { accRow[p + 0] -= 16; }

        accRow[p + 1] = (eeData[0x22 + i] & 0x00F0) >> 4;
        if (accRow[p + 1] > 7) { accRow[p + 1] -= 16; }

        accRow[p + 2] = (eeData[0x22 + i] & 0x0F00) >> 8;
        if (accRow[p + 2] > 7) { accRow[p + 2] -= 16; }

        accRow[p + 3] = (eeData[0x22 + i] & 0xF000) >> 12;
        if (accRow[p + 3] > 7) { accRow[p + 3] -= 16; }
    }
    
    // Set the ACC Col values
    for (int i = 0; i < 8; i++) {
        p = i * 4; // set the pointer to the associated column
        accCol[p + 0] = (eeData[0x28 + i] & 0x000F);
        if (accCol[p + 0] > 7) { accCol[p + 0] -= 16; }

        accCol[p + 1] = (eeData[0x28 + i] & 0x00F0) >> 4;
        if (accCol[p + 1] > 7) { accCol[p + 1] -= 16; }

        accCol[p + 2] = (eeData[0x28 + i] & 0x0F00) >> 8;
        if (accCol[p + 2] > 7) { accCol[p + 2] -= 16; }

        accCol[p + 3] = (eeData[0x28 + i] & 0xF000) >> 12;
        if (accCol[p + 3] > 7) { accCol[p + 3] -= 16; }
    }

    // Set the alpha sensitivity
    for (int r = 0; r < 24; r++) {
        for (int c = 0; c < 32; c++) {
            p = r * 32 + c; // set the pointer to the associated pixels
            alphaTemp[p] = (eeData[0x40 + p] & 0x03F0) >> 4;
            if (alphaTemp[p] > 31) { alphaTemp[p] -= 64; }

            alphaTemp[p] *= LSHIFT(1, accRemScale);
            alphaTemp[p] += (accRow[r] * LSHIFT(1, accRowScale));
            alphaTemp[p] += (accCol[c] * LSHIFT(1, accColScale));
            alphaTemp[p] += alphaRef;
            
            alphaTemp[p] /= (double) LSHIFT(1, alphaScale);
            alphaTemp[p] = alphaTemp[p] - calibration_data->tgc * ((calibration_data->cpAlpha[0] + calibration_data->cpAlpha[1]) / 2);
            alphaTemp[p] = SCALEALPHA / alphaTemp[p];
        }
    }

    float temp = alphaTemp[0];
    // get max alpha
    for (int i = 1; i < 768; i++) {
        if (alphaTemp[p] > temp) { temp = alphaTemp[i]; }
    }

    alphaScale = 0;
    while (temp < 32768) {
        temp *= 2; 
        alphaScale = alphaScale + 1;
    }

    for(int i = 0; i < 768; i++) {
        temp = alphaTemp[i] * (double) LSHIFT(1, alphaScale);
        calibration_data->alpha[i] = (temp + 0.5f);
    }

    calibration_data->alphaScale = alphaScale;
}

/**
 * Calculate Offset value from EEPROM Data and set it to the parameters.
 * Check 11.1.3/11.1.3.1 in the MLX90640 Datasheet
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractOffsetParam(uint16_t* eeData, mlx_param* calibration_data) {
    int occRow[24];
    int occCol[32];
    int p = 0;

    int16_t offsetRef = eeData[0x11];
    if ((uint32_t)offsetRef > 32767) { offsetRef -= 65536; }

    uint8_t occRowScale = BITS(eeData[0x10], 11, 8);
    uint8_t occColScale = BITS(eeData[0x10],  7, 4);
    uint8_t occRemScale = BITS(eeData[0x10],  3, 0);

    // Set the OCC Row values
    for (int i = 0; i < 6; i++) {
        p = i * 4; // set the pointer to the associated row
        occRow[p + 0] = (eeData[0x12 + i] & 0x000F);
        if (occRow[p + 0] > 7) { occRow[p + 0] -= 16; }

        occRow[p + 1] = (eeData[0x12 + i] & 0x00F0) >> 4;
        if (occRow[p + 1] > 7) { occRow[p + 1] -= 16; }

        occRow[p + 2] = (eeData[0x12 + i] & 0x0F00) >> 8;
        if (occRow[p + 2] > 7) { occRow[p + 2] -= 16; }

        occRow[p + 3] = (eeData[0x12 + i] & 0xF000) >> 12;
        if (occRow[p + 3] > 7) { occRow[p + 3] -= 16; }
    }
    
    // Set the OCC Col values
    for (int i = 0; i < 8; i++) {
        p = i * 4; // set the pointer to the associated column
        occCol[p + 0] = (eeData[0x18 + i] & 0x000F);
        if (occCol[p + 0] > 7) { occCol[p + 0] -= 16; }

        occCol[p + 1] = (eeData[0x18 + i] & 0x00F0) >> 4;
        if (occCol[p + 1] > 7) { occCol[p + 1] -= 16; }

        occCol[p + 2] = (eeData[0x18 + i] & 0x0F00) >> 8;
        if (occCol[p + 2] > 7) { occCol[p + 2] -= 16; }

        occCol[p + 3] = (eeData[0x18 + i] & 0xF000) >> 12;
        if (occCol[p + 3] > 7) { occCol[p + 3] -= 16; }
    }

    // set the offset values in the calibration data
    for (int r = 0; r < 24; r++) {
        for (int c = 0; c < 32; c++) {
            p = 32 * r + c;

            calibration_data->offset[p] = (eeData[0x40 + p] & 0xFC00) >> 10;
            if (calibration_data->offset[p] > 31) { calibration_data->offset[p] -= 64; }

            calibration_data->offset[p] *= LSHIFT(1, occRemScale);
            calibration_data->offset[p] += (offsetRef + (occRow[r] << occRowScale) + (occCol[c] << occColScale));
        }
    }
}

/**
 * Calculate Kta value per pixel from EEPROM Data and set it to the parameters.
 * Check 11.1.3/11.1.3.1 in the MLX90640 Datasheet.
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractKtaPixelParam(uint16_t* eeData, mlx_param* calibration_data) {
    int8_t KtaRC[4];
    uint8_t ktaScale1;
    uint8_t ktaScale2;
    float ktaTemp[768];
    int p = 0;
    uint8_t split;
    
    // R (odd) + C (odd)
    KtaRC[0] = (eeData[0x36] & 0xFF00) >> 8;
    if ((uint8_t)KtaRC[0] > 127) { KtaRC[0] -= 256; }

    // R (even) + C (odd)
    KtaRC[2] = (eeData[0x36] & 0x00FF);
    if ((uint8_t)KtaRC[2] > 127) { KtaRC[2] -= 256; }

    // R (odd) + C (even)
    KtaRC[1] = (eeData[0x37] & 0xFF00) >> 8;
    if ((uint8_t)KtaRC[1] > 127) { KtaRC[1] -= 256; }

    // R (even) + C (even)
    KtaRC[3] = (eeData[0x37] & 0x00FF);
    if ((uint8_t)KtaRC[3] > 127) { KtaRC[3] -= 256; }

    ktaScale1 = ((eeData[0x38] & 0x00F0) >> 8) + 8;
    ktaScale2 = (eeData[0x38] & 0x000F);

    for(int r = 0; r < 24; r++)
    {
        for(int c = 0; c < 32; c++)
        {
            p = 32 * r + c;
            split = 2*(p/32 - (p/64)*2) + p%2;
            ktaTemp[p] = (eeData[0x40 + p] & 0x000E) >> 1;
            if (ktaTemp[p] > 3) { ktaTemp[p] -= 8; }
            
            ktaTemp[p] = ktaTemp[p] * (1 << ktaScale2);
            ktaTemp[p] = KtaRC[split] + ktaTemp[p];
            ktaTemp[p] /= LSHIFT(1, ktaScale1);
            //ktaTemp[p] = ktaTemp[p] * mlx90640->offset[p];
        }
    }

    // get max kta value
    float temp = fabs(ktaTemp[0]);
    for (int i = 1; i < 768; i++) {
        if (fabs(ktaTemp[i]) > temp) { temp = fabs(ktaTemp[i]); }
    }

    ktaScale1 = 0;
    while (temp < 64) {
        temp = temp * 2;
        ktaScale1++;
    }

    for (int i = 0; i < 768; i++) {
        temp = ktaTemp[i] * LSHIFT(1, ktaScale1);
        if (temp < 0) {
            calibration_data->kta[i] = (temp - 0.5f);
        } else {
            calibration_data->kta[i] = (temp + 0.5f);
        }        
    }

    calibration_data->ktaScale = ktaScale1;
}

/**
 * Calculate Kv value per pixel from EEPROM Data and set it to the parameters.
 * Check 11.1.3/11.1.3.1 in the MLX90640 Datasheet.
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractKvPixelParam(uint16_t* eeData, mlx_param* calibration_data) {
    int8_t kvT[4];
    uint8_t kvScale;
    float kvTemp[768];
    uint8_t split;

    // R (odd) + C (odd)
    kvT[0] = (eeData[0x34] & 0xF000) >> 12;
    if (kvT[0] > 7) { kvT[0] -= 16; }

    // R (even) + C (odd)
    kvT[2] = (eeData[0x34] & 0x0F00) >> 8;
    if (kvT[2] > 7) { kvT[2] -= 16; }

    // R (odd) + C (even)
    kvT[1] = (eeData[0x34] & 0x00F0) >> 4;
    if (kvT[1] > 7) { kvT[1] -= 16; }

    // R (even) + C (even)
    kvT[3] = (eeData[0x34] & 0x000F);
    if (kvT[3] > 7) { kvT[3] -= 16; }

    kvScale = (eeData[0x38] & 0x0F00) >> 8;

    for(int r = 0; r < 24; r++) {
        for(int c = 0; c < 32; c++) {
            int p = 32 * r + c;
            split = 2*(p/32 - (p/64)*2) + p%2;
            kvTemp[p] = kvT[split];
            kvTemp[p] /= ((double) LSHIFT(1, kvScale));
        }
    }

     // get max kv value
    float temp = fabs(kvTemp[0]);
    for (int i = 1; i < 768; i++) {
        if (fabs(kvTemp[i]) > temp) { temp = fabs(kvTemp[i]); }
    }

    kvScale = 0;
    while (temp < 64) {
        temp = temp * 2;
        kvScale++;
    }

    for (int i = 0; i < 768; i++) {
        temp = kvTemp[i] * LSHIFT(1, kvScale);
        if (temp < 0) {
            calibration_data->kv[i] = (temp - 0.5f);
        } else {
            calibration_data->kv[i] = (temp + 0.5f);
        }        
    }

    calibration_data->kvScale = kvScale;
}

/**
 * Calculate Parameters for Compensation Pixel from the given EEPROM Data and set it to the parameters.
 * Check 11.1.12, 11.1.13, 11.1.14, & 11.1.15 in the MLX90640 Datasheet.
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractCPParam(uint16_t* eeData, mlx_param* calibration_data) {
    float alphaSP[2];       // alpha coefficient for CP subpages 0 & 1
    int16_t offsetSP[2];    // Offset values for CP subpages 0 & 1
    float cpKv;
    float cpKta;
    uint8_t alphaScale;
    uint8_t ktaScale1;
    uint8_t kvScale;

    alphaScale = ((eeData[0x20] & 0xF000) >> 12) + 27;

    offsetSP[0] = (eeData[0x3A] & 0x3FF);           // offset_CP_sp0
    if (offsetSP[0] > 511) { offsetSP[0] -= 1024; }

    offsetSP[1] = (eeData[0x3A] & 0xFC00) >> 10;    // offset_CP_sp1_delta
    if (offsetSP[1] > 31) { offsetSP[1] -= 64; }
    offsetSP[1] += offsetSP[0];

    alphaSP[0] = (eeData[0x39] & 0x03FF);           // alpha CP sp 0
    if (alphaSP[0] > 511) { alphaSP[0] -= 1024; }
    alphaSP[0] /= ((double) LSHIFT(1, alphaScale));

    alphaSP[1] = (eeData[0x39] & 0xFC00) >> 10;     // CP_P1/P0 ratio
    if (alphaSP[1] > 31) { alphaSP[1] -= 64; }
    alphaSP[1] = alphaSP[0] * (1 + alphaSP[1] / ((double) LSHIFT(1, 7))); // alpha CP sp 1 

    cpKta = (eeData[0x3B] & 0x00FF);                    // Kta_CP_EE
    if (cpKta > 127) { cpKta -= 256; }

    ktaScale1 = ((eeData[0x38] & 0x00F0) >> 4) + 8;     // Kta_scale1
    cpKta /= ((double) LSHIFT(1, ktaScale1));
    calibration_data->cpKta = cpKta;                    // Kta_CP

    cpKv = (eeData[0x3B] & 0xFF00) >> 8;                // Kv_CP_EE
    if (cpKv > 127) { cpKv -= 256; }
    
    kvScale = (eeData[0x38] & 0x0F00) >> 8;             // Kv_Scale
    cpKv /= ((double) LSHIFT(1, kvScale));
    calibration_data->cpKv = cpKv;                      // Kv_CP

    calibration_data->cpAlpha[0] = alphaSP[0];
    calibration_data->cpAlpha[1] = alphaSP[1];
    calibration_data->cpOffset[0] = offsetSP[0];
    calibration_data->cpOffset[1] = offsetSP[1];
}

/**
 * Calculate offset for Interleaved Pattern values from the given EEPROM Data and set it to the parameters.
 * Check 11.1.3.1 in the MLX90640 Datasheet.
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static void ExtractCILCParam(uint16_t* eeData, mlx_param* calibration_data) {
    float ilChessC[3];
    uint8_t calibrationModeEE;
    
    calibrationModeEE = (eeData[0x0A] & 0x0800) >> 4;           
    calibrationModeEE ^= 0x80;

    ilChessC[0] = (eeData[0x35] & 0x003F);
    if (ilChessC[0] > 31) { ilChessC[0] = ilChessC[0] - 64; }
    ilChessC[0] /= 16.0f;
    
    ilChessC[1] = (eeData[0x35] & 0x07C0) >> 6;
    if (ilChessC[1] > 15) { ilChessC[1] = ilChessC[1] - 32; }
    ilChessC[1] /= 2.0f;
    
    ilChessC[2] = (eeData[0x35] & 0xF800) >> 11;
    if (ilChessC[2] > 15) { ilChessC[2] = ilChessC[2] - 32; }
    ilChessC[2] /= 8.0f;
    
    calibration_data->calibrationModeEE = calibrationModeEE;
    calibration_data->ilChessC[0] = ilChessC[0];
    calibration_data->ilChessC[1] = ilChessC[1];
    calibration_data->ilChessC[2] = ilChessC[2];
}

/**
 * Calculate if there are outliers in the EEPROM data.
 * 
 * @param eeData                EEPROM data from the sensor
 * @param calibration_data      address to the parameter struct
 */
static int ExtractDeviatingPixels(uint16_t* eeData, mlx_param* calibration_data) {
    uint16_t pixCnt = 0;
    uint16_t brokenCnt = 0;
    uint16_t outlierCnt = 0;
    int stat = 0;

    for(pixCnt = 0; pixCnt < 5; pixCnt ++) {
        calibration_data->brokenPixels[pixCnt] = 0xFFFF;
        calibration_data->outlierPixels[pixCnt] = 0xFFFF;
    }
    pixCnt = 0; 
    while(pixCnt < 768 && brokenCnt < 5 && outlierCnt < 5) {
        uint16_t pixelParam = eeData[0x40 + pixCnt];
        if (pixelParam == 0) {
            calibration_data->brokenPixels[brokenCnt] = pixCnt;
            brokenCnt += 1;
        } else if ((pixelParam & 0x0001) != 0) {
            calibration_data->outlierPixels[outlierCnt] = pixCnt;
            outlierCnt += 1;
        }
        pixCnt++;
    }

    if (brokenCnt > 4) { stat = -3; }
    else if (outlierCnt > 4) { stat = -4; }
    else if ((brokenCnt + outlierCnt) > 4) { stat = -5; }
    else {
        // Check Adjacent broken pixels
        for(pixCnt = 0; pixCnt < brokenCnt; pixCnt++) {
            for(int i = pixCnt + 1; i < brokenCnt; i++) {
                stat = checkAdjPixel(calibration_data->brokenPixels[pixCnt], calibration_data->brokenPixels[i]);
                if (stat != 0) { return stat; }
            }
        }

        // Check Adjacent outlier pixels
        for(pixCnt = 0; pixCnt < outlierCnt; pixCnt++) {
            for(int i = pixCnt + 1; i < outlierCnt; i++) {
                stat = checkAdjPixel(calibration_data->outlierPixels[pixCnt], calibration_data->outlierPixels[i]);
                if (stat != 0) { return stat; }
            }
        }

        // Check Outlier & broken Pixels
        for(pixCnt = 0; pixCnt < brokenCnt; pixCnt++) {
            for(int i = 0; i < outlierCnt; i++) {
                stat = checkAdjPixel(calibration_data->brokenPixels[pixCnt], calibration_data->outlierPixels[i]);
                if (stat != 0) { return stat; }
            }
        }
    }
    return stat;
}

/**
 * Checks the difference between the pixels
 */
static int checkAdjPixel(uint16_t pix1, uint16_t pix2) {
    int diffPixel = pix1 - pix2;
    if ((diffPixel > -34 && diffPixel < -30) ||
        (diffPixel > -2 && diffPixel < 2) ||
        (diffPixel > 30 && diffPixel < 34)) {
            return -6;
    }
    return 0;
}

static int checkEEDataValid(uint16_t* eeData) {
    int devSel = eeData[0x0A] & 0x0040;
    if (devSel == 0) {
        return 0;
    }
    return -7;
}

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
mlx_error mlx_getFrameData(uint16_t* frameData) {
    uint16_t dataReady = 0;
    mlx_error err;
    uint16_t statusReg;
    uint16_t controlReg;
    static const uint16_t clearStatReg = 0x0030;

    // Check if the data is ready
    while (dataReady == 0) {
        err = mlx_i2cRead(MLX_DEV_ADDR, MLX_STATUS, 1, &statusReg);
        if(err != MLX_SUCCESS) {
            return err;
        }
        dataReady = statusReg & BIT(3);
    }
    
    // clear the status register
    err = mlx_i2cWrite(MLX_DEV_ADDR, MLX_STATUS, 1, &clearStatReg);
    if (err != MLX_SUCCESS) {
        return err;
    }

    // read the frame data from RAM (pixel data + aux data)
    err = mlx_i2cRead(MLX_DEV_ADDR, MLX_FRAME_ADDR_START, MLX_PIXEL_LEN, frameData);
    if (err != MLX_SUCCESS) {
        return err;
    }
    
    // read the status register
    err = mlx_i2cRead(MLX_DEV_ADDR, MLX_STATUS, 1, &statusReg);
    if (err != MLX_SUCCESS) {
        return err;
    }

    // read the controll register
    err = mlx_i2cRead(MLX_DEV_ADDR, MLX_CTRL1, 1, &controlReg);
    if (err != MLX_SUCCESS) {
        return err;
    }

    // store the control register into the frame data resulting array
    frameData[832] = controlReg;
    
    // store the subpage data into the frame data array
    frameData[833] = (statusReg & BIT(0));
    
    // Validate Aux & Frame data
}