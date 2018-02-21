#ifndef PNG_H
#define PNG_H

#include <stdio.h>
#include "bitmap.h"
#include "common.h"

struct tree {
    int len;
    int code;
};

void decode_png(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data);

#endif
