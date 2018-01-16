#ifndef AVERAGE_FILTER
#define AVERAGE_FILTER

#include "common.h"

void average_filter(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, IMAGEINFO *image_info, int kernel_size);

#endif
