#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "common.h"
#include "bitmap.h"
#include "average_filter.h"
#include "gaussian_filter.h"
#include "bilateral_filter.h"

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
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    IMAGEINFO image_info;
    RGBTRIPLE **image_data;
    RGBTRIPLE **output_image_data;
    int i;
    int j;
    uint8_t dummy;
    //uint16_t new_blue;
    //uint16_t new_green;
    //uint16_t new_red;
    //double new_blue_double_x;
    //double new_blue_double_y;
    //double new_green_double_x;
    //double new_green_double_y;
    //double new_red_double_x;
    //double new_red_double_y;
    //int k;
    //int l;
    //int w;
    //int h;
    //int kernel_size;
    //double **filter_x;
    //double **filter_y;
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
    bilateral_filter(&output_image_data, &image_data, &image_info, 25.0, 25.0, 7);
#if 0
    kernel_size = 3;
    filter_x = (double **)malloc(sizeof(double *)*kernel_size);
    filter_y = (double **)malloc(sizeof(double *)*kernel_size);
    for(k = 0; k < kernel_size; k++) {
        filter_x[k] = (double *)malloc(sizeof(double)*kernel_size);
        filter_y[k] = (double *)malloc(sizeof(double)*kernel_size);
    }
    for(k = 0; k < kernel_size; k++) {
        filter_x[k][0] = -1;
        filter_y[0][k] = -1;
        for(l = 1; l < kernel_size-1; l++) {
            filter_x[k][l] = 0;
            filter_y[l][k] = 0;
        }
        filter_x[k][kernel_size-1] = 1;
        filter_y[kernel_size-1][k] = 1;
    }
    for(i = 0; i < info_header.biHeight; i++) {
        for(j = 0; j < info_header.biWidth; j++) {
            new_blue_double_x = 0;
            new_blue_double_y = 0;
            new_green_double_x = 0;
            new_green_double_y = 0;
            new_red_double_x = 0;
            new_red_double_y = 0;
            for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
                for(l = -(kernel_size-1)/2; l <= (kernel_size-1)/2; l++) {
                    h = i + k;
                    if(h < 0) {
                        h = 0;
                    }
                    if(h > info_header.biHeight-1) {
                        h = info_header.biHeight - 1;
                    }
                    w = j + l;
                    if(w < 0) {
                        w = 0;
                    }
                    if(w > info_header.biWidth-1) {
                        w = info_header.biWidth - 1;
                    }

                    new_blue_double_x  += (image_data[h][w].rgbtBlue)  * filter_x[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_blue_double_y  += (image_data[h][w].rgbtBlue)  * filter_y[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_green_double_x += (image_data[h][w].rgbtGreen) * filter_x[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_green_double_y += (image_data[h][w].rgbtGreen) * filter_y[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_red_double_x   += (image_data[h][w].rgbtRed)   * filter_x[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_red_double_y   += (image_data[h][w].rgbtRed)   * filter_y[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                }
            }
            new_blue  = (uint16_t)sqrt(pow(new_blue_double_x,  2) + pow(new_blue_double_y,  2));
            new_green = (uint16_t)sqrt(pow(new_green_double_x, 2) + pow(new_green_double_y, 2));
            new_red   = (uint16_t)sqrt(pow(new_red_double_x,   2) + pow(new_red_double_y,   2));
            output_image_data[i][j].rgbtBlue = new_blue;
            output_image_data[i][j].rgbtGreen = new_green;
            output_image_data[i][j].rgbtRed = new_red;
        }
    }
#endif
#if 0
    kernel_size = 3;
    filter_x = (double **)malloc(sizeof(double *)*kernel_size);
    filter_y = (double **)malloc(sizeof(double *)*kernel_size);
    for(k = 0; k < kernel_size; k++) {
        filter_x[k] = (double *)malloc(sizeof(double)*kernel_size);
        filter_y[k] = (double *)malloc(sizeof(double)*kernel_size);
    }
    for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
        filter_x[k+(kernel_size-1)/2][0] = -1 * ((kernel_size-abs(k)) - 1);
        filter_y[0][k+(kernel_size-1)/2] = -1 * ((kernel_size-abs(k)) - 1);
        for(l = 1; l < kernel_size-1; l++) {
            filter_x[k+(kernel_size-1)/2][l] = 0;
            filter_y[l][k+(kernel_size-1)/2] = 0;
        }
        filter_x[k+(kernel_size-1)/2][kernel_size-1] = (kernel_size-abs(k)) - 1;
        filter_y[kernel_size-1][k+(kernel_size-1)/2] = (kernel_size-abs(k)) - 1;
    }
    for(i = 0; i < info_header.biHeight; i++) {
        for(j = 0; j < info_header.biWidth; j++) {
            new_blue_double_x = 0;
            new_blue_double_y = 0;
            new_green_double_x = 0;
            new_green_double_y = 0;
            new_red_double_x = 0;
            new_red_double_y = 0;
            for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
                for(l = -(kernel_size-1)/2; l <= (kernel_size-1)/2; l++) {
                    h = i + k;
                    if(h < 0) {
                        h = 0;
                    }
                    if(h > info_header.biHeight-1) {
                        h = info_header.biHeight - 1;
                    }
                    w = j + l;
                    if(w < 0) {
                        w = 0;
                    }
                    if(w > info_header.biWidth-1) {
                        w = info_header.biWidth - 1;
                    }

                    new_blue_double_x  += (image_data[h][w].rgbtBlue)  * filter_x[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_blue_double_y  += (image_data[h][w].rgbtBlue)  * filter_y[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_green_double_x += (image_data[h][w].rgbtGreen) * filter_x[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_green_double_y += (image_data[h][w].rgbtGreen) * filter_y[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_red_double_x   += (image_data[h][w].rgbtRed)   * filter_x[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_red_double_y   += (image_data[h][w].rgbtRed)   * filter_y[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                }
            }
            new_blue  = (uint16_t)sqrt(pow(new_blue_double_x,  2) + pow(new_blue_double_y,  2));
            new_green = (uint16_t)sqrt(pow(new_green_double_x, 2) + pow(new_green_double_y, 2));
            new_red   = (uint16_t)sqrt(pow(new_red_double_x,   2) + pow(new_red_double_y,   2));
            output_image_data[i][j].rgbtBlue = new_blue;
            output_image_data[i][j].rgbtGreen = new_green;
            output_image_data[i][j].rgbtRed = new_red;
        }
    }
#endif

    output = fopen("test", "wb");
    if(output == NULL) {
        return 0;
    }

    file_header.bfType      = ('M' << 8) | 'B';
    file_header.bfSize      = image_info.fileSize;
    file_header.bfReserved1 = 0;
    file_header.bfReserved2 = 0;
    file_header.bfOffBits   = 14 + 40;

    fwrite(&file_header.bfType, 2, 1, output);
    fwrite(&file_header.bfSize, 4, 1, output);
    fwrite(&file_header.bfReserved1, 2, 1, output);
    fwrite(&file_header.bfReserved2, 2, 1, output);
    fwrite(&file_header.bfOffBits, 4, 1, output);

    info_header.biSize          = 40;
    info_header.biWidth         = image_info.width;
    info_header.biHeight        = image_info.height;
    info_header.biPlanes        = 1;
    info_header.biBitCount      = 24;
    info_header.biCompression   = 0;
    info_header.biSizeImage     = image_info.width * image_info.height * 3;
    info_header.biXPelsPerMeter = 0;
    info_header.biYPelsPerMeter = 0;
    info_header.biClrUsed       = 0;
    info_header.biClrImportant  = 0;
    fwrite(&info_header.biSize, 4, 1, output);
    fwrite(&info_header.biWidth, 4, 1, output);
    fwrite(&info_header.biHeight, 4, 1, output);
    fwrite(&info_header.biPlanes, 2, 1, output);
    fwrite(&info_header.biBitCount, 2, 1, output);
    fwrite(&info_header.biCompression, 4, 1, output);
    fwrite(&info_header.biSizeImage, 4, 1, output);
    fwrite(&info_header.biXPelsPerMeter, 4, 1, output);
    fwrite(&info_header.biYPelsPerMeter, 4, 1, output);
    fwrite(&info_header.biClrUsed, 4, 1, output);
    fwrite(&info_header.biClrImportant, 4, 1, output);

    fseek(output, file_header.bfOffBits, SEEK_SET);

    for(i = info_header.biHeight-1; i >= 0; i--) {
        for(j = 0; j < info_header.biWidth; j++) {
            fwrite(&output_image_data[i][j].rgbtBlue, 1, 1, output);
            fwrite(&output_image_data[i][j].rgbtGreen, 1, 1, output);
            fwrite(&output_image_data[i][j].rgbtRed, 1, 1, output);
        }
        dummy = 0;
        fwrite(&dummy, 1, (3*info_header.biWidth)%4, output);
    }

    fclose(output);

    for(i = 0; i < image_info.height; i++) {
        free((void *)image_data[i]);
        free((void *)output_image_data[i]);
    }
    free((void *)image_data);
    free((void *)output_image_data);

    return 0;
}
