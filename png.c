#include "png.h"
#include "crc.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

enum {
    NONE    = 0,
    SUB     = 1,
    UP      = 2,
    AVERAGE = 3,
    PAETH   = 4,

    END_OF_BLOCK = 256
};

struct chunk_read_info {
    char *name;
    void (*func)(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data);
};

void chunk_read_ihdr(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t crc_32;
    uint32_t crc;

    assert(size == (25-12)); /* 12 = Length, Chunk Type, CRS */
    png_info->width  = read_4bytes(input);
    png_info->height = read_4bytes(input);
    fread(&(png_info->bps),            1, 1, input);
    fread(&(png_info->color_type),     1, 1, input);
    fread(&(png_info->compress_type),  1, 1, input);
    fread(&(png_info->filter_type),    1, 1, input);
    fread(&(png_info->interlace_type), 1, 1, input);
    crc_32 = read_4bytes(input);

    crc = 0xFFFFFFFF;
    crc = crc32((uint8_t *)chunk, 4, crc);
    crc = crc32_4bytes(&(png_info->width), crc);
    crc = crc32_4bytes(&(png_info->height), crc);
    crc = crc32(&(png_info->bps),            1, crc);
    crc = crc32(&(png_info->color_type),     1, crc);
    crc = crc32(&(png_info->compress_type),  1, crc);
    crc = crc32(&(png_info->filter_type),    1, crc);
    crc = crc32(&(png_info->interlace_type), 1, crc);
    crc ^= 0xFFFFFFFF;
    if(crc != crc_32) {
        printf("incorrect %s checksum\n", chunk);
        exit(0);
    }

    printf("width:%d\n", png_info->width);
    printf("height:%d\n", png_info->height);
    printf("bps:%d\n", png_info->bps);
    printf("color type:%d\n", png_info->color_type);
    printf("interlace type:%d\n", png_info->interlace_type);

    assert(png_info->compress_type == 0);
    assert(png_info->filter_type == 0);

    if(png_info->bps == 1 || png_info->bps == 2 || png_info->bps == 4 || png_info->bps == 8 || png_info->bps == 16) {
    } else {
        printf("Don't support depth. Exit!\n");
        exit(0);
    }

    if(png_info->color_type == 0 || png_info->color_type == 2 || png_info->color_type == 3 || png_info->color_type == 4 || png_info->color_type == 5 || png_info->color_type == 6) {
    } else {
        printf("Don't support color_type. Exit!\n");
        exit(0);
    }

    *output_stream = (uint8_t *)malloc(sizeof(uint8_t) * (png_info->width+1) * png_info->height);
}

void chunk_read_idat(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t crc_32;
    uint32_t crc;
    uint8_t *tmp;

    png_info->idat_size += size;

    crc = 0xFFFFFFFF;
    crc = crc32((uint8_t *)chunk, 4, crc);
    if(size == png_info->idat_size) {
        *png_image_data = (uint8_t *)malloc(sizeof(uint8_t) * size);
        printf("png_image_data size : %d\n", size);
        fread((*png_image_data)+png_info->idat_size-size, 1, size, input);
        crc = crc32((*png_image_data)+png_info->idat_size-size, size, crc);
    } else {
        tmp = (uint8_t *)realloc(*png_image_data, sizeof(uint8_t) * png_info->idat_size);
        *png_image_data = tmp;
        fread((*png_image_data)+png_info->idat_size-size, 1, size, input);
        crc = crc32((*png_image_data)+png_info->idat_size-size, size, crc);
    }
    crc ^= 0xFFFFFFFF;
    crc_32 = read_4bytes(input);
    printf("chunk:%s\n", chunk);
    if(crc != crc_32) {
        printf("incorrect %s checksum\n", chunk);
        exit(0);
    }
}

void chunk_read_plte(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t crc_32;
    int i;

    png_info->color_palette = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * size);
    png_info->palette_size = size;

    assert((size%3) == 0);
    for(i = 0; i < size/3; i++) {
        (png_info->color_palette)[i].rgbtRed   = 0;
        (png_info->color_palette)[i].rgbtGreen = 0;
        (png_info->color_palette)[i].rgbtBlue  = 0;
        fread(&((png_info->color_palette[i]).rgbtRed),   1, 1, input);
        fread(&((png_info->color_palette[i]).rgbtGreen), 1, 1, input);
        fread(&((png_info->color_palette[i]).rgbtBlue),  1, 1, input);
    }
    crc_32 = read_4bytes(input);

    for(i = 0; i < size/3; i++) {
        printf("PLTE [%d] : %d %d %d\n", i, (png_info->color_palette)[i].rgbtRed, (png_info->color_palette)[i].rgbtGreen, (png_info->color_palette)[i].rgbtBlue);
    }
}

void chunk_read_iend(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t crc_32;
    uint32_t crc;

    assert(size == (12-12)); /* 12 = Length, Chunk Type, CRS */
    crc = 0xFFFFFFFF;
    crc = crc32((uint8_t *)chunk, 4, crc);
    crc ^= 0xFFFFFFFF;
    crc_32 = read_4bytes(input);
    if(crc != crc_32) {
        printf("incorrect %s checksum\n", chunk);
        exit(0);
    }

    png_info->flag = 1;
}

void chunk_read_gama(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t crc_32;

    png_info->gamma = read_4bytes(input);
    crc_32 = read_4bytes(input);
    printf("gamma:%f(%d)\n", png_info->gamma/100000.0, png_info->gamma);
}

void chunk_read_srgb(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t crc_32;
    uint8_t rendering_intent;

    fread(&rendering_intent, 1, 1, input);
    crc_32 = read_4bytes(input);
    printf("Rendering intent:%d\n", rendering_intent);
}

