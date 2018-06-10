#ifndef PREWITT_FILTER_H
#define PREWITT_FILTER_H

#include "common.h"
#include "bitmap.h"

void prewitt_filter(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, IMAGEINFO *image_info, int kernel_size);

#endif
