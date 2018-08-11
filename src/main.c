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
#include "gif.h"

int main(int argc, char *argv[])
{
    FILE *input;
    FILE *output;
    IMAGEINFO image_info;
    RGBTRIPLE **image_data;
    RGBTRIPLE **output_image_data;
    int i;
    int j;
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
    if(file_format == UNKOWN_FORMAT) {
        printf("unsupported file format\n");
        return 0;
    } else if(file_format == BITMAP) {
        decode_bitmap(input, &image_info, &image_data);
        for(i = 0; i < image_info.height; i++) {
            for(j = 0; j < image_info.width; j++) {
                image_data[i][j].rgbtAlpha = 255;
            }
        }
        printf("%d %d %d\n", image_info.fileSize, image_info.width, image_info.height);
    } else if(file_format == PNG) {
        decode_png(input, &image_info, &image_data);
        printf("%d %d %d\n", image_info.fileSize, image_info.width, image_info.height);
    } else if(file_format == GIF) {
        decode_gif(input, &image_info, &image_data);
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
    LoG_filter(&output_image_data, &image_data, &image_info, 0.5, 9);
   
#if 0
    for(i = 0; i < image_info.height; i++) {
        for(j = 0; j < image_info.width; j++) {
            output_image_data[i][j] = image_data[i][j];
        }
    }
#endif
#if 1
    for(i = 0; i < image_info.height; i++) {
        for(j = 0; j < image_info.width; j++) {
            output_image_data[i][j].rgbtAlpha = 255; 
        }
    }
#endif

    if(argc <= 2) {
        output = fopen("test.bmp", "wb");
    } else {
        output = fopen(argv[2], "wb");
    }
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