void chunk_read_chrm(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t white_point_x;
    uint32_t white_point_y;
    uint32_t red_x;
    uint32_t red_y;
    uint32_t green_x;
    uint32_t green_y;
    uint32_t blue_x;
    uint32_t blue_y;
    uint32_t crc_32;

    white_point_x = read_4bytes(input);
    white_point_y = read_4bytes(input);
    red_x         = read_4bytes(input);
    red_y         = read_4bytes(input);
    green_x       = read_4bytes(input);
    green_y       = read_4bytes(input);
    blue_x        = read_4bytes(input);
    blue_y        = read_4bytes(input);
    crc_32        = read_4bytes(input);
    printf("white point x:%d\n", white_point_x);
    printf("white point y:%d\n", white_point_y);
    printf("red x:%d\n", red_x);
    printf("red y:%d\n", red_y);
    printf("green x:%d\n", green_x);
    printf("green y:%d\n", green_y);
    printf("blue x:%d\n", blue_x);
    printf("blue y:%d\n", blue_y);
}

void chunk_read_phys(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t x_axis;
    uint32_t y_axis;
    uint8_t unit;
    uint32_t crc_32;

    x_axis = read_4bytes(input);
    y_axis = read_4bytes(input);
    fread(&unit, 1, 1, input);
    crc_32 = read_4bytes(input);
    printf("chunk:%s\n", chunk);
    printf("x axis:%d\n", x_axis);
    printf("y axis:%d\n", y_axis);
    printf("unit:%d\n", unit);
}

void chunk_read_vpag(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t VirtualImageWidth;
    uint32_t VirtualImageHeight;
    uint32_t VirtualPageUnits;
    uint32_t crc_32;

    VirtualImageWidth = read_4bytes(input);
    VirtualImageHeight = read_4bytes(input);
    fread(&VirtualPageUnits, 1, 1, input);
    crc_32 = read_4bytes(input);
    printf("chunk:%s\n", chunk);
    printf("VirtualImageWidth:%d\n", VirtualImageWidth);
    printf("VirtualImageHeight:%d\n", VirtualImageHeight);
    printf("VirtualPageUnits:%d\n", VirtualPageUnits);
}

void chunk_read_text(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    char keyword[80];
    char *text;
    uint32_t crc_32;
    int i;

    i = 0;
    do {
        fread(&keyword[i], 1, 1, input);
        i++;
    } while(keyword[i-1] != '\0');
    text = (char *)malloc(sizeof(char) * (size-i+1));
    while(i < size) {
        fread(&text[i], 1, 1, input);
        i++;
    }
    text[i] = '\0';
    crc_32 = read_4bytes(input);
    printf("chunk:%s\n", chunk);
    printf("keyword: %s\n", keyword);
    printf("Text:%s\n", text);

    free((void *)text);
}

void chunk_read_trns(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t crc_32;
    int k;

    if(png_info->color_type == 3) {
        png_info->alpha_index = (uint8_t *)malloc(sizeof(uint8_t) * size);
        for(k = 0; k < size; k++) {
            fread(&(png_info->alpha_index[k]), 1, 1, input);
            printf("[%d] : %d (alpha)\n", k, png_info->alpha_index[k]);
        }
    } else if(png_info->color_type == 0) {
        png_info->alpha_gray = (uint16_t *)malloc(sizeof(uint16_t) * size/2);
        for(k = 0; k < size/2; k++) {
            png_info->alpha_gray[k] = read_2bytes(input);
        }
    } else if(png_info->color_type == 2) {
        png_info->alpha_red   = (uint16_t *)malloc(sizeof(uint16_t) * size/6);
        png_info->alpha_green = (uint16_t *)malloc(sizeof(uint16_t) * size/6);
        png_info->alpha_blue  = (uint16_t *)malloc(sizeof(uint16_t) * size/6);
        for(k = 0; k < size/6; k++) {
            png_info->alpha_red[k]   = read_2bytes(input);
            png_info->alpha_green[k] = read_2bytes(input);
            png_info->alpha_blue[k]  = read_2bytes(input);
        }
    }

    crc_32 = read_4bytes(input);

    png_info->tRNS_size = size;
}

void chunk_read_sbit(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint8_t sbit_red;
    uint8_t sbit_green;
    uint8_t sbit_blue;
    uint8_t sbit_gray;
    uint8_t sbit_alpha;
    uint32_t crc_32;

    if(png_info->color_type == 3) {
        fread(&sbit_red, 1, 1, input);
        fread(&sbit_green, 1, 1, input);
        fread(&sbit_blue, 1, 1, input);
    } else if(png_info->color_type == 0) {
        fread(&sbit_gray, 1, 1, input);
    } else if(png_info->color_type == 2) {
        fread(&sbit_red, 1, 1, input);
        fread(&sbit_green, 1, 1, input);
        fread(&sbit_blue, 1, 1, input);
    } else if(png_info->color_type == 4) {
        fread(&sbit_gray, 1, 1, input);
        fread(&sbit_alpha, 1, 1, input);
    } else if(png_info->color_type == 6) {
        fread(&sbit_red, 1, 1, input);
        fread(&sbit_green, 1, 1, input);
        fread(&sbit_blue, 1, 1, input);
        fread(&sbit_alpha, 1, 1, input);
    }

    crc_32 = read_4bytes(input);
}

void chunk_read_bkgd(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint8_t background_color_pallet;
    uint16_t background_color_gray;
    uint16_t background_color_red;
    uint16_t background_color_green;
    uint16_t background_color_blue;
    uint32_t crc_32;

    if(png_info->color_type == 3) {
        fread(&background_color_pallet, 1, 1, input);
    } else if(png_info->color_type == 0 || png_info->color_type == 4) {
        background_color_gray = read_2bytes(input);
    } else if(png_info->color_type == 2 || png_info->color_type == 6) {
        background_color_red   = read_2bytes(input);
        background_color_green = read_2bytes(input);
        background_color_blue  = read_2bytes(input);
    }

    crc_32 = read_4bytes(input);
}

void chunk_read_hist(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint32_t crc_32;
    uint16_t *image_histgram;
    int i;

    printf("palet : %d\n", png_info->palette_size);
    image_histgram = (uint16_t *)malloc(sizeof(uint16_t) * png_info->palette_size / 3);

    for(i = 0; i < png_info->palette_size / 3; i++) {
        image_histgram[i] = read_2bytes(input);
    }

    crc_32 = read_4bytes(input);

    free((void *)image_histgram);
}

