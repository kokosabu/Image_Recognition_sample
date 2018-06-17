#include "gif.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

void decode_gif(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data)
{
    char signature[4];
    char version[4];
    unsigned char byte;
    unsigned char global_color_table_flag;
    unsigned char color_resolution;
    unsigned char sort_flag;
    unsigned char size_of_global_color_table;
    unsigned char background_color_index;
    unsigned char pixel_aspect_ratio;
    RGBTRIPLE *global_color_table;
    int i;

    fread(signature, 1, 3, input);
    signature[3] = '\0';
    assert(strcmp(signature, "GIF") == 0);

    fread(version, 1, 3, input);
    version[3] = '\0';
    assert(strcmp(version, "89a") == 0);

    fread(&byte, 1, 1, input);
    image_info->width = byte;
    fread(&byte, 1, 1, input);
    image_info->width += ((unsigned int)byte) << 8;

    fread(&byte, 1, 1, input);
    image_info->height = byte;
    fread(&byte, 1, 1, input);
    image_info->height += ((unsigned int)byte) << 8;

    fread(&byte, 1, 1, input);
    global_color_table_flag    = (byte & 0x80) >> 7;
    color_resolution           = (byte & 0x70) >> 4;
    sort_flag                  = (byte & 0x08) >> 3;
    size_of_global_color_table = (byte & 0x07);

    fread(&background_color_index, 1, 1, input);
    fread(&pixel_aspect_ratio, 1, 1, input);

    if(global_color_table_flag == 1) {
        global_color_table  = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * pow(2, size_of_global_color_table));
        for(i = 0; i < pow(2, size_of_global_color_table); i++) {
            fread(&((global_color_table[i]).rgbtRed),   1, 1, input);
            fread(&((global_color_table[i]).rgbtGreen), 1, 1, input);
            fread(&((global_color_table[i]).rgbtBlue),  1, 1, input);
        }
    } else {
        global_color_table = NULL;
    }

    fread(&byte, 1, 1, input);
    if(byte == 0x2c) {
        /* Image Block */
    } else if(byte == 0x21) {
        /* Graphic Control Extension */
        /* Comment Extension */
        /* Plain Text Extension */
        /* Application Extension */
    } else {
    }

    /* Trailer(1B) = 0x3b */
}
