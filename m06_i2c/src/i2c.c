#include <i2c.h>
#include <common.h>
#include <gpio.h>
#include <timer.h>
#include <uart.h>
#include <mb.h>

#define DETECT_FORMAT       "    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F\n"

static inline uint64_t critical_section_enter(void) {
    uint64_t daif;
    asm volatile("mrs %0, daif" : "=r"(daif));   // Save current DAIF
    asm volatile("msr daifset, #0xF");            // Mask all: D, A, I, F
    asm volatile("isb");                          // Ensure mask takes effect
    return daif;
}

static inline void critical_section_exit(uint64_t saved_daif) {
    asm volatile("msr daif, %0" :: "r"(saved_daif));  // Restore original DAIF
    asm volatile("isb");
}

// Get Core Clock Speed
static uint32_t get_core_clk() {
    // set mbox array
    mbox[0] = 8 * sizeof(uint32_t);
    mbox[1] = MBOX_REQUEST;

    mbox[2] = MBOX_TAG_GETCLK_MEAS;
    mbox[3] = 8;
    mbox[4] = MBOX_REQUEST;
    mbox[5] = MBOX_CLK_CORE;
    mbox[6] = 0;
    mbox[7] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP) && mbox[1] == MBOX_SUCCESS) {
        return mbox[6];
    }
    return 0;
}

/**
 * Set the clock speed on the I2C Master by setting the Divisor Register.
 * 
 * @param bus           i2c bus base address
 * @param clkSpeed      i2c clock speed
 */
void i2c_setClock(volatile i2c_reg_t* bus, i2c_clk_mode clkSpeed) {
    if (clkSpeed <= 0) {
        uart_printf("i2c_setClock: Invalid Clock Speed Input.\n");
        return;
    }

    bus->div = (uint16_t)(get_core_clk() / clkSpeed);
    // bus->div = 0x05DC;
}

/**
 * Setup the master device for the i2c bus.
 * - Default SCL Freq = 100 kHz
 * - Initializes the given SDA pins and SCL Pins as Alt0 w/ Pull-Up Resistors
 * - Set Timeout values to 0xFFF
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
    gpio_pull(scl_pin, PULL_UP);

    // Let the bus settle 
    wait_us(10);
    
    // set SCL Clock Divisor
    i2c_setClock(bus, DEF_CLK_MODE);

    // set Delay Register FEDL(CDIV / 16), REDL(CDIV/4)
    // uint16_t delVal = bus->div / 16;
    // bus->del = LSHIFT(delVal, 16) | (delVal);

    // set clock timeout value
    bus->clkt = 35 * DEF_CLK_MODE / 1000;

    // Clear any previous status
    i2c_reset(bus);

    wait_ms(10);

    // debugging information
    uart_printf("I2C Enabled & Initialized\n");
    uart_printf("- GFPSEL0:     %x\n", mmio_read(GPFSEL0));
    uart_printf("- PUP_PDN:     %x\n", mmio_read(GPPUPPDN0));
    uart_printf("- GPLEV0:      %x\n", mmio_read(GPLEV0));
    uart_printf("- CDIV:        %x\n", bus->div);
    uart_printf("- CTRL:        %x\n", bus->control);
    uart_printf("- DEL:         %x\n", bus->del);
}

/**
 * Sets the device address for the given bus.
 * 
 * @param bus           i2c bus base address
 * @param dev           device address
 * @param mode          i2c 7-bit or 10-bit addressing mode
 */
void i2c_setDevAddr(volatile i2c_reg_t* bus, uint16_t dev, i2c_mode mode) {
    // Set the device address using 10-bit address
    if (mode == I2C_10BIT) {
        // writes to the FIFO for the 8 LSB of the address
        bus->fifo = (dev & BIT_MASK(7,0));
        bus->addr = 0b1111000 | BITS(dev, 9, 8);
    } 
    // Set the device address using 7-bit address
    else if (mode == I2C_7BIT) {
        bus->addr = (uint8_t) dev;
    }
    // Unknown mode
    else {
        uart_printf("i2c_setDevAddr: Unknown mode = %d\n", (int) mode);
    }
}

/**
 * Resets the I2C bus by clearing the FIFO, clearing the DONE status, clearing the timeout status, &
 * clearing the acknowledgement error. 
 * 
 * @param bus           i2c bus base address
 */
