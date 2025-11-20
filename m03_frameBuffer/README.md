# Frame Buffer
The frame buffer is the region of memory that stores the data for the image that will be displayed on the screen. It's a *contiguous* block of memory that contains pixel data. Each pixel in the frame buffer corresponds to a location on the screen with the memory setup so that its row by row. The frame buffer on the Raspberry Pi 4B can support **16-bit** (RGB565), **24-bit**(RGB888), and **32-bit**(ARGB8888) color formats.

## Mailbox Protocol
To setup the frame buffer, we need the ARM processor to communicate with the GPU to allocate a frame buffer. To communicate with the GPU, the Raspberry Pi uses something called a **Mailbox Protocol**. Here's a good [resource link](https://jsandler18.github.io/extra/mailbox.html) that explains this well.

### ARM Mailboxes

The ARM mailboxes helps with communication between the ARM Cortex-A72 cores. There's around 16 mailboxes in total with each core having 4 of their own. The mailbox is a **32-bit wide register** where each bit can be set or cleared independently. The memory structure of these mailboxes are set like this:
- 16-bit write-set registers (MBOX_SET00 to MBOX_SET15)
- 16-bit write-clear registers (MBOX_CLR00 to MBOX_CLR15)

*NOTE: The write-clear registers can be read to check current state*

Each core occupies 4 mailboxes:
- Mailbox 0-3 --> Core 0
- Mailbox 4-7 --> Core 1
- Mailbox 8-11 --> Core 2
- Mailbox 12-15 --> Core 3

**NOTE: The ARM Mailboxes located in the ARM_LOCAL block are distinct from the VPU Mailboxes located in the ARMC block.**
 
Mailbox bits can be set by writing the appropriate `MBOX_SET` register. It will generate an interrupt when any of the bits are *non-zero*

The Mailbox's value can be *read* from the appropriate `MBOX_CLR` register. It can be *cleared* by writing to the corresponding `MBOX_CLR` register. It has read/write capabilities. (These registers are mostly used in the ARM core's intterupt handler).

### ARM Addressing
The Base address for the ARM Mailboxes start at `0x4c0000000`. This is for ARM only address and not a legacy master address. The **Low-Peripheral Mode** address is `0xff800000`.

#### MBOX_SET(00-16) Registers
The offset starts at `0x80` to `0xbc` for all Mailboxes. This is the structure of each mailbox register. We know that there are 16 mailboxes and 0-3 corresponds to the first core and so on.
Each mailbox can raise an interrupt to its core when any of the bits in this 32-bit word are set to '1'. 

![MBOX_SET register information](assets/mbox_set_reg.png)

### General Procedure
To read from the mailbox:
1. Read the status register until the empty flag is not set
2. Read data from the read register
3. If the lower four bits do not match the channel number desired then repeat from 1
4. The upper 28-bits are the returned data

To write to the mailbox:
1. Read the status register until the full flag is not set
2. Write the data (shifted into the upper 28-bits)

