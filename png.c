#include "png.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

enum {
    NONE    = 0,
    SUB     = 1,
    UP      = 2,
    AVERAGE = 3,
    PAETH   = 4,

    END_OF_BLOCK = 256
};

uint32_t width;
uint32_t height;

void chunk_read(FILE *input, uint8_t **output_stream, uint8_t **png_image_data, RGBTRIPLE **color_palette, PNG_INFO *png_info)
{
    uint32_t idat_size;
    uint8_t flag;
    uint32_t size;
    char chunk[5];
    uint8_t bps;
    uint8_t color_type;
    uint8_t compress_type;
    uint8_t filter_type;
    uint8_t interlace_type;
    uint32_t crc_32;
    uint8_t *tmp;
    int i;
    int k;
    uint32_t palette_size;
    uint32_t gamma;
    uint8_t rendering_intent;
    uint32_t white_point_x;
    uint32_t white_point_y;
    uint32_t red_x;
    uint32_t red_y;
    uint32_t green_x;
    uint32_t green_y;
    uint32_t blue_x;
    uint32_t blue_y;
    uint32_t x_axis;
    uint32_t y_axis;
    uint8_t unit;
    uint32_t VirtualImageWidth;
    uint32_t VirtualImageHeight;
    uint32_t VirtualPageUnits;
    char keyword[80];
    char *text;
    uint8_t *alpha_index;
    uint16_t alpha_gray;
    uint16_t alpha_red;
    uint16_t alpha_green;
    uint16_t alpha_blue;
    uint8_t sbit_red;
    uint8_t sbit_green;
    uint8_t sbit_blue;
    uint8_t sbit_gray;
    uint8_t sbit_alpha;
    uint8_t background_color_pallet;
    uint16_t background_color_gray;
    uint16_t background_color_red;
    uint16_t background_color_green;
    uint16_t background_color_blue;
    uint16_t *image_histgram;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    idat_size = 0;
    flag = 0;
    do {
        size = read_4bytes(input);
        fread(&chunk[0], 1, 1, input);
        fread(&chunk[1], 1, 1, input);
        fread(&chunk[2], 1, 1, input);
        fread(&chunk[3], 1, 1, input);
        chunk[4] = '\0';

        if(strcmp("IHDR", chunk) == 0) {
            width = read_4bytes(input);
            height = read_4bytes(input);
            fread(&bps, 1, 1, input);
            fread(&color_type, 1, 1, input);
            fread(&compress_type, 1, 1, input);
            fread(&filter_type, 1, 1, input);
            fread(&interlace_type, 1, 1, input);
            crc_32 = read_4bytes(input);

            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            printf("width:%d\n", width);
            printf("height:%d\n", height);
            printf("bps:%d\n", bps);
            printf("color type:%d\n", color_type);
            printf("compress type:%d\n", compress_type);
            printf("filter type:%d\n", filter_type);
            printf("interlace type:%d\n", interlace_type);
            printf("crc-32: %xh\n", crc_32);

            *output_stream = (uint8_t *)malloc(sizeof(uint8_t) * (width+1) * height);

        } else if(strcmp(chunk, "IDAT") == 0) {
            idat_size += size;
            if(size == idat_size) {
                *png_image_data = (uint8_t *)malloc(sizeof(uint8_t) * size);
                fread((*png_image_data)+idat_size-size, 1, size, input);
            } else {
                tmp = (uint8_t *)realloc(*png_image_data, sizeof(uint8_t) * idat_size);
                *png_image_data = tmp;
                fread((*png_image_data)+idat_size-size, 1, size, input);
            }
            crc_32 = read_4bytes(input);
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            printf("crc-32: %xh\n", crc_32);
        } else if(strcmp(chunk, "PLTE") == 0) {
            *color_palette = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * size);
            palette_size = size;

            for(i = 0; i < size/3; i++) {
                (*color_palette)[i].rgbtRed   = 0;
                (*color_palette)[i].rgbtGreen = 0;
                (*color_palette)[i].rgbtBlue  = 0;
                fread(&((*color_palette)[i].rgbtRed),   1, 1, input);
                fread(&((*color_palette)[i].rgbtGreen), 1, 1, input);
                fread(&((*color_palette)[i].rgbtBlue),  1, 1, input);
            }
            crc_32 = read_4bytes(input);

            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            for(i = 0; i < size/3; i++) {
                printf("[%d] : %d %d %d\n", i, (*color_palette)[i].rgbtRed, (*color_palette)[i].rgbtGreen, (*color_palette)[i].rgbtBlue);
            }
            printf("crc-32: %xh\n", crc_32);
        } else if(strcmp(chunk, "IEND") == 0) {
            crc_32 = read_4bytes(input);
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            flag = 1;
        } else if(strcmp(chunk, "gAMA") == 0) {
            gamma = read_4bytes(input);
            crc_32 = read_4bytes(input);
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            printf("gamma:%f(%d)\n", gamma/100000.0, gamma);
        } else if(strcmp(chunk, "sRGB") == 0) {
            fread(&rendering_intent, 1, 1, input);
            crc_32 = read_4bytes(input);
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            printf("Rendering intent:%d\n", rendering_intent);
        } else if(strcmp(chunk, "cHRM") == 0) {
            white_point_x = read_4bytes(input);
            white_point_y = read_4bytes(input);
            red_x         = read_4bytes(input);
            red_y         = read_4bytes(input);
            green_x       = read_4bytes(input);
            green_y       = read_4bytes(input);
            blue_x        = read_4bytes(input);
            blue_y        = read_4bytes(input);
            crc_32 = read_4bytes(input);
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            printf("white point x:%d\n", white_point_x);
            printf("white point y:%d\n", white_point_y);
            printf("red x:%d\n", red_x);
            printf("red y:%d\n", red_y);
            printf("green x:%d\n", green_x);
            printf("green y:%d\n", green_y);
            printf("blue x:%d\n", blue_x);
            printf("blue y:%d\n", blue_y);
        } else if(strcmp(chunk, "pHYs") == 0) {
            x_axis = read_4bytes(input);
            y_axis = read_4bytes(input);
            fread(&unit, 1, 1, input);
            crc_32 = read_4bytes(input);
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            printf("x axis:%d\n", x_axis);
            printf("y axis:%d\n", y_axis);
            printf("unit:%d\n", unit);
        } else if(strcmp(chunk, "vpAg") == 0) {
            VirtualImageWidth = read_4bytes(input);
            VirtualImageHeight = read_4bytes(input);
            fread(&VirtualPageUnits, 1, 1, input);
            crc_32 = read_4bytes(input);
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            printf("VirtualImageWidth:%d\n", VirtualImageWidth);
            printf("VirtualImageHeight:%d\n", VirtualImageHeight);
            printf("VirtualPageUnits:%d\n", VirtualPageUnits);
        } else if(strcmp(chunk, "tEXt") == 0) {
            k = 0;
            do {
                fread(&keyword[k], 1, 1, input);
                k++;
            } while(keyword[k-1] != '\0');
            text = (char *)malloc(sizeof(char) * (size-k+1));
            while(k < size) {
                fread(&text[k], 1, 1, input);
                k++;
            }
            text[k] = '\0';
            crc_32 = read_4bytes(input);
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            printf("keyword: %s\n", keyword);
            printf("Text:%s\n", text);
        } else if(strcmp(chunk, "tRNS") == 0) {
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);

            if(color_type == 3) {
                alpha_index = (uint8_t *)malloc(sizeof(uint8_t) * palette_size);
                for(k = 0; k < palette_size; k++) {
                    fread(&alpha_index[k], 1, 1, input);
                }
            } else if(color_type == 0) {
                alpha_gray = read_2bytes(input);
            } else if(color_type == 2) {
                alpha_red = read_2bytes(input);
                alpha_green = read_2bytes(input);
                alpha_blue = read_2bytes(input);
            }

            crc_32 = read_4bytes(input);
        } else if(strcmp(chunk, "sBIT") == 0) {
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);

            if(color_type == 3) {
                fread(&sbit_red, 1, 1, input);
                fread(&sbit_green, 1, 1, input);
                fread(&sbit_blue, 1, 1, input);
            } else if(color_type == 0) {
                fread(&sbit_gray, 1, 1, input);
            } else if(color_type == 2) {
                fread(&sbit_red, 1, 1, input);
                fread(&sbit_green, 1, 1, input);
                fread(&sbit_blue, 1, 1, input);
            } else if(color_type == 4) {
                fread(&sbit_gray, 1, 1, input);
                fread(&sbit_alpha, 1, 1, input);
            } else if(color_type == 6) {
                fread(&sbit_red, 1, 1, input);
                fread(&sbit_green, 1, 1, input);
                fread(&sbit_blue, 1, 1, input);
                fread(&sbit_alpha, 1, 1, input);
            }

            crc_32 = read_4bytes(input);
        } else if(strcmp(chunk, "bKGD") == 0) {
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);

            if(color_type == 3) {
                fread(&background_color_pallet, 1, 1, input);
            } else if(color_type == 0 || color_type == 4) {
                background_color_gray = read_2bytes(input);
            } else if(color_type == 2 || color_type == 6) {
                background_color_red = read_2bytes(input);
                background_color_green = read_2bytes(input);
                background_color_blue = read_2bytes(input);
            }

            crc_32 = read_4bytes(input);
        } else if(strcmp(chunk, "hIST") == 0) {
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);

            printf("palet : %d\n", palette_size);
            image_histgram = (uint16_t *)malloc(sizeof(uint16_t) * palette_size / 3);

            for(i = 0; i < palette_size / 3; i++) {
                image_histgram[i] = read_2bytes(input);
            }

            crc_32 = read_4bytes(input);
        } else if(strcmp(chunk, "tIME") == 0) {
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);

            year = read_2bytes(input);
            fread(&month, 1, 1, input);
            fread(&day, 1, 1, input);
            fread(&hour, 1, 1, input);
            fread(&minute, 1, 1, input);
            fread(&second, 1, 1, input);

            crc_32 = read_4bytes(input);
        } else if(strcmp(chunk, "eXIf") == 0) {
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            for(i = 0; i < size; i++) {
                fread(&k, 1, 1, input);
            }
            crc_32 = read_4bytes(input);
        } else {
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            printf("Don't support chunk. Exit!\n");
            exit(0);
        }
    } while(flag == 0);

    png_info->color_type = color_type;
    png_info->bps        = bps;
}

