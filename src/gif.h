#ifndef GIF_H
#define GIF_H

#include <stdio.h>
#include "bitmap.h"
#include "common.h"

void decode_gif(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data);

#endif /* GIF_H */
