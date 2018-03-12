#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>

typedef struct tagIMAGEINFO {
    uint32_t fileSize;
    int32_t  width;
    int32_t  height;
} IMAGEINFO;

uint16_t read_2bytes(FILE *input);
uint32_t read_4bytes(FILE *input);
int bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos, int bit_len);
int huffman_bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos, int bit_len);
int image_bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos, int bit_len);

#endif
