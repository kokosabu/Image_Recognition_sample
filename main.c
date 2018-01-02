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
    uint16_t bfType;
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

    fread(&bfType, 2, 1, input);
    printf("%c %c\n", (bfType >> 8)&0xFF, (bfType)&0xFF); /* M B */
    printf("%x\n", bfType);

    output = fopen("test", "wb");
    if(output == NULL) {
        return 0;
    }

    fwrite(&bfType, 2, 1, output);

    fclose(input);
    fclose(output);

    return 0;
}
