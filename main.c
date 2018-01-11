#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "bitmap.h"


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
    RGBTRIPLE **image_data;
    RGBTRIPLE **output_image_data;
    RGBTRIPLE *color_pallet_rgb;
    int i;
    int j;
    uint8_t dummy;
    uint16_t new_blue;
    uint16_t new_green;
    uint16_t new_red;
    double new_blue_double;
    double new_green_double;
    double new_red_double;
    double new_blue_double_d;
    double new_green_double_d;
    double new_red_double_d;
    int k;
    int l;
    int w;
    int h;
    int kernel_size;
    double sigma;
    double **filter;
    double filter_sum;
    double filter_sum2;
    double sigma_r;
    double sigma_d;
    uint8_t data8;
    uint16_t data16;
    uint32_t data32;
    int file_format;

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
        decode_bitmap(input, &file_header, &info_header, &image_data);
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




        return 0;
    }
    fclose(input);

    output_image_data = (RGBTRIPLE **)malloc(sizeof(RGBTRIPLE *) * info_header.biHeight);
    for(i = info_header.biHeight-1; i >= 0; i--) {
        output_image_data[i] = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * info_header.biWidth);
    }

    kernel_size = 11;
    for(i = 0; i < info_header.biHeight; i++) {
        for(j = 0; j < info_header.biWidth; j++) {
            new_blue = 0;
            new_green = 0;
            new_red = 0;
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

                    new_blue += image_data[h][w].rgbtBlue;
                    new_green += image_data[h][w].rgbtGreen;
                    new_red += image_data[h][w].rgbtRed;
                }
            }
            new_blue /= kernel_size*kernel_size;
            new_green /= kernel_size*kernel_size;
            new_red /= kernel_size*kernel_size;
            output_image_data[i][j].rgbtBlue = new_blue;
            output_image_data[i][j].rgbtGreen = new_green;
            output_image_data[i][j].rgbtRed = new_red;
        }
    }
#if 0
    sigma = 3.0;
    kernel_size = 7;
    filter_sum = 0.0;
    filter = (double **)malloc(sizeof(double *)*kernel_size);
    for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
        filter[k+(kernel_size-1)/2] = (double *)malloc(sizeof(double)*kernel_size);
        for(l = -(kernel_size-1)/2; l <= (kernel_size-1)/2; l++) {
            filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2] = (1.0 / (2.0 * M_PI * sigma * sigma)) * exp(-1.0 * ((k*k)+(l*l)) / (2.0 * sigma * sigma));
            filter_sum += filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
        }
    }
    printf("sum:%f\n", filter_sum);
    for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
        for(l = -(kernel_size-1)/2; l <= (kernel_size-1)/2; l++) {
            filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2] /= filter_sum;
        }
    }
    for(i = 0; i < info_header.biHeight; i++) {
        for(j = 0; j < info_header.biWidth; j++) {
            new_blue_double = 0;
            new_green_double = 0;
            new_red_double = 0;
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

                    new_blue_double += image_data[h][w].rgbtBlue * filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_green_double += image_data[h][w].rgbtGreen * filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_red_double += image_data[h][w].rgbtRed * filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                }
            }
            output_image_data[i][j].rgbtBlue = (uint16_t)new_blue_double;
            output_image_data[i][j].rgbtGreen = (uint16_t)new_green_double;
            output_image_data[i][j].rgbtRed = (uint16_t)new_red_double;
        }
    }
#endif
#if 0
    sigma_d = 25.0;
    sigma_r = 25.0;
    kernel_size = 7;
    filter = (double **)malloc(sizeof(double *)*kernel_size);
    for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
        filter[k+(kernel_size-1)/2] = (double *)malloc(sizeof(double)*kernel_size);
    }
    for(i = 0; i < info_header.biHeight; i++) {
        for(j = 0; j < info_header.biWidth; j++) {
            new_blue_double = 0;
            new_green_double = 0;
            new_red_double = 0;
            new_blue_double_d = 0;
            new_green_double_d = 0;
            new_red_double_d = 0;
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

                    new_blue_double += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            (pow(image_data[i][j].rgbtBlue-image_data[h][w].rgbtBlue, 2) / (2*sigma_r*sigma_r)) )
                        * image_data[h][w].rgbtBlue;
                    new_blue_double_d += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            (pow(image_data[i][j].rgbtBlue-image_data[h][w].rgbtBlue, 2) / (2*sigma_r*sigma_r)) );
                    new_green_double += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            ((image_data[i][j].rgbtGreen-image_data[h][w].rgbtGreen)*(image_data[i][j].rgbtGreen-image_data[h][w].rgbtGreen) / (2*sigma_r*sigma_r)) )
                        * image_data[h][w].rgbtGreen;
                    new_green_double_d += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            ((image_data[i][j].rgbtGreen-image_data[h][w].rgbtGreen)*(image_data[i][j].rgbtGreen-image_data[h][w].rgbtGreen) / (2*sigma_r*sigma_r)) );
                    new_red_double += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            ((image_data[i][j].rgbtRed-image_data[h][w].rgbtRed)*(image_data[i][j].rgbtRed-image_data[h][w].rgbtRed) / (2*sigma_r*sigma_r)) )
                        * image_data[h][w].rgbtRed;
                    new_red_double_d += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            ((image_data[i][j].rgbtRed-image_data[h][w].rgbtRed)*(image_data[i][j].rgbtRed-image_data[h][w].rgbtRed) / (2*sigma_r*sigma_r)) );
                }
            }
            output_image_data[i][j].rgbtBlue = (uint16_t)(new_blue_double / new_blue_double_d);
            output_image_data[i][j].rgbtGreen = (uint16_t)(new_green_double / new_green_double_d);
            output_image_data[i][j].rgbtRed = (uint16_t)(new_red_double / new_red_double_d);
        }
    }
#endif

    output = fopen("test", "wb");
    if(output == NULL) {
        return 0;
    }

    fwrite(&file_header.bfType, 2, 1, output);
    fwrite(&file_header.bfSize, 4, 1, output);
    fwrite(&file_header.bfReserved1, 2, 1, output);
    fwrite(&file_header.bfReserved2, 2, 1, output);
    fwrite(&file_header.bfOffBits, 4, 1, output);

    fwrite(&info_header.biSize, 4, 1, output);
    fwrite(&info_header.biWidth, 4, 1, output);
    fwrite(&info_header.biHeight, 4, 1, output);
    fwrite(&info_header.biPlanes, 2, 1, output);
    info_header.biBitCount = 24;
    fwrite(&info_header.biBitCount, 2, 1, output);
    fwrite(&info_header.biCompression, 4, 1, output);
    fwrite(&info_header.biSizeImage, 4, 1, output);
    fwrite(&info_header.biXPelsPerMeter, 4, 1, output);
    fwrite(&info_header.biYPelsPerMeter, 4, 1, output);
    info_header.biClrUsed = 0;
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

    for(i = 0; i < info_header.biHeight; i++) {
        free((void *)image_data[i]);
        free((void *)output_image_data[i]);
    }
    free((void *)image_data);
    free((void *)output_image_data);

    return 0;
}
