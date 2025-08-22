#include "io.h"
#include "mb.h"
#include "fb.h"

void fb_init() {
    // Requesting mailbox to process multiple commands
    mbox[0] = 35 * 4; // length of message in bytes
    mbox[1] = MBOX_REQUEST; 
    
    mbox[2] = MBOX_TAG_PHYS_DIM;    // Tag ID
    mbox[3] = 8;                    // Tag size in bytes
    mbox[4] = MBOX_REQUEST;         // Request Code
    mbox[5] = 1920;                 // width
    mbox[6] = 1080;                 // height

    mbox[7] = MBOX_TAG_VIRT_DIM;
    mbox[8] = 8;
    mbox[9] = MBOX_REQUEST;
    mbox[10] = 1920;
    mbox[11] = 1080;

    mbox[12] = MBOX_TAG_VIRT_OFFSET;
    mbox[13] = 8;
    mbox[14] = MBOX_REQUEST;
    mbox[15] = 0;
    mbox[16] = 0;

    mbox[17] = MBOX_TAG_DEPTH;
    mbox[18] = 4;
    mbox[19] = MBOX_REQUEST;
    mbox[20] = 32;
}