### VC Mailbox
Since the VideoCore Mailbox implementations are close-sourced and are not well documented, we need to interpret the information that is given and work off of that. In their [mailbox documentation](https://github.com/raspberrypi/firmware/wiki/Mailboxes), they describe the channels that controls the communication between the ARM CPU and VideoCore GPU. They also provide us with the registers and a page on how to *access* the mailbox [here](https://github.com/raspberrypi/firmware/wiki/Accessing-mailboxes). Although not documented well, we can work off what the manufacturer provides. 

The registers in this mailbox protocol uses a different addressing mode. The VideoCore section in Physical RAM is mapped in from `0x0_4000_0000` downwards. The size of the SDRAM for VC peripherals is determined in the `config.txt` and if the L2 Cache is enabled. The **MMU (Memory Management Unit)** translates the virtual memory addresses to physical memory addresses in the RPi. We have not enabled the MMU and we didn't create our own. 

#### Mailbox Messages
The interface has **28-bits (MSB)** available for the data and **4-bits (LSB)** for the channel. This means when you send a request or receive the response the first 4-bits (LSB) will be the channel number and the rest will be the data value for an unsigned integer (~ 32-bits). 

#### Buffer Contents
In the buffer, we can include multiple tags to be processed in one operations. Typically, the tags are processed in order but for interfaces that requires multiple tags for a single operation like the frame buffer. The structure of the buffer should look like this:

```C
typedef struct {
    unsigned int buffer_size;   // Size of the entire buffer
                                // (include header values, end tag, and padding)
    unsigned int buffer_code;   // request/response code
    void *tags;                 // concatenated tags
    unsigned int end_tag = 0x0; // end tag of concatenated tags
    void *padding;              // to align the struct
} mbox_buffer
```

#### Tag Format
In the buffer, each concatenated tag will have a tag id, tag size, request/response code, the buffer, & padding to align the tag to 32 bits. This is what each tag should look like:

```C
typedef struct {
    unsigned int tag_id;    // tag identifier (ex. 0x00000001 for firmware revision)
    unsigned int tag_size;  // size of the tag includes 
                            // the id, code, buffer and padding in bytes
    unsigned int tag_code;  // request/response code
    void *tag_buf;          // values to input or output
    void *padding;          // to align the tag to 32-bits
} mbox_tag
```

**Note: These are not the implementation but merely a template of what it may look like.**
To have the compiler align the buffer for you, use `__attribute__` and find the **aligned** macro.

### Frame Buffer tags
In the property tag channel, we can group the setter and getter tags in one command with the exception of the frame buffer channel. We can group together the tags in one buffer for the VideoCore GPU to execute in one command. To do this, we set the size of the encapsulating tag that will contain a buffer of concatednated tags. This would include the first encapsulating tag, and the buffer, and the end tag.

## Result
As we initialize the framebuffer, there was no output to the mini uart terminal. We usually see a `"UART Initialized"` message but nothing shows up. We diagnosed that the configuration file was not working. We had initially used the default configuration file from the Raspberry Pi Image Tool. We changed it so that the essential firmware can be used. As we fix the UART, we can see that the framebuffer initialization failed. 

The buffer returns the following when it fails. I created a debugging feature when it initializes to see what it returns.
```
UART Initialized
Frame Buffer Init Failed
Buffer 
0: 0x0000008C
1: 0x80000001
2: 0x00048003
3: 0x00000008
4: 0x80000008
5: 0x00000000
6: 0x00000000
7: 0x00048004
8: 0x00000008
9: 0x80000008
10: 0x00000000
11: 0x00000000
12: 0x00048009
13: 0x00000008
14: 0x80000008
15: 0x00000000
16: 0x00000000
17: 0x00048005
18: 0x00000004
19: 0x80000004
20: 0x00000000
21: 0x00048006
22: 0x00000004
23: 0x80000004
24: 0x00000000
25: 0x00040001
26: 0x00000008
27: 0x80000008
28: 0x00000000
29: 0x00000000
30: 0x00040008
31: 0x00000004
32: 0x80000004
33: 0x00000000
34: 0x00000000
35: 0x03000000
```
We see that the value at 1 returns an error code (0x80000001) from the mailbox. I also noticed none of the values are being returned to the array. We fixed the initialization function by sending it to the property channel instead of the framebuffer channel that is deprecated from the VideoCore Documentation. In the following text, we see that the message received by the mailbox came back successful. It contains the returned values and the framebuffer address:
```
Frame Buffer Initialization Success
Buffer: 
0: 0x0000008C
 1: 0x80000000
 2: 0x00048003
 3: 0x00000008
 4: 0x80000008
 5: 0x00000780
 6: 0x00000438
 7: 0x00048004
 8: 0x00000008
 9: 0x80000008
 10: 0x00000780
 11: 0x00000438
 12: 0x00048009
 13: 0x00000008
 14: 0x80000008
 15: 0x00000000
 16: 0x00000000
 17: 0x00048005
 18: 0x00000004
 19: 0x80000004
 20: 0x00000020
 21: 0x00048006
 22: 0x00000004
 23: 0x80000004
 24: 0x00000001
 25: 0x00040001
 26: 0x00000008
 27: 0x80000008
 28: 0xFE402000
 29: 0x007F8000
 30: 0x00040008
 31: 0x00000004
 32: 0x80000004
 33: 0x00001E00
 34: 0x00000000
 35: 0x00000000
Frame Buffer Initialized
```

This time we can see the size of the screen that was allocated in the array position 5, 6, 10, & 11. We can also see the allocation for the framebuffer at position 28 and 29 that returns the address and size of the buffer. We added a few functions to be able to print and draw shapes on the screen. This way we know that the display out is working and therefore for future development, we can create a terminal that allows the users to input commands. This is what it looks like when we tested the framebuffer.

![Resulting Image of the Frame Buffer Test](assets/result.jpeg)
![Close up of string text on display](assets/hello_world_test.jpeg)

