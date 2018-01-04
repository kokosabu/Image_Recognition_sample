#include <stdio.h>
#include <stdint.h>

typedef struct BITMAPFILEHEADER {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER {
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

int main(int argc, char *argv[])
{
    FILE *input;
    FILE *output;
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    uint32_t size;
    uint8_t t;

    if(argc <= 1) {
        printf("filename\n");
        return 0;
    }

    input = fopen(argv[1], "rb");
    if(input == NULL) {
        return 0;
    }

#if 0
    fread(&t, 1, 1, input);
    printf("%c\n", t); /* B */
    fread(&t, 1, 1, input);
    printf("%c\n", t); /* M */
#endif

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

    fclose(input);
    fclose(output);

    return 0;
}
