#ifndef SOBEL_FILTER_H
#define SOBEL_FILTER_H

#include "common.h"
#include "bitmap.h"

void sobel_filter(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, IMAGEINFO *image_info, int kernel_size);

#endif
