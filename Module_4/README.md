# Frame Buffer
The frame buffer is the region of memory that stores the data for the image that will be displayed on the screen. It's a *contiguous* block of memory that contains pixel data. Each pixel in the frame buffer corresponds to a location on the screen with the memory setup so that its row by row. The frame buffer on the Raspberry Pi 4B can support **16-bit** (RGB565), **24-bit**(RGB888), and **32-bit**(ARGB8888) color formats.

## Mailbox Protocol
To setup the frame buffer, we need the ARM processor to communicate with the GPU to allocate a frame buffer. To communicate with the GPU, the Raspberry Pi uses something called a **Mailbox Protocol**. Here's a good [resource link](https://jsandler18.github.io/extra/mailbox.html) that explains this well.

The ARM mailboxes helps with communication between the ARM Cortex-A72 cores. There's around 16 mailboxes in total with each core having 4 of their own. The mailbox is a **32-bit wide register** where each bit can be set or cleared independently. The memory structure of these mailboxes are set like this:
- 16-bit write-set registers (MBOX_SET00 to MBOX_SET15)
- 16-bit write-clear registers (MBOX_CLR00 to MBOX_CLR15)

*NOTE: The write-clear registers can be read to check current state*

Each core occupies 4 mailboxes:
- Mailbox 0-3 --> Core 0
- Mailbox 4-7 --> Core 1
- Mailbox 8-11 --> Core 2
- Mailbox 12-15 --> Core 3

### General Procedure

To read from the mailbox:
1. Read the status register until the empty flag is not set

### VPU Mailbox