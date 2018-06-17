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
    uint8_t compress_type;
    uint8_t interlace_type;
    uint8_t *alpha_index;
    uint16_t *alpha_gray;
    uint16_t *alpha_red;
    uint16_t *alpha_green;
    uint16_t *alpha_blue;
    uint8_t tRNS_size;
    uint8_t filter_type;
    uint32_t gamma;
    uint32_t width;
    uint32_t height;
    uint32_t idat_size;
    uint32_t palette_size;

    RGBTRIPLE *color_palette;
    uint8_t flag;
} PNG_INFO;

void decode_png(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data);

void decompress_fixed_huffman_codes(uint8_t *png_image_data, int *byte_index, int *bit_index, int *lit, int *dist, struct tree *tree, struct tree *dtree);

#endif