void chunk_read_time(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint32_t crc_32;

    year = read_2bytes(input);
    fread(&month,  1, 1, input);
    fread(&day,    1, 1, input);
    fread(&hour,   1, 1, input);
    fread(&minute, 1, 1, input);
    fread(&second, 1, 1, input);
    crc_32 = read_4bytes(input);
}

void chunk_read_exif(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    int i;
    uint8_t k;
    uint32_t crc_32;

    printf("chunk:%s\n", chunk);
    for(i = 0; i < size; i++) {
        fread(&k, 1, 1, input);
    }
    crc_32 = read_4bytes(input);
}

void chunk_read_itxt(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    char keyword_itxt[160+1];
    char tag[160+1];
    char keyword[80];
    char text2[160+1];
    uint8_t compress_flag;
    uint8_t compress_type;
    int k;
    int l;
    uint32_t crc_32;

    k = 0;
    do {
        fread(&keyword_itxt[k], 1, 1, input);
        k++;
    } while(keyword_itxt[k-1] != '\0');

    fread(&compress_flag, 1, 1, input);
    k++;
    fread(&compress_type, 1, 1, input);
    k++;

    l = 0;
    do {
        fread(&tag[l], 1, 1, input);
        k++;
        l++;
    } while(tag[l-1] != '\0');

    l = 0;
    do {
        fread(&keyword[l], 1, 1, input);
        k++;
        l++;
    } while(keyword[l-1] != '\0');

    l = 0;
    text2[l] = '\0';
    while(k < size) {
        fread(&text2[l], 1, 1, input);
        k++;
        l++;
        text2[l] = '\0';
    }

    printf("keyword:%s\n", keyword_itxt);
    printf("compress_flag:%d\n", compress_flag);
    printf("compress_type:%d\n", compress_type);
    printf("tag:%s\n", tag);
    printf("keyword:%s\n", keyword);
    printf("text:%s\n", text2);
    printf("size:%d\n", size);
    printf("chunk:%s\n", chunk);

    crc_32 = read_4bytes(input);
}

void chunk_read_splt(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    int k;
    int i;
    char pallet_name[160+1];
    uint8_t sample;
    uint16_t *red_sample;
    uint16_t *green_sample;
    uint16_t *blue_sample;
    uint16_t *alpha_sample;
    uint16_t *freq_sample;
    uint32_t crc_32;

    printf("chunk:%s\n", chunk);
    k = 0;
    do {
        fread(&pallet_name[k], 1, 1, input);
        k++;
    } while(pallet_name[k-1] != '\0');
    printf("%s\n", pallet_name);
    fread(&sample, 1, 1, input);
    printf("%d\n", sample);
    if(sample == 8) {
        red_sample   = (uint16_t *)malloc(sizeof(uint16_t) * (size - strlen(pallet_name) - 2) / 6);
        green_sample = (uint16_t *)malloc(sizeof(uint16_t) * (size - strlen(pallet_name) - 2) / 6);
        blue_sample  = (uint16_t *)malloc(sizeof(uint16_t) * (size - strlen(pallet_name) - 2) / 6);
        alpha_sample = (uint16_t *)malloc(sizeof(uint16_t) * (size - strlen(pallet_name) - 2) / 6);
        freq_sample  = (uint16_t *)malloc(sizeof(uint16_t) * (size - strlen(pallet_name) - 2) / 6);
        for(i = 0; i < ((size - strlen(pallet_name) - 2) / 6); i++) {
            fread(&red_sample[i],   1, 1, input);
            fread(&green_sample[i], 1, 1, input);
            fread(&blue_sample[i],  1, 1, input);
            fread(&alpha_sample[i], 1, 1, input);
            freq_sample[i] = read_2bytes(input);
        }
    } else if(sample == 16) {
        red_sample   = (uint16_t *)malloc(sizeof(uint16_t) * (size - strlen(pallet_name) - 2) / 10);
        green_sample = (uint16_t *)malloc(sizeof(uint16_t) * (size - strlen(pallet_name) - 2) / 10);
        blue_sample  = (uint16_t *)malloc(sizeof(uint16_t) * (size - strlen(pallet_name) - 2) / 10);
        alpha_sample = (uint16_t *)malloc(sizeof(uint16_t) * (size - strlen(pallet_name) - 2) / 10);
        freq_sample  = (uint16_t *)malloc(sizeof(uint16_t) * (size - strlen(pallet_name) - 2) / 10);
        for(i = 0; i < ((size - strlen(pallet_name) - 2) / 10); i++) {
            red_sample[i]   = read_2bytes(input);
            green_sample[i] = read_2bytes(input);
            blue_sample[i]  = read_2bytes(input);
            alpha_sample[i] = read_2bytes(input);
            freq_sample[i]  = read_2bytes(input);
        }
    } else {
        exit(0);
    }
    crc_32 = read_4bytes(input);
}

void chunk_read_ztxt(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    int i;
    uint8_t k;
    uint32_t crc_32;

    printf("chunk:%s\n", chunk);
    for(i = 0; i < size; i++) {
        fread(&k, 1, 1, input);
    }
    crc_32 = read_4bytes(input);
}

void chunk_read_not_found(FILE *input, char *chunk, uint8_t **output_stream, PNG_INFO *png_info, uint32_t size, uint8_t **png_image_data)
{
    printf("size:%d\n", size);
    printf("chunk:%s\n", chunk);
    printf("Don't support chunk. Exit!\n");
    exit(0);
}

