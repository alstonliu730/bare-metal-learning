#ifndef MB_H
#define MB_H

#include "io.h"

extern volatile unsigned int mbox[36];

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

#define MBOX_TAG_PHYS_DIM    0x00048003
#define MBOX_TAG_VIRT_DIM    0x00048004


unsigned int mbox_call(unsigned char ch);

#endif /* MB_H */