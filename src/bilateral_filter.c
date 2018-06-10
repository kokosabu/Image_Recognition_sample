#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "bitmap.h"
#include "bilateral_filter.h"

void bilateral_filter(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, IMAGEINFO *image_info, double sigma_d, double sigma_r, int kernel_size)
{
    double **filter;
    int i;
    int j;
    int k;
    int l;
    int h;
    int w;
    double new_blue_double;
    double new_green_double;
    double new_red_double;
    double new_blue_double_d;
    double new_green_double_d;
    double new_red_double_d;

    filter = (double **)malloc(sizeof(double *)*kernel_size);
    for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
        filter[k+(kernel_size-1)/2] = (double *)malloc(sizeof(double)*kernel_size);
    }
    for(i = 0; i < image_info->height; i++) {
        for(j = 0; j < image_info->width; j++) {
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

                    new_blue_double += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            (pow((*image_data)[i][j].rgbtBlue-(*image_data)[h][w].rgbtBlue, 2) / (2*sigma_r*sigma_r)) )
                        * (*image_data)[h][w].rgbtBlue;
                    new_blue_double_d += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            (pow((*image_data)[i][j].rgbtBlue-(*image_data)[h][w].rgbtBlue, 2) / (2*sigma_r*sigma_r)) );
                    new_green_double += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            (((*image_data)[i][j].rgbtGreen-(*image_data)[h][w].rgbtGreen)*((*image_data)[i][j].rgbtGreen-(*image_data)[h][w].rgbtGreen) / (2*sigma_r*sigma_r)) )
                        * (*image_data)[h][w].rgbtGreen;
                    new_green_double_d += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            (((*image_data)[i][j].rgbtGreen-(*image_data)[h][w].rgbtGreen)*((*image_data)[i][j].rgbtGreen-(*image_data)[h][w].rgbtGreen) / (2*sigma_r*sigma_r)) );
                    new_red_double += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            (((*image_data)[i][j].rgbtRed-(*image_data)[h][w].rgbtRed)*((*image_data)[i][j].rgbtRed-(*image_data)[h][w].rgbtRed) / (2*sigma_r*sigma_r)) )
                        * (*image_data)[h][w].rgbtRed;
                    new_red_double_d += exp( -1.0 * ((((l)*(l)) + ((k)*(k))) / (2*sigma_d*sigma_d)) -
                            (((*image_data)[i][j].rgbtRed-(*image_data)[h][w].rgbtRed)*((*image_data)[i][j].rgbtRed-(*image_data)[h][w].rgbtRed) / (2*sigma_r*sigma_r)) );
                }
            }
            (*output_image_data)[i][j].rgbtBlue  = (uint16_t)(new_blue_double  / new_blue_double_d);
            (*output_image_data)[i][j].rgbtGreen = (uint16_t)(new_green_double / new_green_double_d);
            (*output_image_data)[i][j].rgbtRed   = (uint16_t)(new_red_double   / new_red_double_d);
        }
    }

    for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
        free((void *)filter[k+(kernel_size-1)/2]);
    }
    free((void *)filter);
}
