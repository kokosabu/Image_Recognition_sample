#ifndef PNG_H
#define PNG_H

#include <stdio.h>
#include "bitmap.h"
#include "common.h"

struct tree {
    int len;
    int code;
};

typedef struct {
    uint8_t color_type;
    uint8_t bps;
    uint8_t interlace_type;
    uint8_t *alpha_index;
    uint16_t *alpha_gray;
    uint16_t *alpha_red;
    uint16_t *alpha_green;
    uint16_t *alpha_blue;
    uint8_t tRNS_size;
} PNG_INFO;

void decode_png(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data);

#endif
