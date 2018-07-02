#include "gif.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static uint8_t *lzw_table[4096];
static int lzw_table_size;

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
    uint16_t image_left_position;
    uint16_t image_top_position;
    uint16_t image_width;
    uint16_t image_height;
    uint8_t local_color_table_flag;
    uint8_t interlace_flag;
    uint8_t local_sort_flag;
    uint8_t reserved;
    uint8_t size_of_local_color_table;
    uint8_t LZW_minimum_code_size;
    uint8_t block_size;
    uint8_t block_image_data[255];
    uint8_t block_terminator;
    uint8_t disposal_mothod;
    uint8_t user_input_flag;
    uint8_t transparent_color_flag;
    uint16_t delay_time;
    uint8_t transparent_color_index;
    uint8_t *comment_data;
    uint16_t text_grid_left_position;
    uint16_t text_grid_top_position;
    uint16_t text_grid_width;
    uint16_t text_grid_height;
    uint8_t character_cell_width;
    uint8_t character_cell_height;
    uint8_t text_foreground_color_index;
    uint8_t text_background_color_index;
    uint8_t *plain_text_data;
    uint8_t application_identifier[8];
    uint8_t application_authentication_code[3];
    uint8_t *application_data;
    RGBTRIPLE *global_color_table;
    RGBTRIPLE *local_color_table;
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
        global_color_table  = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * (uint8_t)pow(2, size_of_global_color_table+1));
        for(i = 0; i < (uint8_t)pow(2, size_of_global_color_table+1); i++) {
            fread(&((global_color_table[i]).rgbtRed),   1, 1, input);
            fread(&((global_color_table[i]).rgbtGreen), 1, 1, input);
            fread(&((global_color_table[i]).rgbtBlue),  1, 1, input);
        }
    } else {
        global_color_table = NULL;
    }

    do {
        fread(&byte, 1, 1, input);
        if(byte == 0x2c) {
            /* Image Block */
            fread(&byte, 1, 1, input);
            image_left_position = byte;
            fread(&byte, 1, 1, input);
            image_left_position += ((unsigned int)byte) << 8;

            fread(&byte, 1, 1, input);
            image_top_position = byte;
            fread(&byte, 1, 1, input);
            image_top_position += ((unsigned int)byte) << 8;

            fread(&byte, 1, 1, input);
            image_width = byte;
            fread(&byte, 1, 1, input);
            image_width += ((unsigned int)byte) << 8;

            fread(&byte, 1, 1, input);
            image_height = byte;
            fread(&byte, 1, 1, input);
            image_height += ((unsigned int)byte) << 8;

            fread(&byte, 1, 1, input);
            local_color_table_flag    = (byte & 0x80) >> 7;
            interlace_flag            = (byte & 0x40) >> 6;
            local_sort_flag           = (byte & 0x20) >> 5;
            reserved                  = (byte & 0x18) >> 3;
            size_of_local_color_table = (byte & 0x07);

            if(local_color_table_flag == 1) {
                local_color_table  = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * pow(2, size_of_local_color_table));
                for(i = 0; i < pow(2, size_of_local_color_table); i++) {
                    fread(&((local_color_table[i]).rgbtRed),   1, 1, input);
                    fread(&((local_color_table[i]).rgbtGreen), 1, 1, input);
                    fread(&((local_color_table[i]).rgbtBlue),  1, 1, input);
                }
            } else {
                local_color_table = NULL;
            }
            fread(&LZW_minimum_code_size, 1, 1, input);
            fread(&block_size, 1, 1, input);
            fread(block_image_data, 1, block_size, input);
            fread(&block_terminator, 1, 1, input);
            if(block_terminator != 0x00) {
                printf("Block: Image Block error\n");
                exit(1);
            }
        } else if(byte == 0x21) {
            fread(&byte, 1, 1, input);

            if(byte == 0xf9) {
                /* Graphic Control Extension */
                fread(&byte, 1, 1, input);
                if(byte != 0x04) {
                    printf("error\n");
                    exit(1);
                }

                fread(&byte, 1, 1, input);
                reserved               = (byte & 0xE0) >> 5;
                disposal_mothod        = (byte & 0x1C) >> 2;
                user_input_flag        = (byte & 0x02) >> 1;
                transparent_color_flag = (byte & 0x01);

                fread(&transparent_color_index, 1, 1, input);

                fread(&byte, 1, 1, input);
                delay_time = byte;
                fread(&byte, 1, 1, input);
                delay_time += ((unsigned int)byte) << 8;

                fread(&block_terminator, 1, 1, input);
                if(block_terminator != 0x00) {
                    printf("Block: Graphic Control Extension error\n");
                    exit(1);
                }
            } else if(byte == 0xfe) {
                /* Comment Extension */
                fread(&byte, 1, 1, input);

                if(byte != 0x00) {
                    block_size = byte;

                    comment_data = (uint8_t *)malloc(sizeof(uint8_t) * block_size);
                    fread(comment_data, 1, block_size, input);

                    fread(&byte, 1, 1, input);
                }

                if(byte != 0x00) {
                    printf("Block: Comment Extension error\n");
                    exit(1);
                }
            } else if(byte == 0x01) {
                /* Plain Text Extension */
                fread(&block_size, 1, 1, input);
                if(block_size != 0x0c) {
                    printf("error\n");
                    exit(1);
                }

                fread(&byte, 1, 1, input);
                text_grid_left_position = byte;
                fread(&byte, 1, 1, input);
                text_grid_left_position += ((unsigned int)byte) << 8;

                fread(&byte, 1, 1, input);
                text_grid_top_position = byte;
                fread(&byte, 1, 1, input);
                text_grid_top_position += ((unsigned int)byte) << 8;

                fread(&byte, 1, 1, input);
                text_grid_width = byte;
                fread(&byte, 1, 1, input);
                text_grid_width += ((unsigned int)byte) << 8;

                fread(&byte, 1, 1, input);
                text_grid_height = byte;
                fread(&byte, 1, 1, input);
                text_grid_height += ((unsigned int)byte) << 8;

                fread(&character_cell_width, 1, 1, input);
                fread(&character_cell_height, 1, 1, input);
                fread(&text_foreground_color_index, 1, 1, input);
                fread(&text_background_color_index, 1, 1, input);
                fread(&byte, 1, 1, input);
                if(byte != 0x00) {
                    plain_text_data = (uint8_t *)malloc(sizeof(uint8_t) * byte);
                    fread(plain_text_data, 1, byte, input);
                    fread(&byte, 1, 1, input);
                }

                if(byte != 0x00) {
                    printf("Block: Comment Extension error\n");
                    exit(1);
                }
            } else if(byte == 0xff) {
                /* Application Extension */
                fread(&block_size, 1, 1, input);
                if(block_size != 0x0b) {
                    printf("error\n");
                    exit(1);
                }

                fread(application_identifier, 1, 8, input);
                fread(application_authentication_code, 1, 3, input);
                fread(&byte, 1, 1, input);
                if(byte != 0x00) {
                    application_data = (uint8_t *)malloc(sizeof(uint8_t) * byte);
                    fread(application_data, 1, byte, input);
                    fread(&byte, 1, 1, input);
                }

                if(byte != 0x00) {
                    printf("error\n");
                    exit(1);
                }
            } else {
                printf("error\n");
                exit(1);
            }
        } else if(byte == 0x3b) {
            /* Trailer(1B) = 0x3b */
            break;
        } else {
            printf("error\n");
            exit(1);
        }
    } while(1);
}

