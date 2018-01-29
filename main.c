#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "common.h"
#include "bitmap.h"
#include "average_filter.h"
#include "gaussian_filter.h"
#include "bilateral_filter.h"
#include "prewitt_filter.h"
#include "sobel_filter.h"
#include "canny_edge_detector.h"
#include "LoG_filter.h"

enum {
    NONE,
    BITMAP,
    PNG,
}; 

int check_file_format(FILE *input)
{
    uint8_t read_byte;

    fseek(input, 0, SEEK_SET);

    fread(&read_byte, 1, 1, input);
    if(read_byte == 'B') {
        fread(&read_byte, 1, 1, input);
        if(read_byte == 'M') {
            fseek(input, 0, SEEK_SET);
            return BITMAP;
        } else {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
    } else if(read_byte == 0x89) {
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'P') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'N') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'G') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != '\r') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != '\n') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != 0x1a) {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != '\n') {
            fseek(input, 0, SEEK_SET);
            return NONE;
        }
        fseek(input, 0, SEEK_SET);
        return PNG;
    }

    fseek(input, 0, SEEK_SET);
    return NONE;
}

uint16_t read_2bytes(FILE *input)
{
    uint8_t byte;
    uint16_t result;

    fread(&byte, 1, 1, input);
    result = byte;
    fread(&byte, 1, 1, input);
    result = (result << 8) | byte;

    return result;
}
uint32_t read_4bytes(FILE *input)
{
    uint8_t byte;
    uint32_t result;

    fread(&byte, 1, 1, input);
    result = byte;
    fread(&byte, 1, 1, input);
    result = (result << 8) | byte;
    fread(&byte, 1, 1, input);
    result = (result << 8) | byte;
    fread(&byte, 1, 1, input);
    result = (result << 8) | byte;

    return result;
}

int bit_read(uint8_t byte, int bit_pos, int bit_len)
{
    uint8_t pattern[8] = {
        //0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
    };

    //byte <<= bit_pos;
    //byte &= pattern[bit_len-1];
    //byte >>= (8 - bit_len);
    byte >>= bit_pos;
    byte &= pattern[bit_len-1];

    return byte;
}

