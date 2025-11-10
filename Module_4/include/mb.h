#ifndef MB_H
#define MB_H

#include "common.h"

extern volatile unsigned int __attribute__((aligned(16))) mbox[36];

#define MBOX_REQUEST            0x00000000
#define MBOX_TAG_LAST           0x00000000
#define MBOX_SUCCESS            0x80000000
#define MBOX_ERROR              0x80000001

// Mailbox Addresses
#define VIDEOCORE_MBOX          (PERIPHERAL_BASE + 0x0000B880)
#define MBOX_READ               (VIDEOCORE_MBOX + 0x00)
#define MBOX_POLL               (VIDEOCORE_MBOX + 0x10)
#define MBOX_SENDER             (VIDEOCORE_MBOX + 0x14)
#define MBOX_STATUS             (VIDEOCORE_MBOX + 0x18)  
#define MBOX_CONFIG             (VIDEOCORE_MBOX + 0x1C)
#define MBOX_WRITE              (VIDEOCORE_MBOX + 0x20)
#define MBOX_RESPONSE           0x80000000
#define MBOX_FULL               0x80000000
#define MBOX_EMPTY              0x40000000

// Mailbox Tag IDs
#define MBOX_TAG_SETPWR         0x00028001
#define MBOX_TAG_SETCLK         0x00038002
#define MBOX_TAG_GETCLK         0x00030002

// Set framebuffer tags
#define MBOX_TAG_PHYS_DIM       0x00048003
#define MBOX_TAG_VIRT_DIM       0x00048004
#define MBOX_TAG_VIRT_OFFSET    0x00048009
#define MBOX_TAG_DEPTH          0x00048005
#define MBOX_TAG_PIXEL_ORDER    0x00048006

// Get framebuffer tags
#define MBOX_TAG_GETFB          0x00040001
#define MBOX_TAG_GETPITCH       0x00040008

// Mailbox Channels
#define MBOX_CH_POWER           0   
#define MBOX_CH_FB              1
#define MBOX_CH_VUART           2
#define MBOX_CH_VCHIQ           3
#define MBOX_CH_LEDS            4
#define MBOX_CH_BTNS            5
#define MBOX_CH_TOUCH           6
#define MBOX_CH_COUNT           7
#define MBOX_CH_PROP            8

// Mailbox Clock IDs
#define MBOX_CLK_EMMC           0x000000001
#define MBOX_CLK_UART           0x000000002
#define MBOX_CLK_ARM            0x000000003
#define MBOX_CLK_CORE           0x000000004
#define MBOX_CLK_V3D            0x000000005
#define MBOX_CLK_H264           0x000000006
#define MBOX_CLK_ISP            0x000000007
#define MBOX_CLK_SDRAM          0x000000008
#define MBOX_CLK_PIXEL          0x000000009
#define MBOX_CLK_PWM            0x00000000A
#define MBOX_CLK_HEVC           0x00000000B
#define MBOX_CLK_EMMC2          0x00000000C
#define MBOX_CLK_M2MC           0x00000000D
#define MBOX_CLK_PIXEL_BVB      0x00000000E

// Mailbox structs
typedef struct {
    unsigned int id;
    unsigned int buffer_size;
    unsigned int value_length;
} mbox_tag;

typedef struct {
    unsigned int size;
    void *tag;
} mbox_cmd;

typedef struct {
    mbox_tag tag;
    unsigned int id;
    unsigned int value;
} mbox_gen;

typedef struct {
    mbox_tag tag;
    unsigned int id;
    unsigned int state;
} mbox_power;

typedef struct {
    mbox_tag tag;
    unsigned int id;
    unsigned int rate;
} mbox_clock;

unsigned int mbox_call(unsigned char ch);

void debug_mbox(int nSize);

#endif /* MB_H */