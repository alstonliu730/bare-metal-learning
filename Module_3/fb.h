#ifndef FB_H
#define FB_H

void fb_init();
void drawPixel(int x, int y, unsigned char attr);
void drawChar(unsigned char ch, int x, int y, unsigned char attr);

#endif /* FB_H */