void read_zlib_header(uint8_t *png_image_data, int *byte_index, int *bit_index)
{
    uint8_t cmf;
    uint8_t cm;
    uint8_t cinfo;
    uint32_t window_size;
    int flg;
    uint8_t fdict;
    uint32_t dictid;

    cmf = png_image_data[*byte_index];
    *byte_index += 1;
    printf("%02xh\n", cmf);
    cm = cmf & 0x0F;
    cinfo = (cmf & 0xF0) >> 4;
    printf("%02xh, %02xh\n", cm, cinfo);

    if(cm != 8) {
        printf("not deflate\n");
        exit(0);
    }
    if(cinfo > 7) {
        printf("CINFO above 7 are not allowed in this version of the specification\n");
        exit(0);
    }
    window_size = pow(2, cinfo + 8);

    flg = png_image_data[*byte_index];
    *byte_index += 1;
    fdict = (flg & 0x10) >> 5;
    printf("fdict %d\n", fdict);

    if(fdict == 1) {
        dictid =               png_image_data[*(byte_index  )];
        dictid = dictid << 8 | png_image_data[*(byte_index+1)];
        dictid = dictid << 8 | png_image_data[*(byte_index+2)];
        dictid = dictid << 8 | png_image_data[*(byte_index+3)];
        *byte_index += 4;
    }
}

