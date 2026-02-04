#include <i2c.h>
#include <common.h>
#include <gpio.h>
#include <timer.h>
#include <uart.h>

// Set GPIO SDA/SCL Pinout (BSC1)
#define SDA1_GPIO       2         
#define SCL1_GPIO       3

/**
 * Setup the master device for the i2c bus.
 * - Default SCL Freq = 100 kHz assuming core clock speed as 150 MHz
 * - CDIV rounds down to an even number
 * - Default value result in 100 kHz I2C clock frequency
 * 
 * @param bus           i2c bus base address
 * @param sda_pin       GPIO number for SDA line
 * @param scl_pin       GPIO number for SCL line
 */
void i2c_init(volatile i2c_reg_t* bus, uint8_t sda_pin, uint8_t scl_pin) {
    // set sda pin function & pull up/down resistor
    gpio_function(sda_pin, GPIO_FUNCTION_ALT0);
    gpio_pull(sda_pin, PULL_UP);

    // set scl pin function & pull up/down resistor
    gpio_function(scl_pin, GPIO_FUNCTION_ALT0);
    gpio_pull(sda_pin, PULL_UP);

    // set SCL Clock Divisor
    bus->div = DEFAULT_CDIV;
}

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
i2c_status i2c_blocking_recv(volatile i2c_reg_t* bus, uint8_t dev, uint8_t reg, uint32_t size, uint8_t* buf) {
    // check if the i2c bus is enabled
    if (!(bus->control & C_ENABLE)) {
        return I2C_NOT_ENABLED;
    }

    // wait for any other transfers
    while (bus->status & S_TA);

    // track number of bytes read
    uint32_t count = 0;

    // ----- Write internal register address to i2c device -----

    // Reset fifo and status
    mmio_write(bus->control, mmio_read(bus->control) | C_CLEAR);
    mmio_write(bus->status, S_DONE);

    // set the data length
    mmio_write(bus->dlen, READ_REG_DLEN);

    // set the slave address (7-bit addressing)
    mmio_write(bus->addr, (dev << 1));

    // write the internal register 
    mmio_write(bus->fifo, reg);

    // initiate write transfer
    mmio_write(bus->control, (C_START));

    // wait till transfer starts and returns done
    while (!(bus->status & S_TA) && !(bus->status & S_DONE));

    // ----- Read from the i2c device -----

    // reset done status
    mmio_write(bus->status, S_DONE);

    // write number of data bytes to read to the dlen
    mmio_write(bus->dlen, size);

    // initiate read transfer (READ = 1, ST = 1)
    mmio_write(bus->control, (C_CLEAR | C_START | C_READ));
    wait_ms(50);

    // read from the fifo until done
    while ((bus->status & S_TA) && !(bus->status & S_DONE) && !(bus->status & S_CLKTOUT)) {
        // Check if the fifo contains data
        if (bus->status & S_RXD) {  
            buf[count++] = mmio_read(bus->fifo) & BIT_MASK(7, 0);
        }
    }
    
    if (count != size) {
        return I2C_DATA_LOSS;
    } else if (bus->status & S_CLKTOUT) {
        return I2C_CLK_TIMEOUT;
    } else {
        return I2C_SUCCESS;
    }
}

/**
 * Polling method for sending data to the slave drive. Waits until the end of transmission.
 * 
 * @param base  i2c bus base address
 * @param dev   device address to the slave drive
 * @param buf   address to the buffer
 * @param size  number of bytes in the buffer
 * 
 * @return An i2c status code
 */
i2c_status i2c_blocking_send(volatile i2c_reg_t* bus, uint8_t dev, uint8_t reg, uint32_t size, uint8_t* buf) {
    // Check if the i2c bus is enabled
    if (bus->control & C_ENABLE) {
        return I2C_NOT_ENABLED;
    }

    if (size == 0 || buf == NULL) {
        return I2C_INV_INPUT;
    }

    // wait for any other transfers
    while ((bus->status & S_TA));

    // Clear the Done Status
    mmio_write(bus->status, S_DONE);

    // clear the FIFO
    mmio_write(bus->control, mmio_read(bus->control) | (C_CLEAR));

    // set slave address
    mmio_write(bus->addr, (dev << 1));

    // set number of bytes to write
    mmio_write(bus->dlen, size + 1);

    // set register address
    mmio_write(bus->fifo, reg);

    // write to the FIFO until full or the buffer is empty
    uint32_t count = 0;
    while ((bus->status & S_TXD) && count < size) {
        mmio_write(bus->fifo, buf[count++]);
    }

    // start write transfer
    mmio_write(bus->control, mmio_read(bus->control) | (C_START));
    wait_ms(50);

    // check for active transfer and see if there's still data to write to
    while ((bus->status & S_TA) && !(bus->status & S_DONE) && !(bus->status & S_CLKTOUT)) {
        // check if there's still data in the buffer
        if ((bus->status & S_TXD) && count < size) {    
            mmio_write(bus->fifo, buf[count++]);
        }

        // Return the Acknowledge Error 
        if (bus->status & S_ERR) {
            // disable the i2c controller bus
            mmio_write(bus->control, mmio_read(bus->control) & (C_DISABLE | C_CLEAR));

            uart_printf("i2c_blocking_recv: ACK Error detected.\n");
            return I2C_ACK_ERR;
        }
    }

    if (bus->status & S_CLKTOUT) {
        return I2C_CLK_TIMEOUT;
    } else {
        return I2C_SUCCESS;
    }
}


