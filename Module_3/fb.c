#include "io.h"
#include "mb.h"
#include "fb.h"
#include "font.h"
#include "common.h"

unsigned int width, height, fb_pitch, isrgb, fb_size;
unsigned char *fb_addr;

#define HEX_STR(h) ((h < 10) ? '0' + h : 'A' + h - 10)

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
    mbox[26] = 8;
    mbox[27] = MBOX_REQUEST;
    mbox[28] = 4096; // Will be filled with the framebuffer address by the GPU
    mbox[29] = 0; // Framebuffer size

    mbox[30] = MBOX_TAG_GETPITCH;
    mbox[31] = 4;
    mbox[32] = MBOX_REQUEST;
    mbox[33] = 0; // Will be filled with the pitch by the GPU

    mbox[34] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP) && mbox[1] == 0x80000000) {
        uart_writeText("Frame Buffer Initialization Success\n");
        // Check individual tag responses
        if ((mbox[27] & 0x80000000) && mbox[28] != 0) {
            fb_addr = (unsigned char *)((uintptr_t)(mbox[28] & 0x3FFFFFFF));
            fb_size = mbox[29];
            fb_pitch = mbox[33];
            width = mbox[5];
            height = mbox[6];
            isrgb = mbox[24];
        }
    } else {
        uart_writeText("Frame Buffer Init Failed\n");
    }

    // debugging
    uart_writeText("Buffer: \n");
    for(int i = 0; i < 36; i++) {
        unsigned int val = mbox[i];

        uart_writeInt(i, sizeof(int));
        uart_writeText(": 0x");
        delay(100);
        for (int k = 28; k >= 0; k -= 4) {
            uart_writeByte(HEX_STR(((val >> k) & 0xF)));
        }
        uart_writeText("\n");
    }
}

void drawPixel(int x, int y, unsigned char attr) {
    int offs = (y * fb_pitch) + (x * 4);
    *((unsigned int*)(fb_addr + offs)) = rgb_pal[attr & 0x0F];
}

void drawChar(unsigned char ch, int x, int y, unsigned char attr)
{   
    int ch_id = (ch < FONT_NUMGLYPHS ? ch : 0);
    unsigned char *glyph = (unsigned char *)&font[ch_id];
    
    for (int i=0;i < FONT_HEIGHT; i++) {
        unsigned int f_line = *glyph & 0xFF;
        uart_writeText("Glyph: ");
        uart_writeInt(f_line, 4);

        for (int j=0; j< FONT_WIDTH; j++) {
            unsigned char mask = 1 << j;
            unsigned char col = (*glyph & mask) ? attr & 0x0f : (attr & 0xf0) >> 4;

            drawPixel(x+j, y+i, col);
        }
        
        glyph += 1;
        uart_writeText("\n");
    }
}