unsigned int decode_huffman(uint8_t *png_image_data, int *byte_index, int *bit_index, struct tree *huffman_tree, int len)
{
    unsigned int code;
    unsigned int code_len;
    unsigned int i;

    /* decode literal/length value from input stream */
    code = 0;
    code_len = 0;
    do {
        code <<= 1;
        code |= huffman_bit_read(png_image_data, byte_index, bit_index, 1);
        code_len += 1;
        for(i = 0; i < len; i++) {
            if(huffman_tree[i].len == code_len && huffman_tree[i].code == code) {
                printf("[%d] %d, %d\n", i, huffman_tree[i].len, huffman_tree[i].code);
                break;
            }
        }
    } while(i == len);

    return i;
}

void calc_next_code(struct tree *tree, int *lens, int *next_code, size_t bl_count_size, size_t tree_size)
{
    int i;
    int *bl_count;
    int max_bits;
    int code;
    int bits;
    uint16_t min_len;
    uint16_t len;

    bl_count = (int *)malloc(sizeof(int)*bl_count_size);

    for(i = 0; i < tree_size; i++) {
        tree[i].len = lens[i];
    }
    for(i = 0; i < bl_count_size; i++) {
        bl_count[i] = 0;
    }
    for(i = 0; i < tree_size; i++) {
        bl_count[lens[i]] += 1;
    }
    max_bits = 0;
    for(i = 0; i < bl_count_size; i++) {
        if(0 < bl_count[i]) {
            max_bits = i+1;
        }
    }
    code = 0;
    bl_count[0] = 0;
    for (bits = 1; bits <= max_bits; bits++) {
        code = (code + bl_count[bits-1]) << 1;
        next_code[bits] = code;
    }

    min_len = UINT16_MAX;
    for (i = 0; i < tree_size; i++) {
        len = tree[i].len;
        if (len != 0) {
            tree[i].code = next_code[len];
            next_code[len]++;
            if(len < min_len) {
                min_len = len;
            }
        }
    }

    free((void *)bl_count);
}

