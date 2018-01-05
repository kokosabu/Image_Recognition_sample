#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct tagBITMAPFILEHEADER {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  uint32_t biSize;
  int32_t  biWidth;
  int32_t  biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t  biXPelsPerMeter;
  int32_t  biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBTRIPLE { 
  uint8_t rgbtBlue; 
  uint8_t rgbtGreen; 
  uint8_t rgbtRed; 
} RGBTRIPLE;

typedef struct tagRGBQUAD {
  uint8_t rgbBlue;
  uint8_t rgbGreen;
  uint8_t rgbRed;
  uint8_t rgbReserved;
} RGBQUAD;

int main(int argc, char *argv[])
{
    FILE *input;
    FILE *output;
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    uint32_t size;
    RGBTRIPLE **image_data;
    int i;
    int j;
    uint8_t dummy;

    if(argc <= 1) {
        printf("filename\n");
        return 0;
    }

    input = fopen(argv[1], "rb");
    if(input == NULL) {
        return 0;
    }

    fread(&file_header.bfType, 2, 1, input);
    printf("bfType:%c%c\n", (file_header.bfType >> 8)&0xFF, (file_header.bfType)&0xFF); /* M B */
    fread(&file_header.bfSize, 4, 1, input);
    printf("bfSize:%d\n", file_header.bfSize);
    fread(&file_header.bfReserved1, 2, 1, input);
    fread(&file_header.bfReserved2, 2, 1, input);
    fread(&file_header.bfOffBits, 4, 1, input);
    printf("bfOffBits:%d\n", file_header.bfOffBits);

    fread(&size, 4, 1, input);
    printf("Size:%d\n", size);
    if(size == 40) {
        info_header.biSize = size;

        fread(&info_header.biWidth, 4, 1, input);
        printf("biWidth:%d\n", info_header.biWidth);
        fread(&info_header.biHeight, 4, 1, input);
        printf("biHeight:%d\n", info_header.biHeight);
        fread(&info_header.biPlanes, 2, 1, input);
        printf("biPlanes:%d\n", info_header.biPlanes);
        fread(&info_header.biBitCount, 2, 1, input);
        printf("biBitCount:%d\n", info_header.biBitCount);
        fread(&info_header.biCompression, 4, 1, input);
        printf("biCompression:%d\n", info_header.biCompression);
        fread(&info_header.biSizeImage, 4, 1, input);
        printf("biSizeImage:%d\n", info_header.biSizeImage);
        fread(&info_header.biXPelsPerMeter, 4, 1, input);
        printf("biXPelsPerMeter:%d\n", info_header.biXPelsPerMeter);
        fread(&info_header.biYPelsPerMeter, 4, 1, input);
        printf("biYPelsPerMeter:%d\n", info_header.biYPelsPerMeter);
        fread(&info_header.biClrUsed, 4, 1, input);
        printf("biClrUsed:%d\n", info_header.biClrUsed);
        fread(&info_header.biClrImportant, 4, 1, input);
        printf("biClrImportant:%d\n", info_header.biClrImportant);

    } else {
        printf("Not supported file header\n");
        return 0;
    }

    if(info_header.biSize == 40 && 
       (info_header.biBitCount == 16 || info_header.biBitCount == 32) &&
       info_header.biCompression == 3) {
        printf("Not supported bit field\n");
        return 0;
    } else {
        ;
    }

    if(info_header.biBitCount == 1 ||
            info_header.biBitCount == 4 ||
            info_header.biBitCount == 8 ||
            info_header.biClrUsed >= 1) {
        printf("Not supported color pallet\n");
        return 0;
    } else {
        ;
    }

    if(info_header.biCompression == 0 || info_header.biCompression == 3) {
        ;
    } else {
        printf("Not supported image data\n");
        return 0;
    }

    if(info_header.biBitCount == 24) {
        ;
    } else {
        printf("Not supported Bit Count\n");
        return 0;
    }

    fseek(input, file_header.bfOffBits, SEEK_SET);

    image_data = (RGBTRIPLE **)malloc(sizeof(RGBTRIPLE *) * info_header.biHeight);
    for(i = info_header.biHeight-1; i >= 0; i--) {
        image_data[i] = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * info_header.biWidth);
        for(j = 0; j < info_header.biWidth; j++) {
            fread(&image_data[i][j].rgbtBlue, 1, 1, input);
            fread(&image_data[i][j].rgbtGreen, 1, 1, input);
            fread(&image_data[i][j].rgbtRed, 1, 1, input);
        }
        fseek(input, (3*info_header.biWidth)%4, SEEK_CUR);
    }

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
    fwrite(&info_header.biBitCount, 2, 1, output);
    fwrite(&info_header.biCompression, 4, 1, output);
    fwrite(&info_header.biSizeImage, 4, 1, output);
    fwrite(&info_header.biXPelsPerMeter, 4, 1, output);
    fwrite(&info_header.biYPelsPerMeter, 4, 1, output);
    fwrite(&info_header.biClrUsed, 4, 1, output);
    fwrite(&info_header.biClrImportant, 4, 1, output);

    for(i = info_header.biHeight-1; i >= 0; i--) {
        for(j = 0; j < info_header.biWidth; j++) {
            fwrite(&image_data[i][j].rgbtBlue, 1, 1, output);
            fwrite(&image_data[i][j].rgbtGreen, 1, 1, output);
            fwrite(&image_data[i][j].rgbtRed, 1, 1, output);
        }
        dummy = 0;
        fwrite(&dummy, 1, (3*info_header.biWidth)%4, output);
    }

    fclose(input);
    fclose(output);

    for(i = 0; i < info_header.biHeight; i++) {
        free((void *)image_data[i]);
    }
    free((void *)image_data);

    return 0;
}
