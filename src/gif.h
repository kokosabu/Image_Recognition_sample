#ifndef GIF_H
#define GIF_H

#include <stdio.h>
#include "bitmap.h"
#include "common.h"

void decode_gif(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data);
void init_table(void);
uint8_t *get_data(int index);
void compress(uint16_t *compress_data, int compress_data_size, uint16_t *original_data, int original_data_size);

enum {
    CLEAR = 0x01,
    END   = 0x02,
};

#endif /* GIF_H */