int main(int argc, char *argv[])
{
    FILE *input;
    FILE *output;
    IMAGEINFO image_info;
    RGBTRIPLE **image_data;
    RGBTRIPLE **output_image_data;
    int i;
    int file_format;
    uint8_t *png_image_data;

    if(argc <= 1) {
        printf("filename\n");
        return 0;
    }

    input = fopen(argv[1], "rb");
    if(input == NULL) {
        printf("don't open file\n");
        return 0;
    }

    file_format = check_file_format(input);
    if(file_format == NONE) {
        printf("unsupported file format\n");
        return 0;
    } else if(file_format == BITMAP) {
        decode_bitmap(input, &image_info, &image_data);
        printf("%d %d %d\n", image_info.fileSize, image_info.width, image_info.height);
    } else if(file_format == PNG) {
        uint8_t byte;
        uint32_t size;
        uint32_t width;
        uint32_t height;
        uint8_t bps;
        uint8_t color_type;
        uint8_t compress_type;
        uint8_t filter_type;
        uint8_t interlace_type;
        uint32_t crc_32;
        char chunk[5];
        int bit_index;
        int byte_index;
        int bfinal;
        int btype;
        uint16_t len;
        uint16_t nlen;
        int flag;
        RGBTRIPLE *color_palette;
        uint8_t cmf;
        uint8_t cm;
        uint8_t cinfo;
        uint32_t window_size;
        int flg;
        uint8_t fdict;
        uint32_t dictid;

        printf("PNG\n");

        for(i = 0; i < 8; i++) {
            fread(&byte, 1, 1, input);
        }

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
                flag = 0;
            } else if(strcmp(chunk, "IDAT") == 0) {
                png_image_data = (uint8_t *)malloc(sizeof(uint8_t) * size);
                fread(png_image_data, 1, size, input);
                crc_32 = read_4bytes(input);
                printf("size:%d\n", size);
                printf("chunk:%s\n", chunk);
                printf("crc-32: %xh\n", crc_32);
            } else if(strcmp(chunk, "PLTE") == 0) {
                color_palette = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * size);

                for(i = 0; i < size/3; i++) {
                    fread(&(color_palette[i].rgbtRed),   1, 1, input);
                    fread(&(color_palette[i].rgbtGreen), 1, 1, input);
                    fread(&(color_palette[i].rgbtBlue),  1, 1, input);
                }
                crc_32 = read_4bytes(input);

                printf("size:%d\n", size);
                printf("chunk:%s\n", chunk);
                for(i = 0; i < size/3; i++) {
                    printf("[%d] : %d %d %d\n", i+1, color_palette[i].rgbtRed, color_palette[i].rgbtGreen, color_palette[i].rgbtBlue);
                }
                printf("crc-32: %xh\n", crc_32);
            } else if(strcmp(chunk, "IEND") == 0) {
                crc_32 = read_4bytes(input);
                printf("size:%d\n", size);
                printf("chunk:%s\n", chunk);
                flag = 1;
            } else {
                printf("size:%d\n", size);
                printf("chunk:%s\n", chunk);
                return 0;
            }
        } while(flag == 0);

        bit_index = 0;
        byte_index = 0;
        do {
            cmf = png_image_data[byte_index];
            byte_index += 1;
            printf("%02xh\n", cmf);
            cm = cmf & 0x0F;
            cinfo = (cmf & 0xF0) >> 4;
            printf("%02xh, %02xh\n", cm, cinfo);

            if(cm != 8) {
                printf("not deflate\n");
                return 0;
            }
            if(cinfo > 7) {
                printf("CINFO above 7 are not allowed in this version of the specification\n");
                return 0;
            }
            window_size = pow(2, cinfo + 8);

            flg = png_image_data[byte_index];
            byte_index += 1;
            fdict = (flg & 0x10) >> 5;
            printf("fdict %d\n", fdict);

            if(fdict == 1) {
                dictid = png_image_data[byte_index];
                byte_index += 1;
                dictid = dictid << 8 | png_image_data[byte_index];
                byte_index += 1;
                dictid = dictid << 8 | png_image_data[byte_index];
                byte_index += 1;
                dictid = dictid << 8 | png_image_data[byte_index];
                byte_index += 1;
            }

            /* read block header from input stream. */
            bfinal = bit_read(png_image_data[byte_index], bit_index, 1);
            bit_index += 1;
            btype = bit_read(png_image_data[byte_index], bit_index, 2);

            printf("%02x %02x\n", bfinal, btype);
#if 0
            if(btype == 0x00) {
                /* skip any remaining bits in current partially processed byte */
                printf("%02x:%02x:%02x:%02x:%02x:%02x\n", png_image_data[0], png_image_data[1], png_image_data[2], png_image_data[3], png_image_data[4], png_image_data[5]);
                byte_index++;
                /* read LEN and NLEN (see next section) */
                len = (png_image_data[byte_index] << 8) | png_image_data[byte_index+1];
                byte_index += 2;
                nlen = (png_image_data[byte_index] << 8) | png_image_data[byte_index+1];
                byte_index += 2;
                printf("%x : %x : %x\n", len, nlen, nlen ^ 0xFFFF);
                printf("%d : %d : %d\n", len, nlen, nlen ^ 0xFFFF);
                /* 
                    00000020  70 00 02 bd 2e 49 44 41  54 78 da ec c0 81 00 00  |p....IDA|Tx......|
                    00000030  00 00 80 a0 fd a9 17 a9  00 00 00 00 00 00 00 00  |........|........|

                    0   1   2   3   4...
                    +---+---+---+---+================================+
                    |  LEN  | NLEN  |... LEN bytes of literal data...|
                    +---+---+---+---+================================+
                   78        da        ec        c0        81
                   0111 1000 1101 1010 1110 1100 1100 0000 1000 0001

                   0001 1110 0101 1011 0011 0111 0000 0011 1000 0001
                   1e        5b        37        03        81

                   1111 0010 1101 1001 1011 1000 0001 1100 0000 1000
                   f2        d9        b8        1c        08
                */
                /*
                   copy LEN bytes of data to output
                */
            } else {
                /*
                   if compressed with dynamic Huffman codes
                   read representation of code trees (see
                   subsection below)
                   loop (until end of block code recognized)
                   decode literal/length value from input stream
                   if value < 256
                   copy value (literal byte) to output stream
                   otherwise
                   if value = end of block (256)
                   break from loop
                   otherwise (value = 257..285)
                   decode distance from input stream

                   move backwards distance bytes in the output
                   stream, and copy length bytes from this
                   position to the output stream.
                   end loop
                   while not last block
                */
            }

#endif
        } while(0);

        return 0;
    }
    fclose(input);

    output_image_data = (RGBTRIPLE **)malloc(sizeof(RGBTRIPLE *) * image_info.height);
    for(i = image_info.height-1; i >= 0; i--) {
        output_image_data[i] = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * image_info.width);
    }

    //average_filter(&output_image_data, &image_data, &image_info, 11);
    //gaussian_filter(&output_image_data, &image_data, &image_info, 3.0, 7);
    //bilateral_filter(&output_image_data, &image_data, &image_info, 25.0, 25.0, 7);
    //prewitt_filter(&output_image_data, &image_data, &image_info, 3);
    //sobel_filter(&output_image_data, &image_data, &image_info, 3);
    //canny_edge_detector(&output_image_data, &image_data, &image_info, 0.8, 7, 3);
    LoG_filter(&output_image_data, &image_data, &image_info, 1.4, 9);

    output = fopen("test", "wb");
    if(output == NULL) {
        return 0;
    }
    encode_bitmap(output, &image_info, &output_image_data);
    fclose(output);

    for(i = 0; i < image_info.height; i++) {
        free((void *)image_data[i]);
        free((void *)output_image_data[i]);
    }
    free((void *)image_data);
    free((void *)output_image_data);

    return 0;
}