void decompress_fixed_huffman_codes(uint8_t *png_image_data, int *byte_index, int *bit_index, int *lit, int *dist, struct tree *tree, struct tree *dtree)
{
    int i;

    *lit = 288;
    *dist = 32;

    for(i = 0x00; i <= 0x8F; i++) {
        tree[i].code = 0x30 + i;
        tree[i].len  = 8;
    }
    for(i = 0x90; i <= 0xFF; i++) {
        tree[i].code = 0x190 + (i - 0x90);
        tree[i].len  = 9;
    }
    for(i = 0x100; i <= 0x117; i++) {
        tree[i].code = 0x00 + (i - 0x100);
        tree[i].len  = 7;
    }
    for(i = 0x118; i <= 0x11F; i++) {
        tree[i].code = 0xC0 + (i - 0x118);
        tree[i].len  = 8;
    }

#if 0
    for(i = 0; i <= 0x11F; i++) {
        printf("[%3d] [%2d] %4d\n", i, tree[i].len, tree[i].code);
    }
#endif

    for(i = 0; i < 32; i++) {
        dtree[i].code = i;
        dtree[i].len  = 5;
    }
}

void decompress_dynamic_huffman_codes(uint8_t *png_image_data, int *byte_index, int *bit_index, int *lit, int *dist, struct tree *tree, struct tree *dtree)
{
    int hlit;
    int hdist;
    int hclen;
    int hclens[19];
    int next_code[288];
    int *id;
    int id_index;
    int value;
    int i;
    int repeat;
    int last_id;
    int hclens_index_table[19] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };

    /* read representation of code trees (see subsection below) */
    hlit = bit_read(png_image_data, byte_index, bit_index, 5);
    printf("hlit = %d\n", hlit);
    hdist = bit_read(png_image_data, byte_index, bit_index, 5);
    printf("hdist = %d\n", hdist);
    hclen = bit_read(png_image_data, byte_index, bit_index, 4);
    printf("hclen = %d\n", hclen);
    for(i = 0; i < 19; i++) {
        hclens[i] = 0;
    }
    for(i = 0; i < (hclen+4); i++) {
        hclens[hclens_index_table[i]] = bit_read(png_image_data, byte_index, bit_index, 3);
    }

    //clen
    calc_next_code(tree, hclens, next_code, 8, 19);

    *lit = hlit + 257;
    *dist = hdist + 1;
    id = (int *)malloc(sizeof(int) * (*lit+*dist));
    id_index = 0;

    do {
        int bit_read_table[19] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 2, 3, 7
        };
        int repeat_base[19] = {
            1, 1, 1, 1, 1, 1, 1, 1,  1, 1,
            1, 1, 1, 1, 1, 1, 3, 3, 11
        };
        value = decode_huffman(png_image_data, byte_index, bit_index, &(tree[0]), 19);

        if(value >= 0 && value <= 15) {
            repeat = bit_read(png_image_data, byte_index, bit_index, bit_read_table[value]) + repeat_base[value];
            last_id = value;
            for(i = 0; i < repeat; i++) {
                id[id_index] = last_id;
                id_index += 1;
            }
        } else if(value == 16) {
            repeat = bit_read(png_image_data, byte_index, bit_index, bit_read_table[value]) + repeat_base[value];
            last_id = id[id_index-1];
            for(i = 0; i < repeat; i++) {
                id[id_index] = last_id;
                id_index += 1;
            }
        } else if(value == 17) {
            repeat = bit_read(png_image_data, byte_index, bit_index, bit_read_table[value]) + repeat_base[value];
            last_id = 0;
            for(i = 0; i < repeat; i++) {
                id[id_index] = last_id;
                id_index += 1;
            }
        } else if(value == 18) {
            repeat = bit_read(png_image_data, byte_index, bit_index, bit_read_table[value]) + repeat_base[value];
            last_id = 0;
            for(i = 0; i < repeat; i++) {
                id[id_index] = last_id;
                id_index += 1;
            }
        }
    } while(id_index != (*lit+*dist));

    printf("lit = %d\n", *lit);
    printf("dist = %d\n", *dist);
    printf("lit+dist = %d\n", *lit+*dist);
    calc_next_code(tree,  &(id[0]),    next_code, 288, *lit);
    calc_next_code(dtree, &(id[*lit]), next_code,  32, *dist);

    free((void *)id);
}


