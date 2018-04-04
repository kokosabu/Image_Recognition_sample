#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bitmap.h"

void decode_bitmap(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data)
{
    uint32_t size;
    RGBQUAD *color_pallet_rgbr;
    int color_pallet_num;
    int i;
    int j;
    uint8_t pixcel;
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;

    fread(&file_header.bfType,      2, 1, input);
    fread(&file_header.bfSize,      4, 1, input);
    fread(&file_header.bfReserved1, 2, 1, input);
    fread(&file_header.bfReserved2, 2, 1, input);
    fread(&file_header.bfOffBits,   4, 1, input);

    image_info->fileSize = file_header.bfSize;

    fread(&size, 4, 1, input);
    printf("Size:%d\n", size);
    if(size == 40) {
        info_header.biSize = size;

        fread(&info_header.biWidth,         4, 1, input);
        fread(&info_header.biHeight,        4, 1, input);
        fread(&info_header.biPlanes,        2, 1, input);
        fread(&info_header.biBitCount,      2, 1, input);
        fread(&info_header.biCompression,   4, 1, input);
        fread(&info_header.biSizeImage,     4, 1, input);
        fread(&info_header.biXPelsPerMeter, 4, 1, input);
        fread(&info_header.biYPelsPerMeter, 4, 1, input);
        fread(&info_header.biClrUsed,       4, 1, input);
        fread(&info_header.biClrImportant,  4, 1, input);

        image_info->width  = info_header.biWidth;
        image_info->height = info_header.biHeight;

        printf("biWidth:%d\n",         info_header.biWidth);
        printf("biHeight:%d\n",        info_header.biHeight);
        printf("biPlanes:%d\n",        info_header.biPlanes);
        printf("biBitCount:%d\n",      info_header.biBitCount);
        printf("biCompression:%d\n",   info_header.biCompression);
        printf("biSizeImage:%d\n",     info_header.biSizeImage);
        printf("biXPelsPerMeter:%d\n", info_header.biXPelsPerMeter);
        printf("biYPelsPerMeter:%d\n", info_header.biYPelsPerMeter);
        printf("biClrUsed:%d\n",       info_header.biClrUsed);
        printf("biClrImportant:%d\n",  info_header.biClrImportant);
    } else {
        printf("Not supported file header\n");
        exit(0);
    }

    if(info_header.biSize == 40 && 
       (info_header.biBitCount == 16 || info_header.biBitCount == 32) &&
       info_header.biCompression == 3) {
        printf("Not supported bit field\n");
        exit(0);
    } else {
        ;
    }

    color_pallet_rgbr = NULL;
    color_pallet_num = 0;
    if(info_header.biBitCount == 1 ||
            info_header.biBitCount == 4 ||
            info_header.biBitCount == 8 ||
            info_header.biClrUsed >= 1) {
        if(info_header.biClrUsed >= 1) {
            color_pallet_num = info_header.biClrUsed;
        } else {
            color_pallet_num = pow(2, info_header.biClrUsed);
        }
        color_pallet_rgbr = (RGBQUAD *)malloc(sizeof(RGBQUAD)*color_pallet_num);
        for(i = 0; i < color_pallet_num; i++) {
            fread(&color_pallet_rgbr[i].rgbBlue, 1, 1, input);
            fread(&color_pallet_rgbr[i].rgbGreen, 1, 1, input);
            fread(&color_pallet_rgbr[i].rgbRed, 1, 1, input);
            fread(&color_pallet_rgbr[i].rgbReserved, 1, 1, input);
            printf("[%d] (%d, %d, %d, %d)\n", i, color_pallet_rgbr[i].rgbBlue, color_pallet_rgbr[i].rgbGreen, color_pallet_rgbr[i].rgbRed, color_pallet_rgbr[i].rgbReserved);
        }
    } else {
        ;
    }

    if(info_header.biCompression == 0 || info_header.biCompression == 3) {
        ;
    } else {
        printf("Not supported image data\n");
        exit(0);
    }

    fseek(input, file_header.bfOffBits, SEEK_SET);

    *image_data = (RGBTRIPLE **)malloc(sizeof(RGBTRIPLE *) * info_header.biHeight);
    for(i = info_header.biHeight-1; i >= 0; i--) {
        (*image_data)[i] = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * info_header.biWidth);
        for(j = 0; j < info_header.biWidth; j++) {
            (*image_data)[i][j].rgbtAlpha = 255;
            if(color_pallet_rgbr == NULL) {
                fread(&((*image_data)[i][j].rgbtBlue), 1, 1, input);
                fread(&((*image_data)[i][j].rgbtGreen), 1, 1, input);
                fread(&((*image_data)[i][j].rgbtRed), 1, 1, input);
            } else {
                fread(&pixcel, 1, 1, input);
                (*image_data)[i][j].rgbtBlue = color_pallet_rgbr[pixcel].rgbBlue;
                (*image_data)[i][j].rgbtGreen = color_pallet_rgbr[pixcel].rgbGreen;
                (*image_data)[i][j].rgbtRed = color_pallet_rgbr[pixcel].rgbRed;
            }
        }
        fseek(input, (3*info_header.biWidth)%4, SEEK_CUR);
    }

    free((void *)color_pallet_rgbr);
}

