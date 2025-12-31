#ifndef _FB_H
#define _FB_H

#include <common.h>

void fb_init();
void drawPixel(uint32_t x, uint32_t y, uint8_t attr);
void drawChar(uint8_t ch, uint32_t x, uint32_t y, uint8_t attr);
void drawString(const char* str, uint32_t x, uint32_t y, uint8_t attr);
void drawLine(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t attr);

#endif /* _FB_H */