void i2c_reset(volatile i2c_reg_t* bus) {
    // clear the done status
    bus->status = (S_DONE | S_CLKTOUT | S_ERR);

    // clear the fifo
    bus->control = (C_ENABLE | C_CLEAR);  
}

/**
 * Pings the device in the current bus by just sending the device address.
 * 
 * @param bus           i2c bus base address
 * @param dev           device address
 * @param mode          i2c 7-bit or 10-bit address modes
 * 
 * @return              An i2c status code
 */
i2c_status i2c_ping(volatile i2c_reg_t* bus, uint16_t dev, i2c_mode mode) {
    // reset the i2c bus done status
    i2c_reset(bus);
    wait_us(10);

    // set the number of data to send
    bus->dlen = (mode == I2C_10BIT) ? 1 : 0;

    // set the device address register
    i2c_setDevAddr(bus, dev, mode);
    
    // start the transfer
    bus->control = (C_ENABLE | C_START);

    // wait until done status
    uint32_t timeout = 10000;
    while (!(bus->status & S_DONE)) {
        if (bus->status & S_ERR) { return I2C_ACK_ERR; }
        if (--timeout == 0) { return I2C_CLK_TIMEOUT; }
        wait_us(1);
    }

    // Reset done status
    bus->status |= S_DONE;

    // Check if the i2c bus timed out
    if (bus->status & S_CLKTOUT) {
        return I2C_CLK_TIMEOUT;
    } else if (bus->status & S_ERR) {
        return I2C_ACK_ERR;
    }
    
    // Return success error code
    return I2C_SUCCESS;
}

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
void i2c_detect(volatile i2c_reg_t* bus, uint8_t first, uint8_t last) {
    // check first and last are valid addresses
    if (first < DEFAULT_BEGIN || first > 0x7F || last < DEFAULT_BEGIN || last > 0x7F) {
        uart_printf("i2c_detect: Invalid input\n");
        return;
    }

    // print the header (specific for now to first being less than 0x10);
    uart_printf(DETECT_FORMAT);
    uart_printf("0%c: ", HEX_STR(RSHIFT(first & 0xF0, 4)));
    if ((first & 0xF) > 0 && (first & 0xF) <= 0xF) {
        for(int i = 0; i < (first & 0xF); i++) {
            uart_printf("    ");

            if (i == 0xF) {
                uart_printf("\n");
            }
        }
    }

    // loop through the range of address through the i2c bus
    for(uint8_t dev = first; dev < last; dev++) {
        i2c_status res = i2c_ping(bus, dev, I2C_7BIT);

        if ((dev & 0xF) == 0) {
            uart_printf("0%c: ", HEX_STR(RSHIFT(dev, 4)));
        }

        // check i2c status
        if (res == I2C_ACK_ERR) {
            uart_printf("--");
        } else if (res == I2C_CLK_TIMEOUT) {
            uart_printf("UU");
        } else {
            uart_printf("%c%c", HEX_STR(RSHIFT(dev, 4)), HEX_STR(dev & 0xF));
        }
        uart_printf("  ");

        // Print new line
        if ((dev & 0xF) == 0xF) {
            uart_printf("\n");
        }

        // delay for 500 microseconds
        wait_us(500);
    }
    uart_printf("\n");
}

/**
 * Polling method for receiving data from the slave drive. Waits until the end of transmission.
 * 
 * @param bus           i2c bus base address
 * @param dev           device address to the slave drive
 * @param writeBuf      register address in the i2c device
 * @param nWrite        Number of bytes to write
 * @param readBuf       address to the buffer
 * @param nRead         umber of bytes to read per transfer
 * 
 * @return              An i2c status code
 */
