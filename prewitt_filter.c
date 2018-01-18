#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "prewitt_filter.h"

void prewitt_filter(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, IMAGEINFO *image_info, int kernel_size)
{
    int i;
    int j;
    int k;
    int l;
    int w;
    int h;
    double **filter_x;
    double **filter_y;
    double new_blue_double_x;
    double new_blue_double_y;
    double new_green_double_x;
    double new_green_double_y;
    double new_red_double_x;
    double new_red_double_y;

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
    for(i = 0; i < image_info->height; i++) {
        for(j = 0; j < image_info->width; j++) {
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
                    if(h > image_info->height-1) {
                        h = image_info->height - 1;
                    }
                    w = j + l;
                    if(w < 0) {
                        w = 0;
                    }
                    if(w > image_info->width-1) {
                        w = image_info->width - 1;
                    }

                    new_blue_double_x  += ((*image_data)[h][w].rgbtBlue)  * filter_x[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_blue_double_y  += ((*image_data)[h][w].rgbtBlue)  * filter_y[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_green_double_x += ((*image_data)[h][w].rgbtGreen) * filter_x[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_green_double_y += ((*image_data)[h][w].rgbtGreen) * filter_y[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_red_double_x   += ((*image_data)[h][w].rgbtRed)   * filter_x[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_red_double_y   += ((*image_data)[h][w].rgbtRed)   * filter_y[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                }
            }
            (*output_image_data)[i][j].rgbtBlue  = (uint8_t)sqrt(pow(new_blue_double_x,  2) + pow(new_blue_double_y,  2));
            (*output_image_data)[i][j].rgbtGreen = (uint8_t)sqrt(pow(new_green_double_x, 2) + pow(new_green_double_y, 2));
            (*output_image_data)[i][j].rgbtRed   = (uint8_t)sqrt(pow(new_red_double_x,   2) + pow(new_red_double_y,   2));
        }
    }

    for(k = 0; k < kernel_size; k++) {
        free((void *)filter_x[k]);
        free((void *)filter_y[k]);
    }
    free((void *)filter_x);
    free((void *)filter_y);
}
