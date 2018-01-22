#include "canny_edge_detector.h"
#include "gaussian_filter.h"
#include "sobel_filter.h"

void canny_edge_detector(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, IMAGEINFO *image_info, double sigma, int gausian_kernel_size, int sobel_kernel_size)
{
    int i;
    int j;

    gaussian_filter(output_image_data, image_data, image_info, sigma, gausian_kernel_size);
    for(i = 0; i < image_info->height; i++) {
        for(j = 0; j < image_info->width; j++) {
            (*image_data)[i][j] = (*output_image_data)[i][j];
        }
    }
    sobel_filter(output_image_data, image_data, image_info, sobel_kernel_size);
}
