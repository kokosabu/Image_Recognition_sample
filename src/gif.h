#ifndef GIF_H
#define GIF_H

#include <stdio.h>
#include "bitmap.h"
#include "common.h"

void decode_gif(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data);
void init_table(int bit);
uint8_t *get_data(int index);
void compress(uint8_t *compress_data, int compress_data_size, uint8_t *original_data, int original_data_size, uint8_t *bit_lengths, int bit_lengths_size);
int decompress(uint8_t *compress_data, int compress_data_size, uint8_t *original_data, int original_data_size, int *first_flag);

enum {
    CLEAR = 0x01,
    END   = 0x02,
};

typedef struct {
    uint8_t transparent_color_flag;
    uint8_t transparent_color_index;
    uint16_t image_left_position;
    uint16_t image_top_position;
    uint16_t image_width;
    uint16_t image_height;
} GIF_INFO;

#endif /* GIF_H */