void chunk_read(FILE *input, uint8_t **output_stream, uint8_t **png_image_data, PNG_INFO *png_info)
{
    uint32_t size;
    char chunk[5];
    int i;
    struct chunk_read_info chunk_read_table[] =
    {
        {"IHDR",      chunk_read_ihdr},
        {"IDAT",      chunk_read_idat},
        {"PLTE",      chunk_read_plte},
        {"IEND",      chunk_read_iend},
        {"gAMA",      chunk_read_gama},
        {"sRGB",      chunk_read_srgb},
        {"cHRM",      chunk_read_chrm},
        {"pHYs",      chunk_read_phys},
        {"vpAg",      chunk_read_vpag},
        {"tEXt",      chunk_read_text},
        {"tRNS",      chunk_read_trns},
        {"sBIT",      chunk_read_sbit},
        {"bKGD",      chunk_read_bkgd},
        {"hIST",      chunk_read_hist},
        {"eXIf",      chunk_read_exif},
        {"iTXt",      chunk_read_itxt},
        {"sPLT",      chunk_read_splt},
        {"zTXt",      chunk_read_ztxt},
        {&(chunk[0]), chunk_read_not_found},
    };

    png_info->alpha_index = NULL;
    png_info->alpha_gray  = NULL;
    png_info->alpha_red   = NULL;
    png_info->alpha_green = NULL;
    png_info->alpha_blue  = NULL;
    png_info->tRNS_size   = 0;
    png_info->gamma       = 100000;
    png_info->idat_size   = 0;
    png_info->flag        = 0;

    *png_image_data = NULL;

    make_crc_table();

    do {
        size = read_4bytes(input);
        fread(&chunk[0], 1, 1, input);
        fread(&chunk[1], 1, 1, input);
        fread(&chunk[2], 1, 1, input);
        fread(&chunk[3], 1, 1, input);
        chunk[4] = '\0';

        for(i = 0; i < sizeof(chunk_read_table)/sizeof(chunk_read_table[0]); i++) {
            if(strcmp(chunk_read_table[i].name, chunk) == 0) {
                chunk_read_table[i].func(input, &(chunk[0]), output_stream, png_info, size, png_image_data);
                break;
            }
        }
    } while(png_info->flag == 0);

    if(*png_image_data == NULL) {
        printf("missing IDAT chunk. Exit!\n");
        exit(0);
    }
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
    cm    = cmf & 0x0F;
    cinfo = (cmf & 0xF0) >> 4;
    printf("cm %02xh, cinfo %02xh\n", cm, cinfo);

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
    fdict = (flg & 0x20) >> 5;
    printf("fdict %d\n", fdict);

    if(((((uint16_t)cmf << 8) + flg) % 31) != 0) {
        printf("zlib header broken\n");
        exit(0);
    }

    if(fdict == 1) {
        dictid =               png_image_data[*(byte_index  )];
        dictid = dictid << 8 | png_image_data[*(byte_index+1)];
        dictid = dictid << 8 | png_image_data[*(byte_index+2)];
        dictid = dictid << 8 | png_image_data[*(byte_index+3)];
        *byte_index += 4;
        printf("dictid\n");
        exit(0);
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
        assert(*bit_index < 8);
        code |= one_bit_read(png_image_data, byte_index, bit_index);
        code_len += 1;
        for(i = 0; i < len; i++) {
            if(huffman_tree[i].len == code_len && huffman_tree[i].code == code) {
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
    int len;

    for(i = 0; i < tree_size; i++) {
        tree[i].len = 0;
        tree[i].code = 0;
    }

    for(i = 0; i < 288; i++) {
        next_code[i] = 0;
    }

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

    for (i = 0; i < tree_size; i++) {
        len = tree[i].len;
        if (len != 0) {
            tree[i].code = next_code[len];
            next_code[len]++;
        } else {
            tree[i].code = 0;
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
    int hclens_index_table[19] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };
    int bit_read_table[19] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 2, 3, 7
    };
    int repeat_base[19] = {
        1, 1, 1, 1, 1, 1, 1, 1,  1, 1,
        1, 1, 1, 1, 1, 1, 3, 3, 11
    };
    int last_id_table[19] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, /* value */
        -1,  /* last_id */
        0,   /* 17 */
        0    /* 18 */
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
        value = decode_huffman(png_image_data, byte_index, bit_index, &(tree[0]), 19);
        repeat = bit_read(png_image_data, byte_index, bit_index, bit_read_table[value]) + repeat_base[value];
        for(i = 0; i < repeat; i++) {
            id[id_index] = last_id_table[value];
            id_index += 1;
        }
        last_id_table[16] = id[id_index-1];
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

        if(png_info->color_type != 3) {
            switch(png_info->bps) {
                case 1:
                    data *= 255 / (2-1);
                    break;
                case 2:
                    data *= 255 / (4-1);
                    break;
                case 4:
                    data *= 255 / (16-1);
                    break;
                case 8:
                default:
                    data *= 255 / (256-1);
                    break;
            }
        }
    } else {
        printf("bps16 [%d][%d] : %3d, %3d : ", *write_byte_index, *index, output_stream[*write_byte_index], output_stream[*write_byte_index+1]);

        data = bit_read(output_stream, write_byte_index, index, 8);
        data = data << 8 | bit_read(output_stream, write_byte_index, index, 8);
    }

    return data;
}

RGBTRIPLE get_color(uint8_t *output_stream, int *write_byte_index, PNG_INFO *png_info, int *index)
{
    RGBTRIPLE c;
    int data;

    if(png_info->bps != 16) {
        c.rgbtAlpha = 0xFF;
    } else {
        c.rgbtAlpha = 0xFFFF;
    }

    if(png_info->color_type == 0) {
        data = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtRed   = data;
        c.rgbtGreen = data;
        c.rgbtBlue  = data;
        if(png_info->alpha_gray != NULL) {
            c.rgbtAlpha = png_info->alpha_gray[data];
        }
    } else if(png_info->color_type == 2) {
        c.rgbtRed   = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtGreen = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtBlue  = get_color_data(output_stream, write_byte_index, png_info, index);
    } else if(png_info->color_type == 3) {
        data = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtRed   = (png_info->color_palette)[data].rgbtRed;
        c.rgbtGreen = (png_info->color_palette)[data].rgbtGreen;
        c.rgbtBlue  = (png_info->color_palette)[data].rgbtBlue;
        if(png_info->alpha_index != NULL) {
            if(data < png_info->tRNS_size) {
                c.rgbtAlpha = png_info->alpha_index[data];
            } else {
                c.rgbtAlpha = 255;
            }
        }
    } else if(png_info->color_type == 4) {
        data = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtRed   = data;
        c.rgbtGreen = data;
        c.rgbtBlue  = data;
        data = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtAlpha = data;
    } else if(png_info->color_type == 6) {
        c.rgbtRed   = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtGreen = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtBlue  = get_color_data(output_stream, write_byte_index, png_info, index);
        c.rgbtAlpha = get_color_data(output_stream, write_byte_index, png_info, index);
    } else {
        c.rgbtRed   = 0;
        c.rgbtGreen = 0;
        c.rgbtBlue  = 0;
        c.rgbtAlpha = 0xFFFF;
    }

    if(png_info->gamma != 100000) {
        c.rgbtRed   = (uint8_t)(255.0 * pow((c.rgbtRed   / 255.0), (100000.0/png_info->gamma)));
        c.rgbtGreen = (uint8_t)(255.0 * pow((c.rgbtGreen / 255.0), (100000.0/png_info->gamma)));
        c.rgbtBlue  = (uint8_t)(255.0 * pow((c.rgbtBlue  / 255.0), (100000.0/png_info->gamma)));
    }

    if(png_info->bps == 16) {
        c.rgbtRed   >>= 8;
        c.rgbtGreen >>= 8;
        c.rgbtBlue  >>= 8;
        c.rgbtAlpha >>= 8;
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

void filter_interlace(uint8_t *output_stream, int *write_byte_index, PNG_INFO *png_info, int pass)
{
    int i;
    int j;
    int k;
    uint8_t up_byte;
    uint8_t left_byte;
    uint8_t upper_left_byte;
    uint8_t w[] = {1, 0, 3, 1, 2, 0, 4};
    uint8_t start_y[7] = {0, 0, 4, 0, 2, 0, 1};
    uint8_t start_x[7] = {0, 4, 0, 2, 0, 1, 0};
    uint8_t step_y[7]  = {8, 8, 8, 4, 4, 2, 2};
    uint8_t step_x[7]  = {8, 8, 4, 4, 2, 2, 1};
    int tmp;
    int count;

    if(png_info->bps != 16) {
        tmp = png_info->width / (8 / png_info->bps);
        if((png_info->width % (8/png_info->bps)) != 0) {
            tmp += 1;
        } else {
            ;
        }
    }

    for(i = start_y[pass]; i < png_info->height; i += step_y[pass]) {
        if(start_x[pass] >= png_info->width) {
            break;
        }
        printf("%d, %d\n", *write_byte_index, output_stream[*write_byte_index]);
        if(output_stream[*write_byte_index] == NONE) {
            *write_byte_index += 1;
            for(j = start_x[pass]; j < png_info->width; j += step_x[pass]) {
                count = 0;
                if(png_info->bps != 16) {
                    count += w[png_info->color_type];
                } else {
                    count += w[png_info->color_type] * 2;
                }
                if(png_info->bps != 16) {
                    *write_byte_index += count / (8 / png_info->bps);
                    if((count % (8/png_info->bps)) != 0) {
                        *write_byte_index += 1;
                    }
                } else {
                    *write_byte_index += count;
                }
            }
        } else if(output_stream[*write_byte_index] == SUB) {
            *write_byte_index += 1;
            for(j = start_x[pass]; j < png_info->width; j += step_x[pass]) {
                count = 0;
                if(png_info->bps != 16) {
                    for(k = 0; k < w[png_info->color_type]; k++) {
                        if(j == start_x[pass]) {
                            output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + 0) % 256;
                        } else {
                            output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + output_stream[*write_byte_index+k-w[png_info->color_type]]) % 256;
                        }
                    }
                    count += w[png_info->color_type];
                } else {
                    for(k = 0; k < w[png_info->color_type]*2; k++) {
                        if(j == start_x[pass]) {
                            output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + 0) % 256;
                        } else {
                            output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + output_stream[*write_byte_index+k-w[png_info->color_type]*2]) % 256;
                        }
                    }
                    count += w[png_info->color_type] * 2;
                }
                if(png_info->bps != 16) {
                    *write_byte_index += count / (8 / png_info->bps);
                    if((count % (8/png_info->bps)) != 0) {
                        *write_byte_index += 1;
                    }
                } else {
                    *write_byte_index += count;
                }
            }
        }
        else if(output_stream[*write_byte_index] == UP) {
            *write_byte_index += 1;
            for(j = start_x[pass]; j < png_info->width; j += step_x[pass]) {
                count = 0;
                int tmp = (png_info->width-start_x[pass]) / step_x[pass];
                if(((png_info->width-start_x[pass])%step_x[pass]) != 0) {
                    tmp += 1;
                }
                if(png_info->bps != 16) {
                    for(k = 0; k < w[png_info->color_type]; k++) {
                        if(i == start_y[pass]) {
                            up_byte = 0;
                        } else {
                            up_byte = output_stream[*write_byte_index + k - (tmp*w[png_info->color_type]+1)];
                        }
                        output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + up_byte) % 256;
                    }
                    count += w[png_info->color_type];
                } else {
                    for(k = 0; k < w[png_info->color_type]*2; k++) {
                        if(i == start_y[pass]) {
                            up_byte = 0;
                        } else {
                            up_byte = output_stream[*write_byte_index + k - (tmp*w[png_info->color_type]*2+1)];
                        }
                        output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + up_byte) % 256;
                    }
                    count += w[png_info->color_type] * 2;
                }
                if(png_info->bps != 16) {
                    *write_byte_index += count / (8 / png_info->bps);
                    if((count % (8/png_info->bps)) != 0) {
                        *write_byte_index += 1;
                    }
                } else {
                    *write_byte_index += count;
                }
            }
        } else if(output_stream[*write_byte_index] == AVERAGE) {
            *write_byte_index += 1;
            for(j = start_x[pass]; j < png_info->width; j += step_x[pass]) {
                count = 0;
                int tmp = (png_info->width-start_x[pass]) / step_x[pass];
                if(((png_info->width-start_x[pass])%step_x[pass]) != 0) {
                    tmp += 1;
                }
                if(png_info->bps != 16) {
                    for(k = 0; k < w[png_info->color_type]; k++) {
                        if(i == start_y[pass]) {
                            up_byte = 0;
                        } else {
                            up_byte = output_stream[*write_byte_index + k - (tmp*w[png_info->color_type]+1)];
                        }
                        if(j == start_x[pass]) {
                            output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + (up_byte)/2) % 256;
                        } else {
                            output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + (output_stream[*write_byte_index+k] + up_byte)/2) % 256;
                        }
                    }
                    count += w[png_info->color_type];
                } else {
                    for(k = 0; k < w[png_info->color_type]*2; k++) {
                        if(i == start_y[pass]) {
                            up_byte = 0;
                        } else {
                            up_byte = output_stream[*write_byte_index + k - (tmp*w[png_info->color_type]*2+1)];
                        }
                        if(j == start_x[pass]) {
                            output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + (up_byte)/2) % 256;
                        } else {
                            output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + (output_stream[*write_byte_index+k-w[png_info->color_type]*2] + up_byte)/2) % 256;
                        }
                    }
                    count += w[png_info->color_type] * 2;
                }
                if(png_info->bps != 16) {
                    *write_byte_index += count / (8 / png_info->bps);
                    if((count % (8/png_info->bps)) != 0) {
                        *write_byte_index += 1;
                    }
                } else {
                    *write_byte_index += count;
                }
            }
        } else if(output_stream[*write_byte_index] == PAETH) {
            *write_byte_index += 1;

            for(j = start_x[pass]; j < png_info->width; j += step_x[pass]) {
                count = 0;
                int tmp = (png_info->width-start_x[pass]) / step_x[pass];
                if(((png_info->width-start_x[pass])%step_x[pass]) != 0) {
                    tmp += 1;
                }
                if(png_info->bps != 16) {
                    for(k = 0; k < w[png_info->color_type]; k++) {
                        if(i == start_y[pass]) {
                            up_byte = 0;
                        } else {
                            up_byte = output_stream[*write_byte_index + k - (tmp*w[png_info->color_type]+1)];
                        }
                        if(j == start_x[pass]) {
                            left_byte = 0;
                        } else {
                            left_byte = output_stream[*write_byte_index + k - w[png_info->color_type]];
                        }
                        if(i == start_y[pass] || j == start_x[pass]) {
                            upper_left_byte = 0;
                        } else {
                            upper_left_byte = output_stream[*write_byte_index + k - (tmp*w[png_info->color_type]+1) - w[png_info->color_type]];
                        }
                        output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + paeth_predictor(left_byte, up_byte, upper_left_byte)) % 256;
                    }
                    count += w[png_info->color_type];
                } else {
                    for(k = 0; k < w[png_info->color_type]*2; k++) {
                        if(i == start_y[pass]) {
                            up_byte = 0;
                        } else {
                            up_byte = output_stream[*write_byte_index + k - (tmp*w[png_info->color_type]*2+1)];
                        }
                        if(j == start_x[pass]) {
                            left_byte = 0;
                        } else {
                            left_byte = output_stream[*write_byte_index + k - w[png_info->color_type]*2];
                        }
                        if(i == start_y[pass] || j == start_x[pass]) {
                            upper_left_byte = 0;
                        } else {
                            upper_left_byte = output_stream[*write_byte_index + k - (tmp*w[png_info->color_type]*2+1) - w[png_info->color_type]*2];
                        }

                        output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + paeth_predictor(left_byte, up_byte, upper_left_byte)) % 256;
                    }
                    count += w[png_info->color_type] * 2;
                }
                if(png_info->bps != 16) {
                    *write_byte_index += count / (8 / png_info->bps);
                    if((count % (8/png_info->bps)) != 0) {
                        *write_byte_index += 1;
                    }
                } else {
                    *write_byte_index += count;
                }
            }
        } else {
            printf("undefined filter type\n");
            exit(0);
        }
    }
}

