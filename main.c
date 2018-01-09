#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

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
    RGBTRIPLE **output_image_data;
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
    fclose(input);

    output_image_data = (RGBTRIPLE **)malloc(sizeof(RGBTRIPLE *) * info_header.biHeight);
    for(i = info_header.biHeight-1; i >= 0; i--) {
        output_image_data[i] = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * info_header.biWidth);
    }

#if 0
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
#endif
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