i2c_status i2c_writeReadRepeat(volatile i2c_reg_t* bus, uint8_t dev, 
    const void* writeBuf, const uint32_t nWrite, void* readBuf, const uint32_t nRead) {
    // check for the parameter validity
    if (bus == NULL || dev >= MAX_I2C_DEV_ADDR || nRead == 0 || readBuf == NULL || writeBuf == NULL) {
        return I2C_INV_INPUT;
    }
    
    // Reset fifo and status
    i2c_reset(bus);

    // check if the i2c bus is enabled
    if (!(bus->control & C_ENABLE)) {
        return I2C_NOT_ENABLED;
    }

    // ----- Write internal register address to i2c device -----
    // set the slave address (7-bit addressing)
    // uart_printf("i2c_setDevAddr: Setting Device Address = %x\n", dev);
    i2c_setDevAddr(bus, dev, I2C_7BIT);

    // set the data length
    bus->dlen = nWrite;
    
    const uint8_t *pWriteBuf = (const uint8_t *) writeBuf;

    // put write buffer in the fifo
    // uart_printf("write_xfer: ");
    for (uint32_t i = 0; i < nWrite; i++) {
        bus->fifo = pWriteBuf[i];
        // uart_printf("%x ", pWriteBuf[i]);
    }
    // uart_printf("\n");

    // initiate write transfer
    uint64_t daif = critical_section_enter();
    bus->control = (C_ENABLE | C_START);
    
    // Wait till the xfer starts
    while(!(bus->status & S_TA));

    // ----- Read from the i2c device -----
    // Reset the status
    bus->status |= (S_DONE | S_ERR | S_CLKTOUT);

    // write number of data bytes to read to the dlen
    bus->dlen = nRead;

    // initiate read transfer (READ = 1, ST = 1)
    bus->control |= (C_ENABLE | C_START | C_READ);

    critical_section_exit(daif);
    wait_us(200);

    // track number of bytes read
    int count = 0;
    
    // set the read buffer pointer
    uint8_t *pReadBuf = (uint8_t *) readBuf;
    
    // read from the fifo until done
    while (!(bus->status & S_DONE)) {
        while ((bus->status & S_RXD) && count < nRead ) {
            // uint32_t val = mmio_read(BSC1_ADDR + I2C_FIFO_OFFSET);
            uint32_t val = bus->fifo;
            pReadBuf[count++] = (uint8_t) val;
            // uart_printf("   %x\n", val);
        }   
    }
    
    // Read any remainder data
    while ((bus->status & S_RXD) && count < nRead) {
        // uint32_t val = mmio_read(BSC1_ADDR + I2C_FIFO_OFFSET);
        uint32_t val = bus->fifo;
        pReadBuf[count++] = (uint8_t) val;
        // uart_printf("   %x\n", val);
    }

    // Reset the done status
    bus->status |= S_DONE;

    // Check for status
    if (bus->status & S_ERR) {
        bus->status = S_ERR; // clear error status
        return I2C_ACK_ERR;
    } else if (bus->status & S_CLKTOUT) {
        bus->status = S_CLKTOUT;
        return I2C_CLK_TIMEOUT;
    } else if (count < nRead) {
        return I2C_DATA_LOSS;
    } 
        
    return I2C_SUCCESS;
}

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
i2c_status i2c_blocking_send(volatile i2c_reg_t* bus, uint8_t dev, const uint32_t size, const void* buf) {
    // Check the input validity
    if (size == 0 || buf == NULL || bus == NULL) {
        return I2C_INV_INPUT;
    }

    // Reset the i2c bus
    i2c_reset(bus);

    // check if the i2c bus is enabled
    if (!(bus->control & C_ENABLE)) {
        return I2C_NOT_ENABLED;
    }

    // set slave address
    i2c_setDevAddr(bus, dev, I2C_7BIT);

    // set number of bytes to write
    bus->dlen = size;

    // convert buffer to uint8_t sized array
    const uint8_t* writeBuf = (const uint8_t *) buf;
    
    // check if the fifo can accept data or when there's no data left
    uint32_t count = 0;
    while ((bus->status & S_TXD) && count < size) {
        bus->fifo = writeBuf[count++];
    }
    
    uint64_t daif = critical_section_enter();   // critical section start
    // start write transfer
    bus->control |= (C_ENABLE | C_START);
    critical_section_exit(daif);                // critical section stop

    // wait until the Transfer started
    while (!(bus->status & S_TA));

    // check for active transfer and see if there's still data to write to
    while (!(bus->status & (S_DONE | S_CLKTOUT))) {
        // check if there's still data in the buffer
        if((bus->status & S_TXD) && count < size) {    
            bus->fifo = writeBuf[count++];
        }

        // Return the No Acknowledgement Error 
        if (bus->status & S_ERR) {
            // clear the fifo and stop writing into the FIFO.
            bus->control = C_CLEAR;
            uart_printf("i2c_blocking_send: ACK Error detected.\n");
            return I2C_ACK_ERR;
        }
    }

    if (bus->status & S_CLKTOUT) {
        return I2C_CLK_TIMEOUT;
    } else if (bus->status & S_ERR) {
        return I2C_ACK_ERR;
    } else {
        return I2C_SUCCESS;
    }
}