void encode_bitmap(FILE *output, IMAGEINFO *image_info, RGBTRIPLE ***output_image_data)
{
    int i;
    int j;
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    uint32_t cal;
    uint8_t write_data;
    uint8_t dummy;

    file_header.bfType      = ('M' << 8) | 'B';
    file_header.bfSize      = image_info->fileSize;
    file_header.bfReserved1 = 0;
    file_header.bfReserved2 = 0;
    file_header.bfOffBits   = 14 + 40;

    fwrite(&file_header.bfType, 2, 1, output);
    fwrite(&file_header.bfSize, 4, 1, output);
    fwrite(&file_header.bfReserved1, 2, 1, output);
    fwrite(&file_header.bfReserved2, 2, 1, output);
    fwrite(&file_header.bfOffBits, 4, 1, output);

    info_header.biSize          = 40;
    info_header.biWidth         = image_info->width;
    info_header.biHeight        = image_info->height;
    info_header.biPlanes        = 1;
    info_header.biBitCount      = 24;
    info_header.biCompression   = 0;
    info_header.biSizeImage     = image_info->width * image_info->height * 3;
    info_header.biXPelsPerMeter = 0;
    info_header.biYPelsPerMeter = 0;
    info_header.biClrUsed       = 0;
    info_header.biClrImportant  = 0;
    fwrite(&info_header.biSize,          4, 1, output);
    fwrite(&info_header.biWidth,         4, 1, output);
    fwrite(&info_header.biHeight,        4, 1, output);
    fwrite(&info_header.biPlanes,        2, 1, output);
    fwrite(&info_header.biBitCount,      2, 1, output);
    fwrite(&info_header.biCompression,   4, 1, output);
    fwrite(&info_header.biSizeImage,     4, 1, output);
    fwrite(&info_header.biXPelsPerMeter, 4, 1, output);
    fwrite(&info_header.biYPelsPerMeter, 4, 1, output);
    fwrite(&info_header.biClrUsed,       4, 1, output);
    fwrite(&info_header.biClrImportant,  4, 1, output);

    fseek(output, file_header.bfOffBits, SEEK_SET);

    for(i = info_header.biHeight-1; i >= 0; i--) {
        for(j = 0; j < info_header.biWidth; j++) {
            cal = ((uint32_t)(*output_image_data)[i][j].rgbtBlue * ((*output_image_data)[i][j].rgbtAlpha) / 255) + (255 * ((uint32_t)255 - (*output_image_data)[i][j].rgbtAlpha) / 255);
            if(cal > 255) {
                write_data = 255;
            } else {
                write_data = cal;
            }
            fwrite(&write_data, 1, 1, output);

            cal = ((uint32_t)(*output_image_data)[i][j].rgbtGreen * ((*output_image_data)[i][j].rgbtAlpha) / 255) + (255 * ((uint32_t)255 - (*output_image_data)[i][j].rgbtAlpha) / 255);
            if(cal > 255) {
                write_data = 255;
            } else {
                write_data = cal;
            }
            fwrite(&write_data, 1, 1, output);

            cal = ((uint32_t)(*output_image_data)[i][j].rgbtRed * ((*output_image_data)[i][j].rgbtAlpha) / 255) + (255 * ((uint32_t)255 - (*output_image_data)[i][j].rgbtAlpha) / 255);
            if(cal > 255) {
                write_data = 255;
            } else {
                write_data = cal;
            }
            fwrite(&write_data, 1, 1, output);
        }
        dummy = 0;
        fwrite(&dummy, 1, (3*info_header.biWidth)%4, output);
    }
}
