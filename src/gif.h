#ifndef GIF_H
#define GIF_H

#include <stdio.h>
#include "bitmap.h"
#include "common.h"

void decode_gif(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data);
void init_table(void);

#endif /* GIF_H */