int get_color_data(uint8_t *output_stream, int *write_byte_index, PNG_INFO *png_info, int *index)
{
    int data;
    int i;

    if(png_info->bps == 1 || png_info->bps == 2 || png_info->bps == 4 || png_info->bps == 8) {
        printf("[%d][%d] : %3d : ", *write_byte_index, *index, output_stream[*write_byte_index]);

        data = 0;
        for(i = 0; i < png_info->bps; i++) {
            data <<= 1;
            data |= image_bit_read(output_stream, write_byte_index, index, 1);
        }

        printf("%d\n", data);
       
        if(png_info->color_type != 3) {
            switch(png_info->bps) {
                case 1:
                    data = data * 255 / (2-1);
                    break;
                case 2:
                    data = data * 255 / (4-1);
                    break;
                case 4:
                    data = data * 255 / (16-1);
                    break;
                case 8:
                default:
                    data = data * 255 / (256-1);
                    break;
            }
        }
    } else {
        data = bit_read(output_stream, write_byte_index, index, 8);
        data = data << 8 | bit_read(output_stream, write_byte_index, index, 8);
    }

    return data;
}

RGBTRIPLE get_color(RGBTRIPLE *color_palette, uint8_t *output_stream, int *write_byte_index, PNG_INFO *png_info, int *index)
{
    RGBTRIPLE c;
    int data;

    if(png_info->color_type == 0) {
        data = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtRed   = data;
        c.rgbtGreen = data;
        c.rgbtBlue  = data;
    } else if(png_info->color_type == 2) {
        data = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtRed   = data;
        data = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtGreen = data;
        data = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtBlue  = data;
    } else if(png_info->color_type == 3) {
        data = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtRed   = color_palette[data].rgbtRed;
        c.rgbtGreen = color_palette[data].rgbtGreen;
        c.rgbtBlue  = color_palette[data].rgbtBlue;
    } else {
        c.rgbtRed   = 0;
        c.rgbtGreen = 0;
        c.rgbtBlue  = 0;
    }

    return c;
}

unsigned int paeth_predictor(int a, int b, int c)
{
    int p;
    int pa;
    int pb;
    int pc;

    p = a + b - c;
    pa = abs(p - a);
    pb = abs(p - b);
    pc = abs(p - c);

    if(pa <= pb && pa <= pc) {
        return a;
    } else if(pb <= pc) {
        return b;
    } else {
        return c;
    }
}

