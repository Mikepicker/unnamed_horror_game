#ifndef bmp_h
#define bmp_h

#include <stdio.h>

#define BYTES_PER_PIXEL 3 /// red, green, & blue

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

#endif
