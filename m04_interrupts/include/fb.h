#ifndef FB_H
#define FB_H

void fb_init();
void drawPixel(int x, int y, unsigned char attr);
void drawChar(unsigned char ch, int x, int y, unsigned char attr);
void drawString(const char* str, int x, int y, unsigned char attr);
void drawLine(int x0, int y0, int x1, int y1, unsigned char attr);

#endif /* FB_H */