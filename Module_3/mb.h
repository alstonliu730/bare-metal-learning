#ifndef MB_H
#define MB_H

#include "io.h"

extern volatile unsigned int mbox[36];

#define MBOX_REQUEST    0x00000000
#define MBOX_TAG_LAST   0x00000000

// Mailbox Addresses
#define VIDEOCORE_MBOX  (PERIPHERAL_BASE + 0x0000B880)
#define MBOX_READ       (VIDEOCORE_MBOX + 0x00)
#define MBOX_POLL       (VIDEOCORE_MBOX + 0x10)
#define MBOX_SENDER     (VIDEOCORE_MBOX + 0x14)
#define MBOX_STATUS     (VIDEOCORE_MBOX + 0x18)  
#define MBOX_CONFIG     (VIDEOCORE_MBOX + 0x1C)
#define MBOX_WRITE      (VIDEOCORE_MBOX + 0x20)
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

// Mailbox Tag IDs
#define MBOX_TAG_SETPWR      0x00028001
#define MBOX_TAG_SETCLK      0x00038002

// Set framebuffer tags
#define MBOX_TAG_PHYS_DIM    0x00048003
#define MBOX_TAG_VIRT_DIM    0x00048004
#define MBOX_TAG_VIRT_OFFSET 0x00048009
#define MBOX_TAG_DEPTH       0x00048005
#define MBOX_TAG_PIXEL_ORDER 0x00048006

// Get framebuffer tags
#define MBOX_TAG_GETFB       0x00040001
#define MBOX_TAG_GETPITCH    0x00040008

// Mailbox Channels
#define MBOX_CH_POWER        0   
#define MBOX_CH_FB           1
#define MBOX_CH_VUART        2
#define MBOX_CH_VCHIQ        3
#define MBOX_CH_LEDS         4
#define MBOX_CH_BTNS         5
#define MBOX_CH_TOUCH        6
#define MBOX_CH_COUNT        7
#define MBOX_CH_PROP         8

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

#endif /* MB_H */