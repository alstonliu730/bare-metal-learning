#include <gpio.h>
#include <mb.h>
#include <fb.h>
#include <font.h>
#include <common.h>
#include <uart.h>

uint32_t width, height, fb_pitch, isrgb, fb_size;
uint8_t *fb_addr;

/**
 * Initializes the Framebuffer using the Mailbox Property Channel.
 */
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

    if (mbox_call(MBOX_CH_PROP) && mbox[1] == MBOX_SUCCESS) {
        //uart_writeText("Frame Buffer Initialization Success\n");
        // Check individual tag responses
        if ((mbox[27] & MBOX_SUCCESS) && mbox[28] != 0) {
            fb_addr = (uint8_t *)((uintptr_t)(mbox[28] & 0x3FFFFFFF));
            fb_size = mbox[29];
            fb_pitch = mbox[33];
            width = mbox[5];
            height = mbox[6];
            isrgb = mbox[24];
        }
    } else {
        //uart_writeText("Frame Buffer Init Failed\n");
    }
}

/**
 * Draws a pixel with the color attribute from the rgb pallete.
 */
void drawPixel(uint32_t x, uint32_t y, uint8_t attr) {
    uint32_t offs = (y * fb_pitch) + (x * 4);
    *((unsigned int*)(fb_addr + offs)) = rgb_pal[attr & 0x0F];
}

/**
 * Draw a character at the given point with the color attribute.
 */
void drawChar(uint8_t ch, uint32_t x, uint32_t y, uint8_t attr)
{   
    int ch_id = (ch < FONT_NUMGLYPHS ? ch : 0);
    uint8_t *glyph = (uint8_t *)&font[ch_id];
    
    for (int i=0;i < FONT_HEIGHT; i++) {
        for (int j=0; j< FONT_WIDTH; j++) {
            uint8_t mask = 1 << j;
            uint8_t col = (*glyph & mask) ? attr & 0x0f : (attr & 0xf0) >> 4;

            drawPixel(x+j, y+i, col);
        }
        
        glyph += 1;
    }
}

/**
 * Draw a line of characters starting at the given point with the color attribute.
 */
void drawString(const char* str, uint32_t x, uint32_t y, uint8_t attr) {
    uint32_t cx = x;
    uint32_t cy = y;

    // traverse through string
    while(*str) {
        char ch = *str;

        // check for boundaries
        if (cx >= width || ch == '\r' || ch == '\n') {
            cy += FONT_HEIGHT;
            cx = 0; 
        }

        drawChar(ch, cx, cy, attr);
        cx += FONT_WIDTH;
        str++;
    }
}

/**
 * Draw a line from a source point to a destination point with the color attribute.
 */
void drawLine(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t attr) {
    int dx = x1 - x0;
    int dy = y1 - y0;

    int m_new = 2 * dy;
    int err_new = m_new - dx;
    
    for (uint32_t i = x0, j = y0; i <= x1; i++) {
        drawPixel(i, j, attr);

        err_new += m_new;

        if (err_new >= 0){
            j++;
            err_new -= 2 * dx;
        }
    }
}