void interlace(uint8_t *output_stream, int *write_byte_index, RGBTRIPLE ***image_data, PNG_INFO *png_info, int pass)
{
    int i;
    int j;
    int k;
    uint8_t start_y[7] = {0, 0, 4, 0, 2, 0, 1};
    uint8_t start_x[7] = {0, 4, 0, 2, 0, 1, 0};
    uint8_t step_y[7]  = {8, 8, 8, 4, 4, 2, 2};
    uint8_t step_x[7]  = {8, 8, 4, 4, 2, 2, 1};

    printf("[%d] %d\n", pass, output_stream[*write_byte_index]);
    k = 0;
    for(i = start_y[pass]; i < png_info->height; i += step_y[pass]) {
        if(start_x[pass] >= png_info->width) {
            break;
        }
        *write_byte_index += 1;
        for(j = start_x[pass]; j < png_info->width; j += step_x[pass]) {
            (*image_data)[i][j] = get_color(output_stream, write_byte_index, png_info, &k);
        }
        if(k != 0) {
            *write_byte_index += 1;
            k = 0;
        }
        printf("%d %d %d %d\n", (*image_data)[i][j].rgbtRed, (*image_data)[i][j].rgbtGreen, (*image_data)[i][j].rgbtBlue, (*image_data)[i][j].rgbtAlpha);
    }
}

