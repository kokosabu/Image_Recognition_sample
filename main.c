#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "bitmap.h"
#include "average_filter.h"
#include "gaussian_filter.h"
#include "bilateral_filter.h"
#include "prewitt_filter.h"
#include "sobel_filter.h"
#include "canny_edge_detector.h"
#include "LoG_filter.h"
#include "png.h"

enum {
    NONE,
    BITMAP,
    PNG,
}; 

int check_file_format(FILE *input)
{
    uint8_t read_byte;

    fseek(input, 0, SEEK_SET);

    fread(&read_byte, 1, 1, input);
    if(read_byte == 'B') {
        fread(&read_byte, 1, 1, input);
        if(read_byte == 'M') {
            fseek(input, 0, SEEK_SET);
            return BITMAP;
        } else {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
    } else if(read_byte == 0x89) {
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'P') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'N') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'G') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != '\r') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != '\n') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != 0x1a) {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != '\n') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fseek(input, 0, SEEK_SET);
        return PNG;
    }

    fseek(input, 0, SEEK_SET);
    return NONE;
}

uint16_t read_2bytes(FILE *input)
{
    uint8_t byte;
    uint16_t result;

    fread(&byte, 1, 1, input);
    result = byte;
    fread(&byte, 1, 1, input);
    result = (result << 8) | byte;

    return result;
}

uint32_t read_4bytes(FILE *input)
{
    uint8_t byte;
    uint32_t result;

    fread(&byte, 1, 1, input);
    result = byte;
    fread(&byte, 1, 1, input);
    result = (result << 8) | byte;
    fread(&byte, 1, 1, input);
    result = (result << 8) | byte;
    fread(&byte, 1, 1, input);
    result = (result << 8) | byte;

    return result;
}

int bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos, int bit_len)
{
    uint8_t pattern[8] = {
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
    };
    uint8_t byte;
    uint8_t next_byte;

    byte      = input_stream[*byte_pos];
    next_byte = input_stream[(*byte_pos)+1];

    if((8-(*bit_pos)) >= bit_len) {
        byte >>= *bit_pos;
        byte &= pattern[bit_len-1];
    } else {
        byte >>= *bit_pos;
        byte &= pattern[(8-(*bit_pos))-1];
        next_byte &= pattern[(*bit_pos) - (8-bit_len) - 1];
        byte = byte | ( next_byte << (8-(*bit_pos)) );
    }

    *bit_pos += bit_len;
    if(*bit_pos >= 8) {
        *byte_pos += (*bit_pos) / 8;
        *bit_pos %= 8;
    }

    return byte;
}

int huffman_bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos, int bit_len)
{
    uint8_t pattern[8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
            //0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
    };
    uint8_t byte;

    byte = input_stream[*byte_pos];
    byte &= pattern[*bit_pos];
    byte >>= *bit_pos;

    *bit_pos += bit_len;
    if(*bit_pos >= 8) {
        *byte_pos += (*bit_pos) / 8;
        *bit_pos %= 8;
    }

    return byte;
}

int main(int argc, char *argv[])
{
    FILE *input;
    FILE *output;
    IMAGEINFO image_info;
    RGBTRIPLE **image_data;
    RGBTRIPLE **output_image_data;
    int i;
    int file_format;
    uint8_t *output_stream;

    if(argc <= 1) {
        printf("filename\n");
        return 0;
    }

    input = fopen(argv[1], "rb");
    if(input == NULL) {
        printf("don't open file\n");
        return 0;
    }

    file_format = check_file_format(input);
    if(file_format == NONE) {
        printf("unsupported file format\n");
        return 0;
    } else if(file_format == BITMAP) {
        decode_bitmap(input, &image_info, &image_data);
        printf("%d %d %d\n", image_info.fileSize, image_info.width, image_info.height);
    } else if(file_format == PNG) {
        decode_png(input, &image_info, &image_data);
        printf("%d %d %d\n", image_info.fileSize, image_info.width, image_info.height);
    }
    fclose(input);


    output_image_data = (RGBTRIPLE **)malloc(sizeof(RGBTRIPLE *) * image_info.height);
    for(i = image_info.height-1; i >= 0; i--) {
        output_image_data[i] = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * image_info.width);
    }

    //average_filter(&output_image_data, &image_data, &image_info, 11);
    //gaussian_filter(&output_image_data, &image_data, &image_info, 3.0, 7);
    //bilateral_filter(&output_image_data, &image_data, &image_info, 25.0, 25.0, 7);
    //prewitt_filter(&output_image_data, &image_data, &image_info, 3);
    //sobel_filter(&output_image_data, &image_data, &image_info, 3);
    //canny_edge_detector(&output_image_data, &image_data, &image_info, 0.8, 7, 3);
    //LoG_filter(&output_image_data, &image_data, &image_info, 1.4, 9);

    for(i = 0; i < image_info.height; i++) {
        for(int j = 0; j < image_info.width; j++) {
            output_image_data[i][j] = image_data[i][j];
        }
    }

    output = fopen("test", "wb");
    if(output == NULL) {
        return 0;
    }
    encode_bitmap(output, &image_info, &output_image_data);
    fclose(output);

    for(i = 0; i < image_info.height; i++) {
        free((void *)image_data[i]);
        free((void *)output_image_data[i]);
    }
    free((void *)image_data);
    free((void *)output_image_data);

    return 0;
}
