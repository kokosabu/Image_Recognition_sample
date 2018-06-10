#ifndef LOG_FILTER_H
#define LOG_FILTER_H

#include "common.h"
#include "bitmap.h"

void LoG_filter(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, IMAGEINFO *image_info, double sigma, int kernel_size);

#endif