void init_table(int bit)
{
    int i;
    for(i = 0; i < pow(2, bit-1); i++) {
        lzw_table[i] = (uint8_t *)malloc(sizeof(uint8_t) * 1);
        lzw_table[i][0] = i;
    }
    lzw_table[i] = (uint8_t *)CLEAR;
    i += 1;
    lzw_table[i] = (uint8_t *)END;

    lzw_table_size = 0x102;
}

uint8_t *get_data(int index)
{
    return lzw_table[index];
}

void compress(uint8_t *compress_data, int compress_data_size, uint8_t *original_data, int original_data_size)
{
    uint8_t W;
    uint8_t K;
    int N;
    int I;
    int i;

    uint8_t prefix;
    uint8_t suffix;
    int compress_data_index;
    int original_data_index;

    compress_data_index = 0;
    original_data_index = 0;

    for(i = 0; i < lzw_table_size; i++) {
        if(lzw_table[i] == (uint8_t *)CLEAR) {
            compress_data[compress_data_index] = i;
        }
    }
    compress_data_index += 1;

    prefix = original_data[original_data_index];
    original_data_index += 1;

    suffix = original_data[original_data_index];


    W = original_data[0];

    I = 1;

    for(i = 0; i < lzw_table_size; i++) {
        if(W == lzw_table[i][0]) {
            N = i;
        }
    }

    K = original_data[I];

    I++;
}