void filter(uint8_t *output_stream, int i, int *write_byte_index, PNG_INFO *png_info)
{
    int j;
    int k;
    uint8_t up_byte;
    uint8_t left_byte;
    uint8_t tmp;
    uint8_t width;
    uint8_t upper_left_byte;
    uint8_t w[] = {1, 0, 3, 1, 2, 0, 4};

    printf("[%d( %d )] %d\n", i, *write_byte_index, output_stream[*write_byte_index]);
    if(png_info->bps != 16) {
        tmp = png_info->width / (8 / png_info->bps);
        if((png_info->width % (8/png_info->bps)) != 0) {
            width = tmp + 1;
        } else {
            width = tmp;
        }
    } else {
        ;
    }

    if(png_info->bps != 16) {
        tmp = 1;
    } else {
        tmp = 2;
    }

    if(output_stream[*write_byte_index] == NONE) {
        *write_byte_index += 1;
        *write_byte_index += w[png_info->color_type] * tmp * width;
    } else if(output_stream[*write_byte_index] == SUB) {
        *write_byte_index += 1;
        for(j = 0; j < width; j++) {
            for(k = 0; k < w[png_info->color_type]*tmp; k++) {
                if(j == 0) {
                    output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + 0) % 256;
                } else {
                    output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + output_stream[*write_byte_index+k-w[png_info->color_type]*tmp]) % 256;
                }
            }
            *write_byte_index += w[png_info->color_type] * tmp;
        }
    } else if(output_stream[*write_byte_index] == UP) {
        *write_byte_index += 1;
        for(j = 0; j < width; j++) {
            for(k = 0; k < w[png_info->color_type]*tmp; k++) {
                if(i == 0) {
                    up_byte = 0;
                } else {
                    up_byte = output_stream[*write_byte_index + k - (width*w[png_info->color_type]*tmp+1)];
                }
                output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + up_byte) % 256;
            }
            *write_byte_index += w[png_info->color_type] * tmp;
        }
    } else if(output_stream[*write_byte_index] == AVERAGE) {
        *write_byte_index += 1;
        for(j = 0; j < width; j++) {
            if(png_info->bps != 16) {
                for(k = 0; k < w[png_info->color_type]; k++) {
                    if(i == 0) {
                        up_byte = 0;
                    } else {
                        up_byte = output_stream[*write_byte_index + k - (width*w[png_info->color_type]+1)];
                    }
                    if(j == 0) {
                        output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + (up_byte)/2) % 256;
                    } else {
                        output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + (output_stream[*write_byte_index+k-w[png_info->color_type]] + up_byte)/2) % 256;
                    }
                }
                *write_byte_index += w[png_info->color_type];
            } else {
                for(k = 0; k < w[png_info->color_type]*2; k++) {
                    if(i == 0) {
                        up_byte = 0;
                    } else {
                        up_byte = output_stream[*write_byte_index + k - (width*w[png_info->color_type]*2+1)];
                    }
                    if(j == 0) {
                        output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + (up_byte)/2) % 256;
                    } else {
                        output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + (output_stream[*write_byte_index+k-w[png_info->color_type]*2] + up_byte)/2) % 256;
                    }
                }
                *write_byte_index += w[png_info->color_type] * 2;
            }
        }
    } else if(output_stream[*write_byte_index] == PAETH) {
        *write_byte_index += 1;

        for(j = 0; j < width; j++) {
            if(png_info->bps != 16) {
                printf("[%d] ", j);
                for(k = 0; k < w[png_info->color_type]; k++) {
                    if(i == 0) {
                        up_byte = 0;
                    } else {
                        up_byte = output_stream[*write_byte_index + k - (width*w[png_info->color_type]+1)];
                    }
                    if(j == 0) {
                        left_byte = 0;
                    } else {
                        left_byte = output_stream[*write_byte_index + k - w[png_info->color_type]];
                    }
                    if(i == 0 || j == 0) {
                        upper_left_byte = 0;
                    } else {
                        upper_left_byte = output_stream[*write_byte_index + k - (width*w[png_info->color_type]+1) - w[png_info->color_type]];
                    }

                    output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + paeth_predictor(left_byte, up_byte, upper_left_byte)) % 256;
                    printf("%d\n", output_stream[*write_byte_index+k]);
                }
                printf("\n");
                *write_byte_index += w[png_info->color_type];
            } else {
                printf("[%d] ", j);
                for(k = 0; k < w[png_info->color_type]*2; k++) {
                    if(i == 0) {
                        up_byte = 0;
                    } else {
                        up_byte = output_stream[*write_byte_index + k - (width*w[png_info->color_type]*2+1)];
                    }
                    if(j == 0) {
                        left_byte = 0;
                    } else {
                        left_byte = output_stream[*write_byte_index + k - w[png_info->color_type]*2];
                    }
                    if(i == 0 || j == 0) {
                        upper_left_byte = 0;
                    } else {
                        upper_left_byte = output_stream[*write_byte_index + k - (width*w[png_info->color_type]*2+1) - w[png_info->color_type]*2];
                    }

                    output_stream[*write_byte_index+k] = (output_stream[*write_byte_index+k] + paeth_predictor(left_byte, up_byte, upper_left_byte)) % 256;
                    printf("%d\n", output_stream[*write_byte_index+k]);
                }
                printf("\n");
                *write_byte_index += w[png_info->color_type]*2;
            }
        }
    } else {
        printf("undefined filter type\n");
        exit(0);
    }
}

