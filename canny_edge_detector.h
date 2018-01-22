#ifndef CANNY_EDGE_DETECTOR_H
#define CANNY_EDGE_DETECTOR_H

#include "common.h"
#include "bitmap.h"

void canny_edge_detector(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, IMAGEINFO *image_info, double sigma, int gausian_kernel_size, int sobel_kernel_size);

#endif