void write_line(uint8_t *output_stream, int i, int *write_byte_index, int width, RGBTRIPLE ***image_data, RGBTRIPLE *color_palette, PNG_INFO *png_info)
{
    int j;
    int k;
    uint16_t old_red;
    uint16_t old_green;
    uint16_t old_blue;
    uint16_t up_red;
    uint16_t up_green;
    uint16_t up_blue;
    uint16_t left_red;
    uint16_t left_green;
    uint16_t left_blue;
    uint16_t upper_left_red;
    uint16_t upper_left_green;
    uint16_t upper_left_blue;
    RGBTRIPLE c;

    printf("[%d] %d\n", i, output_stream[*write_byte_index]);
    if(output_stream[*write_byte_index] == NONE) {
        *write_byte_index += 1;
        k = 0;
        for(j = 0; j < width; j++) {
            c = get_color(color_palette, output_stream, write_byte_index, png_info, &k);
            if(png_info->bps != 16) {
                (*image_data)[i][j].rgbtBlue  = c.rgbtBlue;
                (*image_data)[i][j].rgbtGreen = c.rgbtGreen;
                (*image_data)[i][j].rgbtRed   = c.rgbtRed;
            } else {
                (*image_data)[i][j].rgbtBlue  = c.rgbtBlue  >> 8;
                (*image_data)[i][j].rgbtGreen = c.rgbtGreen >> 8;
                (*image_data)[i][j].rgbtRed   = c.rgbtRed   >> 8;
            }
        }
    } else if(output_stream[*write_byte_index] == SUB) {
        *write_byte_index += 1;
        k = 0;
        old_red   = 0;
        old_green = 0;
        old_blue  = 0;
        for(j = 0; j < width; j++) {
            c = get_color(color_palette, output_stream, write_byte_index, png_info, &k);
            if(png_info->bps != 16) {
                (*image_data)[i][j].rgbtRed   = (c.rgbtRed   + old_red)   % 256;
                (*image_data)[i][j].rgbtGreen = (c.rgbtGreen + old_green) % 256;
                (*image_data)[i][j].rgbtBlue  = (c.rgbtBlue  + old_blue)  % 256;
                old_red   = (*image_data)[i][j].rgbtRed;
                old_green = (*image_data)[i][j].rgbtGreen;
                old_blue  = (*image_data)[i][j].rgbtBlue;
            } else {
                (*image_data)[i][j].rgbtRed   = ((c.rgbtRed   + old_red)   % 65536) >> 8;
                (*image_data)[i][j].rgbtGreen = ((c.rgbtGreen + old_green) % 65536) >> 8;
                (*image_data)[i][j].rgbtBlue  = ((c.rgbtBlue  + old_blue)  % 65536) >> 8;
                old_red   = (*image_data)[i][j].rgbtRed   << 8;
                old_green = (*image_data)[i][j].rgbtGreen << 8;
                old_blue  = (*image_data)[i][j].rgbtBlue  << 8;
            }
        }
    } else if(output_stream[*write_byte_index] == UP) {
        *write_byte_index += 1;
        k = 0;
        for(j = 0; j < width; j++) {
            if(i == 0) {
                old_red   = 0;
                old_green = 0;
                old_blue  = 0;
            } else {
                if(png_info->bps != 16) {
                    old_red   = (*image_data)[i-1][j].rgbtRed;
                    old_green = (*image_data)[i-1][j].rgbtGreen;
                    old_blue  = (*image_data)[i-1][j].rgbtBlue;
                } else {
                    old_red   = (*image_data)[i-1][j].rgbtRed   << 8;
                    old_green = (*image_data)[i-1][j].rgbtGreen << 8;
                    old_blue  = (*image_data)[i-1][j].rgbtBlue  << 8;
                }
            }
            c = get_color(color_palette, output_stream, write_byte_index, png_info, &k);
            if(png_info->bps != 16) {
                (*image_data)[i][j].rgbtRed   = (c.rgbtRed   + old_red)   % 256;
                (*image_data)[i][j].rgbtGreen = (c.rgbtGreen + old_green) % 256;
                (*image_data)[i][j].rgbtBlue  = (c.rgbtBlue  + old_blue)  % 256;
            } else {
                (*image_data)[i][j].rgbtRed   = ((c.rgbtRed   + old_red)   % 65536) >> 8;
                (*image_data)[i][j].rgbtGreen = ((c.rgbtGreen + old_green) % 65536) >> 8;
                (*image_data)[i][j].rgbtBlue  = ((c.rgbtBlue  + old_blue)  % 65536) >> 8;
            }
        }
    } else if(output_stream[*write_byte_index] == AVERAGE) {
        *write_byte_index += 1;
        k = 0;
        old_red   = 0;
        old_green = 0;
        old_blue  = 0;
        for(j = 0; j < width; j++) {
            if(i == 0) {
                old_red   += 0;
                old_green += 0;
                old_blue  += 0;
            } else {
                if(png_info->bps != 16) {
                    old_red   += (*image_data)[i-1][j].rgbtRed;
                    old_green += (*image_data)[i-1][j].rgbtGreen;
                    old_blue  += (*image_data)[i-1][j].rgbtBlue;
                } else {
                    old_red   += (*image_data)[i-1][j].rgbtRed   << 8;
                    old_green += (*image_data)[i-1][j].rgbtGreen << 8;
                    old_blue  += (*image_data)[i-1][j].rgbtBlue  << 8;
                }
            }
            c = get_color(color_palette, output_stream, write_byte_index, png_info, &k);
            if(png_info->bps != 16) {
                (*image_data)[i][j].rgbtRed   = (c.rgbtRed   + old_red   / 2) % 256;
                (*image_data)[i][j].rgbtGreen = (c.rgbtGreen + old_green / 2) % 256;
                (*image_data)[i][j].rgbtBlue  = (c.rgbtBlue  + old_blue  / 2) % 256;
            } else {
                (*image_data)[i][j].rgbtRed   = ((c.rgbtRed   + old_red   / 2) % 65536) >> 8;
                (*image_data)[i][j].rgbtGreen = ((c.rgbtGreen + old_green / 2) % 65536) >> 8;
                (*image_data)[i][j].rgbtBlue  = ((c.rgbtBlue  + old_blue  / 2) % 65536) >> 8;
            }
            old_red   = (*image_data)[i][j].rgbtRed   << 8;
            old_green = (*image_data)[i][j].rgbtGreen << 8;
            old_blue  = (*image_data)[i][j].rgbtBlue  << 8;
        }
    } else if(output_stream[*write_byte_index] == PAETH) {
        *write_byte_index += 1;
        k = 0;

        left_blue  = 0;
        left_green = 0;
        left_red   = 0;
        upper_left_red   = 0;
        upper_left_green = 0;
        upper_left_blue  = 0;
        for(j = 0; j < width; j++) {
            if(i == 0) {
                up_red   = 0;
                up_green = 0;
                up_blue  = 0;
                upper_left_red   = 0;
                upper_left_green = 0;
                upper_left_blue  = 0;
            } else if(j == 0) {
                if(png_info->bps != 16) {
                    up_blue  = (*image_data)[i-1][j].rgbtBlue;
                    up_green = (*image_data)[i-1][j].rgbtGreen;
                    up_red   = (*image_data)[i-1][j].rgbtRed;
                } else {
                    up_blue  = (*image_data)[i-1][j].rgbtBlue  << 8;
                    up_green = (*image_data)[i-1][j].rgbtGreen << 8;
                    up_red   = (*image_data)[i-1][j].rgbtRed   << 8;
                }
                upper_left_red   = 0;
                upper_left_green = 0;
                upper_left_blue  = 0;
            } else {
                if(png_info->bps != 16) {
                    up_blue  = (*image_data)[i-1][j].rgbtBlue;
                    up_green = (*image_data)[i-1][j].rgbtGreen;
                    up_red   = (*image_data)[i-1][j].rgbtRed;
                    upper_left_red   = (*image_data)[i-1][j-1].rgbtRed;
                    upper_left_green = (*image_data)[i-1][j-1].rgbtGreen;
                    upper_left_blue  = (*image_data)[i-1][j-1].rgbtBlue;
                } else {
                    up_blue  = (*image_data)[i-1][j].rgbtBlue  << 8;
                    up_green = (*image_data)[i-1][j].rgbtGreen << 8;
                    up_red   = (*image_data)[i-1][j].rgbtRed   << 8;
                    upper_left_red   = (*image_data)[i-1][j-1].rgbtRed   << 8;
                    upper_left_green = (*image_data)[i-1][j-1].rgbtGreen << 8;
                    upper_left_blue  = (*image_data)[i-1][j-1].rgbtBlue  << 8;
                }
            }
            c = get_color(color_palette, output_stream, write_byte_index, png_info, &k);
            if(png_info->bps != 16) {
                (*image_data)[i][j].rgbtRed   = (c.rgbtRed   + paeth_predictor(left_red,   up_red,   upper_left_red))   % 256;
                (*image_data)[i][j].rgbtGreen = (c.rgbtGreen + paeth_predictor(left_green, up_green, upper_left_green)) % 256;
                (*image_data)[i][j].rgbtBlue  = (c.rgbtBlue  + paeth_predictor(left_blue,  up_blue,  upper_left_blue))  % 256;
                left_red   = (*image_data)[i][j].rgbtRed;
                left_green = (*image_data)[i][j].rgbtGreen;
                left_blue  = (*image_data)[i][j].rgbtBlue;
            } else {
                (*image_data)[i][j].rgbtRed   = ((c.rgbtRed   + paeth_predictor(left_red,   up_red,   upper_left_red))   % 65536) >> 8;
                (*image_data)[i][j].rgbtGreen = ((c.rgbtGreen + paeth_predictor(left_green, up_green, upper_left_green)) % 65536) >> 8;
                (*image_data)[i][j].rgbtBlue  = ((c.rgbtBlue  + paeth_predictor(left_blue,  up_blue,  upper_left_blue))  % 65536) >> 8;
                left_red   = (*image_data)[i][j].rgbtRed   << 8;
                left_green = (*image_data)[i][j].rgbtGreen << 8;
                left_blue  = (*image_data)[i][j].rgbtBlue  << 8;
            }
        }
    } else {
        printf("undefined filter type\n");
        exit(0);
    }
}