void line(uint8_t *output_stream, RGBTRIPLE ***image_data, PNG_INFO *png_info)
{
    int i;
    int j;
    int write_bit_index;
    int write_byte_index;

    write_byte_index = 0;
    for(i = 0; i < png_info->height; i++) {
        printf("[%d] %d\n", i, output_stream[write_byte_index]);
        write_byte_index += 1;
        write_bit_index = 0;
        for(j = 0; j < png_info->width; j++) {
            (*image_data)[i][j] = get_color(output_stream, &write_byte_index, png_info, &write_bit_index);
        }
        if(write_bit_index != 0) {
            write_byte_index += 1;
        }
    }
}

void decode_huffman_codes(uint8_t *png_image_data, int *byte_index, int *bit_index, struct tree *tree, int lit, uint8_t *output_stream, int *write_byte_index, struct tree *dtree, int dist)
{
    int i;
    int value;
    int len_bit;
    uint16_t len;
    uint16_t dlen;
    int len_bit_value;
    int dist_bit;
    int dist_bit_value;
    int len_block_bit[29] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, // 257-266
        1, 1, 2, 2, 2, 2, 3, 3, 3, 3, // 267-276
        4, 4, 4, 4, 5, 5, 5, 5, 0     // 277-285
    };
    int len_block[29] = {
        3,  4,  5,  6,   7,   8,   9,   10,  11,  13,
        15, 17, 19, 23,  27,  31,  35,  43,  51,  59,
        67, 83, 99, 115, 131, 163, 195, 227, 258
    };
    int dist_block_bit[30] = {
        0, 0, 0,  0,  1,  1,  2,  2,  3,  3,
        4, 4, 5,  5,  6,  6,  7,  7,  8,  8,
        9, 9, 10, 10, 11, 11, 12, 12, 13, 13
    };
    int dist_block[30] = {
        1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
        33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
        1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
    };

    /* loop (until end of block code recognized) */
    printf("(%d)(%d)\n", *byte_index, *bit_index);
    do {
        value = decode_huffman(png_image_data, byte_index, bit_index, tree, lit);

        //printf("value = %d\n", value);
        if(value < 256) {
            /* copy value (literal byte) to output stream */
            output_stream[*write_byte_index] = value;
            printf("write_byte:%d = %d  (%d)(%d)\n", *write_byte_index, value, *byte_index, *bit_index);
            *write_byte_index += 1;
        } else if(value == END_OF_BLOCK) {
            printf("%d\n", *write_byte_index);
            break;
        } else {/* (value = 257..285) */
            printf("len value = %d (%d)\n", value - 257, value);

            assert(value >= 257 && value <= 285);
            len_bit = len_block_bit[value - 257];

            len_bit_value = 0;
            if(len_bit != 0) {
                len_bit_value = bit_read(png_image_data, byte_index, bit_index, len_bit);
                printf("len_bit_vlaue = %d  (%d)(%d)\n", len_bit_value, *byte_index, *bit_index);
            }
            assert(value >= 257 && value <= 285);
            len  = len_block[value-257];
            len += len_bit_value;

            /* decode distance from input stream */
            value = decode_huffman(png_image_data, byte_index, bit_index, dtree, dist);
            printf("dist value = %d (%d)(%d)\n", value, *byte_index, *bit_index);

            assert(value >= 0 && value <= 29);
            dist_bit = dist_block_bit[value];

            dist_bit_value = 0;
            if(dist_bit != 0) {
                dist_bit_value = bit_read(png_image_data, byte_index, bit_index, dist_bit);
                printf("dist_bit_value = %d  (%d)(%d)\n", dist_bit_value, *byte_index, *bit_index);
            }
            assert(value >= 0 && value <= 29);
            dlen  = dist_block[value];
            dlen += dist_bit_value;

            /* move backwards distance bytes in the output stream, and copy length bytes from this position to the output stream. */
            for(i = 0; i < len; i++) {
                printf("write_byte:%d = %d (%d=%d-%d)\n", *write_byte_index, output_stream[*write_byte_index-dlen], *write_byte_index-dlen, *write_byte_index, dlen);
                output_stream[*write_byte_index] = output_stream[*write_byte_index-dlen];
                *write_byte_index += 1;
            }
        }
    } while(1);
}

