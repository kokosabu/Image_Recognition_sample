#include <stdint.h>
#include "bitmap.h"
#include "average_filter.h"

void average_filter(RGBTRIPLE ***output_image_data, RGBTRIPLE ***image_data, BITMAPINFOHEADER *info_header, int kernel_size)
{
    uint16_t new_blue;
    uint16_t new_green;
    uint16_t new_red;
    int i;
    int j;
    int k;
    int l;
    int h;
    int w;

    for(i = 0; i < info_header->biHeight; i++) {
        for(j = 0; j < info_header->biWidth; j++) {
            new_blue = 0;
            new_green = 0;
            new_red = 0;
            for(k = -(kernel_size-1)/2; k <= (kernel_size-1)/2; k++) {
                for(l = -(kernel_size-1)/2; l <= (kernel_size-1)/2; l++) {
                    h = i + k;
                    if(h < 0) {
                        h = 0;
                    }
                    if(h > info_header->biHeight-1) {
                        h = info_header->biHeight - 1;
                    }
                    w = j + l;
                    if(w < 0) {
                        w = 0;
                    }
                    if(w > info_header->biWidth-1) {
                        w = info_header->biWidth - 1;
                    }

                    new_blue  += (*image_data)[h][w].rgbtBlue;
                    new_green += (*image_data)[h][w].rgbtGreen;
                    new_red   += (*image_data)[h][w].rgbtRed;
                }
            }
            new_blue  /= kernel_size*kernel_size;
            new_green /= kernel_size*kernel_size;
            new_red   /= kernel_size*kernel_size;
            (*output_image_data)[i][j].rgbtBlue  = new_blue;
            (*output_image_data)[i][j].rgbtGreen = new_green;
            (*output_image_data)[i][j].rgbtRed   = new_red;
        }
    }
}
