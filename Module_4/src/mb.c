#include "../include/mb.h"
#include "../include/io.h"

// The buffer must be 16-byte aligned as only the upper 28 bits of the address can be passed via the mailbox
volatile unsigned int __attribute__((aligned(16))) mbox[36];

#define MBOX_STATUS_FULL mmio_read(MBOX_STATUS) & MBOX_FULL
#define MBOX_STATUS_EMPTY mmio_read(MBOX_STATUS) & MBOX_EMPTY

/**
 * Sends the 16-byte aligned array to the mailbox for the framebuffer.
 */
unsigned int mbox_call(unsigned char ch) {
    // Get the 28-bit (MSB) aligned address of the mailbox buffer (MSB)
    unsigned int msb = (unsigned int)(((long) &mbox) & ~0xF);

    // Get the channel number (LSB of the address)
    unsigned int lsb_ch = (unsigned int)(ch & 0xF);
    
    // Get full mailbox address MSB and LSB
    unsigned int r = msb | lsb_ch;

    while (MBOX_STATUS_FULL) {
        // Wait until the mailbox is not full
        uart_writeText("MBOX FULL\n");
    }

    // Write the address to the mailbox
    mmio_write(MBOX_WRITE, r);

    while (1) {
        // Wait for a response
        while (MBOX_STATUS_EMPTY) {
            // Wait until the mailbox is not empty
        }

        if (r == mmio_read(MBOX_READ)) return (mbox[1] == MBOX_RESPONSE);
    }
    
    return 0; // Should never reach here
}

/**
 * Reads the mailbox data from the given channel.
 */
unsigned int mbox_read(unsigned char ch) {
    while (MBOX_STATUS_EMPTY) {
        // Wait until the mailbox is not empty
    }

    unsigned int data = mmio_read(MBOX_READ);

    if((data & 0xF) == ch) {
        return (data >>= 4);
    }

    return 0;
}

/**
 * Writes to the mailbox given the channel.
 */
void mbox_write(unsigned char ch, unsigned int data) {
    unsigned int addr = (data << 4) | (ch & 0xF);

    while (MBOX_STATUS_FULL) {
        // Wait until the mailbox is empty
    }

    mmio_write(MBOX_WRITE, addr);
    return;
}

