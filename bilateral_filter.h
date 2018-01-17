#ifndef BILATERAL_FILTER_H
#define BILATERAL_FILTER_H

#include "common.h"
#include "bitmap.h"

void bilateral_filter(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, IMAGEINFO *image_info, double sigma_d, double sigma_r, int kernel_size);

#endif
