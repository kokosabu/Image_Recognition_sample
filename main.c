#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "bitmap.h"
#include "average_filter.h"
#include "gaussian_filter.h"
#include "bilateral_filter.h"
#include "prewitt_filter.h"
#include "sobel_filter.h"
#include "canny_edge_detector.h"

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

int main(int argc, char *argv[])
{
    FILE *input;
    FILE *output;
    IMAGEINFO image_info;
    RGBTRIPLE **image_data;
    RGBTRIPLE **output_image_data;
    int i;
    int file_format;
    uint8_t *png_image_data;

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
        uint8_t byte;
        uint32_t size;
        uint32_t width;
        uint32_t height;
        uint8_t bps;
        uint8_t color_type;
        uint8_t compress_type;
        uint8_t filter_type;
        uint8_t interlace_type;
        uint32_t crc_32;
        char chunk[5];
        printf("PNG\n");

        for(i = 0; i < 8; i++) {
            fread(&byte, 1, 1, input);
        }

        size = read_4bytes(input);
        fread(&chunk[0], 1, 1, input);
        fread(&chunk[1], 1, 1, input);
        fread(&chunk[2], 1, 1, input);
        fread(&chunk[3], 1, 1, input);
        chunk[4] = '\0';
        width = read_4bytes(input);
        height = read_4bytes(input);
        fread(&bps, 1, 1, input);
        fread(&color_type, 1, 1, input);
        fread(&compress_type, 1, 1, input);
        fread(&filter_type, 1, 1, input);
        fread(&interlace_type, 1, 1, input);
        crc_32 = read_4bytes(input);

        printf("size:%d\n", size);
        printf("chunk:%s\n", chunk);
        printf("width:%d\n", width);
        printf("height:%d\n", height);
        printf("bps:%d\n", bps);
        printf("color type:%d\n", color_type);
        printf("compress type:%d\n", compress_type);
        printf("filter type:%d\n", filter_type);
        printf("interlace type:%d\n", interlace_type);
        printf("crc-32: %xh\n", crc_32);

        size = read_4bytes(input);
        fread(&chunk[0], 1, 1, input);
        fread(&chunk[1], 1, 1, input);
        fread(&chunk[2], 1, 1, input);
        fread(&chunk[3], 1, 1, input);
        chunk[4] = '\0';
        printf("size:%d\n", size);
        printf("chunk:%s\n", chunk);
        if(strcmp(chunk, "IDAT") == 0) {
            png_image_data = (uint8_t *)malloc(sizeof(uint8_t) * size);
            fread(png_image_data, 1, size, input);
            crc_32 = read_4bytes(input);
        } else {
            return 0;
        }

        size = read_4bytes(input);
        fread(&chunk[0], 1, 1, input);
        fread(&chunk[1], 1, 1, input);
        fread(&chunk[2], 1, 1, input);
        fread(&chunk[3], 1, 1, input);
        chunk[4] = '\0';
        printf("size:%d\n", size);
        printf("chunk:%s\n", chunk);
        if(strcmp(chunk, "IEND") == 0) {
            crc_32 = read_4bytes(input);
        } else {
            return 0;
        }

        printf("%02x\n%02x\n", png_image_data[0], png_image_data[1]);
        printf("%d : %d : %d\n", (png_image_data[1] & 0xC0)>>6, (png_image_data[1]&0x20)>>5, png_image_data[1]&0x1F);

        return 0;
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
    canny_edge_detector(&output_image_data, &image_data, &image_info, 0.8, 7, 3);

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