void decode_huffman_codes(uint8_t *png_image_data, int *byte_index, int *bit_index, struct tree *tree, int lit, uint8_t *output_stream, int *write_byte_index, struct tree *dtree, int dist)
{
    int i;
    int value;
    int len_bit;
    uint16_t len;
    int len_bit_value;
    int dist_bit;
    int dist_bit_value;
    int len_block_bit[29] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 257-266
        1, 1, 2, 2, 2, 2, 3, 3, 3, 3, // 267-276
        4, 4, 4, 4, 5, 5, 5, 5, 0     // 277-285
    };
    int len_block[29] = {
        3,  4,  5,  6,   7,   8,   9,   10,  11, 13,
        15, 17, 19, 23,  27,  31,  35,  43,  51, 59,
        67, 83, 99, 115, 131, 163, 195, 227, 258
    };
    int dist_block_bit[30] = {
        0, 0, 0,  0,  1,  1,  2,  2,  3,  3,
        4, 4, 5,  5,  6,  6,  7,  7,  8,  8,
        9, 9, 10, 10, 11, 11, 12, 12, 13, 13
    };
    int dist_block[30] = {
        1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
        33,   49,   65,   97,  129,  193,  257,   385,   513,   769,
        1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
    };

    /* loop (until end of block code recognized) */
    do {
        value = decode_huffman(png_image_data, byte_index, bit_index, tree, lit);

        if(value < 256) {
            /* copy value (literal byte) to output stream */
            output_stream[*write_byte_index] = value;
            printf("write_byte:%d = %d  (%d)(%d)\n", *write_byte_index, value, *byte_index, *bit_index);
            *write_byte_index += 1;
        } else if(value == END_OF_BLOCK) {
            printf("%d\n", *write_byte_index);
            break;
        } else {/* (value = 257..285) */
            printf("value = %d\n", value - 257);
            len_bit = len_block_bit[value - 257];
            len_bit_value = 0;
            if(len_bit != 0) {
                len_bit_value = bit_read(png_image_data, byte_index, bit_index, len_bit);
            }
            len  = len_block[value-257];
            len += len_bit_value;

            /* decode distance from input stream */
            value = decode_huffman(png_image_data, byte_index, bit_index, dtree, 32);
            dist_bit = dist_block_bit[value];
            dist_bit_value = 0;
            if(dist_bit != 0) {
                dist_bit_value = bit_read(png_image_data, byte_index, bit_index, dist_bit);
            }

            dist = dist_block[value];
            dist += dist_bit_value;
            printf("write_byte:%d (%d)(%d)\n", *write_byte_index, *write_byte_index/(width+1), *write_byte_index%(width+1));
            printf("dist = %d, len = %d\n", dist, len);
            /* move backwards distance bytes in the output stream, and copy length bytes from this position to the output stream. */
            for(i = 0; i < len; i++) {
                output_stream[*write_byte_index] = output_stream[*write_byte_index-dist];
                *write_byte_index += 1;
            }
        }
    } while(1);
}