void decode_codes(uint8_t *png_image_data, int byte_index, uint8_t *output_stream, int *write_byte_index)
{
    uint16_t len;
    uint16_t nlen;
    int i;

    /* skip any remaining bits in current partially processed byte */
    byte_index++;

    /* read LEN and NLEN (see next section) */
    len = (png_image_data[byte_index] << 8) | png_image_data[byte_index+1];
    byte_index += 2;
    nlen = (png_image_data[byte_index] << 8) | png_image_data[byte_index+1];
    byte_index += 2;

    /* copy LEN bytes of data to output */
    for(i = 0; i < len; i++) {
        output_stream[*write_byte_index] = png_image_data[byte_index];
        *write_byte_index += 1;
        byte_index += 1;
    }
}

void decode_fixed_huffman_codes(uint8_t *png_image_data, int byte_index, int bit_index, uint8_t *output_stream, int *write_byte_index)
{
    int lit;
    int dist;
    struct tree tree[288];
    struct tree dtree[32];

    decompress_fixed_huffman_codes(png_image_data, &byte_index, &bit_index, &lit, &dist, tree, dtree);
    decode_huffman_codes(png_image_data, &byte_index, &bit_index, tree, lit, output_stream, write_byte_index, dtree, dist);
}

void decode_dynamic_huffman_codes(uint8_t *png_image_data, int byte_index, int bit_index, uint8_t *output_stream, int *write_byte_index)
{
    int lit;
    int dist;
    struct tree tree[288];
    struct tree dtree[32];

    decompress_dynamic_huffman_codes(png_image_data, &byte_index, &bit_index, &lit, &dist, tree, dtree);
    decode_huffman_codes(png_image_data, &byte_index, &bit_index, tree, lit, output_stream, write_byte_index, dtree, dist);
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
    PNG_INFO png_info;

    printf("PNG\n");

    fseek(input, 8, SEEK_CUR);

    chunk_read(input, &output_stream, &png_image_data, &png_info);

    bit_index = 0;
    byte_index = 0;
    write_byte_index = 0;

    printf("%d\n", png_info.idat_size);

    read_zlib_header(png_image_data, &byte_index, &bit_index);
    do {
        /* read block header from input stream. */
        bfinal = bit_read(png_image_data, &byte_index, &bit_index, 1);
        btype  = bit_read(png_image_data, &byte_index, &bit_index, 2);
        printf("bfinal %02x, btype %02x\n", bfinal, btype);

        if(btype == 0x00) {
            decode_codes(png_image_data, byte_index, output_stream, &write_byte_index);
        } else if(btype == 0x01) {
            decode_fixed_huffman_codes(png_image_data, byte_index, bit_index, output_stream, &write_byte_index);
        } else if(btype == 0x02) {
            decode_dynamic_huffman_codes(png_image_data, byte_index, bit_index, output_stream, &write_byte_index);
        }
    } while(bfinal == 0);

    printf("write_byte : %d\n", write_byte_index);
    (*image_data) = (RGBTRIPLE **)malloc(sizeof(RGBTRIPLE *) * png_info.height);
    for(i = 0; i < png_info.height; i++) {
        (*image_data)[i] = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * png_info.width);
    }

    if(png_info.interlace_type == 0) {
        write_byte_index = 0;
        for(i = 0; i < png_info.height; i++) {
            filter(output_stream, i, &write_byte_index, &png_info);
        }
        line(output_stream, image_data, &png_info);
    } else {
        write_byte_index = 0;
        for(i = 0; i < 7; i++) {
            printf("-- %d --\n", i);
            filter_interlace(output_stream, &write_byte_index, &png_info, i);
        }
        write_byte_index = 0;
        for(i = 0; i < 7; i++) {
            interlace(output_stream, &write_byte_index, image_data, &png_info, i);
        }
    }

    image_info->height   = png_info.height;
    image_info->width    = png_info.width;
    image_info->fileSize = png_info.height*png_info.width*3 + 54;

    free((void *)output_stream);
}
