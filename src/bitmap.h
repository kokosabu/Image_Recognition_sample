#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>
#include <stdio.h>
#include "common.h"

typedef struct tagBITMAPFILEHEADER {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  uint32_t biSize;
  int32_t  biWidth;
  int32_t  biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t  biXPelsPerMeter;
  int32_t  biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBTRIPLE { 
  uint16_t rgbtBlue; 
  uint16_t rgbtGreen; 
  uint16_t rgbtRed;

  uint16_t rgbtAlpha; 
} RGBTRIPLE;

typedef struct tagRGBQUAD {
  uint16_t rgbBlue;
  uint16_t rgbGreen;
  uint16_t rgbRed;
  uint16_t rgbReserved;
} RGBQUAD;

void decode_bitmap(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data);
void encode_bitmap(FILE *output, IMAGEINFO *image_info, RGBTRIPLE ***image_data);

#endif
