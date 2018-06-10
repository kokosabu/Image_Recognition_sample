#include <stdlib.h>
#include <math.h>
#include "gaussian_filter.h"

void gaussian_filter(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, IMAGEINFO *image_info, double sigma, int kernel_size)
{
    double filter_sum;
    double **filter;
    int k;
    int l;
    int i;
    int j;
    double new_blue_double;
    double new_green_double;
    double new_red_double;
    int h;
    int w;

    filter_sum = 0.0;
    filter = (double **)malloc(sizeof(double *)*kernel_size);
    for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
        filter[k+(kernel_size-1)/2] = (double *)malloc(sizeof(double)*kernel_size);
        for(l = -(kernel_size-1)/2; l <= (kernel_size-1)/2; l++) {
            filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2] = (1.0 / (2.0 * M_PI * sigma * sigma)) * exp(-1.0 * ((k*k)+(l*l)) / (2.0 * sigma * sigma));
            filter_sum += filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
        }
    }
    for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
        for(l = -(kernel_size-1)/2; l <= (kernel_size-1)/2; l++) {
            filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2] /= filter_sum;
        }
    }

    for(i = 0; i < image_info->height; i++) {
        for(j = 0; j < image_info->width; j++) {
            new_blue_double = 0;
            new_green_double = 0;
            new_red_double = 0;
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

                    new_blue_double  += (*image_data)[h][w].rgbtBlue  * filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_green_double += (*image_data)[h][w].rgbtGreen * filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                    new_red_double   += (*image_data)[h][w].rgbtRed   * filter[k+(kernel_size-1)/2][l+(kernel_size-1)/2];
                }
            }
            (*output_image_data)[i][j].rgbtBlue  = (uint16_t)new_blue_double;
            (*output_image_data)[i][j].rgbtGreen = (uint16_t)new_green_double;
            (*output_image_data)[i][j].rgbtRed   = (uint16_t)new_red_double;
        }
    }

    for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
        free((void *)filter[k+(kernel_size-1)/2]);
    }
    free((void *)filter);
}