void decode_png(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data)
{
    uint8_t *output_stream;
    int i;
    uint8_t *png_image_data;
    int bit_index;
    int byte_index;
    int write_byte_index;
    int bfinal;
    int btype;
    uint16_t len;
    uint16_t nlen;
    RGBTRIPLE *color_palette;
    int lit;
    int dist;
    struct tree tree[288];
    struct tree dtree[32];
    PNG_INFO png_info;

    printf("PNG\n");

    fseek(input, 8, SEEK_CUR);

    chunk_read(input, &output_stream, &png_image_data, &color_palette, &png_info);

    bit_index = 0;
    byte_index = 0;
    write_byte_index = 0;
    do {
        read_zlib_header(png_image_data, &byte_index, &bit_index);

        /* read block header from input stream. */
        bfinal = bit_read(png_image_data, &byte_index, &bit_index, 1);
        btype = bit_read(png_image_data, &byte_index, &bit_index, 2);
        printf("%02x %02x\n", bfinal, btype);

        if(btype == 0x00) {
            /* skip any remaining bits in current partially processed byte */
            byte_index++;

            /* read LEN and NLEN (see next section) */
            len = (png_image_data[byte_index] << 8) | png_image_data[byte_index+1];
            byte_index += 2;
            nlen = (png_image_data[byte_index] << 8) | png_image_data[byte_index+1];
            byte_index += 2;

            /* copy LEN bytes of data to output */
            for(i = 0; i < len; i++) {
                output_stream[write_byte_index] = png_image_data[byte_index];
                write_byte_index += 1;
                byte_index += 1;
            }
        } else if(btype == 0x01) {
            decompress_fixed_huffman_codes(png_image_data, &byte_index, &bit_index, &lit, &dist, tree, dtree);
            decode_huffman_codes(png_image_data, &byte_index, &bit_index, tree, lit, output_stream, &write_byte_index, dtree, dist);
        } else if(btype == 0x02) {
            /* if compressed with dynamic Huffman codes */
            decompress_dynamic_huffman_codes(png_image_data, &byte_index, &bit_index, &lit, &dist, tree, dtree);
            decode_huffman_codes(png_image_data, &byte_index, &bit_index, tree, lit, output_stream, &write_byte_index, dtree, dist);
        }
    } while(bfinal == 0);

    printf("write_byte : %d\n", write_byte_index);
    (*image_data) = (RGBTRIPLE **)malloc(sizeof(RGBTRIPLE *) * height);
    for(i = 0; i < height; i++) {
        (*image_data)[i] = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * width);
    }

    write_byte_index = 0;
    for(i = 0; i < height; i++) {
        write_line(output_stream, i, &write_byte_index, width, image_data, color_palette, &png_info);
    }

    image_info->height   = height;
    image_info->width    = width;
    image_info->fileSize = height*width*3 + 54;
}
