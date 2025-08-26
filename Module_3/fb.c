#include "io.h"
#include "mb.h"
#include "fb.h"

unsigned int width, height, fb_pitch, isrgb;
unsigned char *fb_addr;

void fb_init() {
    // Requesting mailbox to process multiple commands
    mbox[0] = 35 * 4; // length of message in bytes
    mbox[1] = MBOX_REQUEST; 
    
    // virtual dimensions
    mbox[2] = MBOX_TAG_PHYS_DIM;    // Tag ID
    mbox[3] = 8;                    // Tag size in bytes
    mbox[4] = MBOX_REQUEST;         // Request Code
    mbox[5] = 1920;                 // width
    mbox[6] = 1080;                 // height

    mbox[7] = MBOX_TAG_VIRT_DIM;    // Virutal dimensions
    mbox[8] = 8;
    mbox[9] = MBOX_REQUEST;
    mbox[10] = 1920;
    mbox[11] = 1080;

    mbox[12] = MBOX_TAG_VIRT_OFFSET; 
    mbox[13] = 8;
    mbox[14] = MBOX_REQUEST;
    mbox[15] = 0;
    mbox[16] = 0;

    mbox[17] = MBOX_TAG_DEPTH; // bytes per pixel
    mbox[18] = 4;
    mbox[19] = MBOX_REQUEST;
    mbox[20] = 32;

    mbox[21] = MBOX_TAG_PIXEL_ORDER;
    mbox[22] = 4;
    mbox[23] = MBOX_REQUEST;
    mbox[24] = 0x1; // RGB

    mbox[25] = MBOX_TAG_GETFB;
    mbox[26] = 4;
    mbox[27] = MBOX_REQUEST;
    mbox[28] = 16; // 16-byte aligned
    mbox[29] = 0;  // Will be filled with the framebuffer address by the GPU

    mbox[30] = MBOX_TAG_GETPITCH;
    mbox[31] = 4;
    mbox[32] = MBOX_REQUEST;
    mbox[33] = 0; // Will be filled with the pitch by the GPU

    mbox[34] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_FB) && mbox[20] == 32 && mbox[28] != 0 && mbox[33] != 0) {
        fb_addr = (unsigned char *)(unsigned long)(mbox[28] & 0x3FFFFFFF); // Convert GPU address to ARM address
        fb_pitch = mbox[33];
        width = mbox[5];
        height = mbox[6];
        isrgb = mbox[24];
    }
}


