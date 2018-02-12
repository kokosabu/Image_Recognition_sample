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

struct tree {
    int len;
    int code;
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

int bit_read(uint8_t *input_stream, int byte_pos, int bit_pos, int bit_len)
{
    uint8_t pattern[8] = {
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
    };
    uint8_t byte;
    uint8_t next_byte;

    byte      = input_stream[byte_pos];
    next_byte = input_stream[byte_pos+1];

    if((8-bit_pos) >= bit_len) {
        byte >>= bit_pos;
        byte &= pattern[bit_len-1];
    } else {
        byte >>= bit_pos;
        byte &= pattern[(8-bit_pos)-1];
        next_byte &= pattern[bit_pos - (8-bit_len) - 1];
        byte = byte | ( next_byte << (8-bit_pos) );
    }

    return byte;
}

int huffman_bit_read(uint8_t *input_stream, int byte_pos, int bit_pos, int bit_len)
{
    uint8_t pattern[8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
            //0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
    };
    uint8_t byte;

    byte = input_stream[byte_pos];
    byte &= pattern[bit_pos];
    //byte &= pattern[7 - bit_pos];
    //byte >>= (7 - bit_pos);
    byte >>= bit_pos;

    //printf("pos: %d, bit: %d\n", bit_pos, byte);

    return byte;
}

/*
   clen[ 8] = 000
   clen[ 9] = 001
   clen[14] = 010
   clen[16] = 011
   clen[ 5] = 1000
   clen[ 6] = 1001
   clen[ 7] = 1010
   clen[10] = 1011
   clen[11] = 1100
   clen[12] = 1101
   clen[13] = 1110
   clen[ 3] = 11110
   clen[ 4] = 11111
   */

#if 0
void bit_write(uint8_t *stream, uint8_t byte, int bit_pos, int bit_len)
{
    uint8_t pattern[8] = {
        //0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
    };

    //byte <<= bit_pos;
    //byte &= pattern[bit_len-1];
    //byte >>= (8 - bit_len);
    *stream |= 
        byte >>= bit_pos;
    byte &= pattern[bit_len-1];

    return byte;
}
#endif

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
    uint8_t *output_stream;

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
        int write_byte_index;
        int write_byte_original;
        int bfinal;
        int btype;
        uint16_t len;
        uint16_t nlen;
        uint16_t min_len;
        uint16_t min_dlen;
        int flag;
        RGBTRIPLE *color_palette;
        uint8_t cmf;
        uint8_t cm;
        uint8_t cinfo;
        uint32_t window_size;
        int flg;
        uint8_t fdict;
        uint32_t dictid;
        int value;
        int hlit;
        int hdist;
        int hclen;
        int hclens[19];
        int clen[19];
        int bl_count[286];
        int next_code[286];
        int code;
        int code_len;
        int bits;
        int max_bits;
        struct tree tree[286];
        int liten[286];
        int lit;
        int dist;
        int disten[32];
        struct tree dtree[32];
        uint8_t *id;
        int id_index;
        int table[512];
        int lit_table[512];
        int dist_table[32];
        int repeat;
        int last_id;
        int len_bit;
        int len_bit_value;
        int dist_bit;
        int dist_bit_value;
        int hclens_index_table[19] = {
            16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
        };
        int len_block_bit[29] = {
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
            1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
            4, 4, 4, 4, 5, 5, 5, 5, 0
        };
        int len_block[29] = {
            3,  4,  5,   6,   7,   8,   9,  10,  11, 13,
            15, 17, 19,  23,  27,  31,  35,  43,  51, 59,
            67, 83, 99, 115, 131, 163, 195, 227, 258
        };
        int dist_block_bit[30] = {
            0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
            4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
            9, 9, 10, 10, 11, 11, 12, 12, 13, 13
        };
        int dist_block[30] = {
            1, 2, 3, 4, 5, 7, 9, 13, 17, 25,
            33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
            1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
        };

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

                output_stream = (uint8_t *)malloc(sizeof(uint8_t) * width * height * 3);

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
        write_byte_index = 0;
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

#if 0
            /* aaaaa */
            byte_index = 0;
            bit_index = 0;
            png_image_data[0] = 0xED;
            png_image_data[1] = 0xBD;
            png_image_data[2] = 0x07;
            png_image_data[3] = 0x60;
            png_image_data[4] = 0x1C;
            png_image_data[5] = 0x49;
            png_image_data[6] = 0x96;
            png_image_data[7] = 0x25;
            png_image_data[8] = 0x26;
            png_image_data[9] = 0x2F;
            png_image_data[10] = 0x6D;
            png_image_data[11] = 0xCA;
            png_image_data[12] = 0x7B;
            png_image_data[13] = 0x7F;
            png_image_data[14] = 0x4A;
            png_image_data[15] = 0xF5;
            png_image_data[16] = 0x4A;            
            png_image_data[17] = 0xD7;            
            png_image_data[18] = 0xE0;            
            png_image_data[19] = 0x74;            
            png_image_data[20] = 0xA1;            
            png_image_data[21] = 0x08;            
            png_image_data[22] = 0x80;            
            png_image_data[23] = 0x60;            
            png_image_data[24] = 0x13;            
            png_image_data[25] = 0x24;            
            png_image_data[26] = 0xD8; 
            png_image_data[27] = 0x90;
            png_image_data[28] = 0x40;            
            png_image_data[29] = 0x10;            
            png_image_data[30] = 0xEC;            
            png_image_data[31] = 0xC1;
            png_image_data[32] = 0x88;            
            png_image_data[33] = 0xCD;            
            png_image_data[34] = 0xE6;            
            png_image_data[35] = 0x92;            
            png_image_data[36] = 0xEC;            
            png_image_data[37] = 0x1D;            
            png_image_data[38] = 0x69;            
            png_image_data[39] = 0x47;            
            png_image_data[40] = 0x23;            
            png_image_data[41] = 0x29;            
            png_image_data[42] = 0xAB; 
            png_image_data[43] = 0x2A;          
            png_image_data[44] = 0x81;            
            png_image_data[45] = 0xCA;            
            png_image_data[46] = 0x65;            
            png_image_data[47] = 0x56;
            png_image_data[48] = 0x65;            
            png_image_data[49] = 0x5D;            
            png_image_data[50] = 0x66;            
            png_image_data[51] = 0x16;            
            png_image_data[52] = 0x40;            
            png_image_data[53] = 0xCC;            
            png_image_data[54] = 0xED;            
            png_image_data[55] = 0x9D;            
            png_image_data[56] = 0xBC;            
            png_image_data[57] = 0xF7;            
            png_image_data[58] = 0xDE; 
            png_image_data[59] = 0x7B;          
            png_image_data[60] = 0xEF;            
            png_image_data[61] = 0xBD;            
            png_image_data[62] = 0xF7;            
            png_image_data[63] = 0xDE;
            png_image_data[64] = 0x7B;            
            png_image_data[65] = 0xEF;            
            png_image_data[66] = 0xBD;            
            png_image_data[67] = 0xF7;            
            png_image_data[68] = 0xBA;            
            png_image_data[69] = 0x3B;            
            png_image_data[70] = 0x9D;            
            png_image_data[71] = 0x4E;            
            png_image_data[72] = 0x27;            
            png_image_data[73] = 0xF7;            
            png_image_data[74] = 0xDF; 
            png_image_data[75] = 0xFF;          
            png_image_data[76] = 0x3F;            
            png_image_data[77] = 0x5C;            
            png_image_data[78] = 0x66;            
            png_image_data[79] = 0x64;
            png_image_data[80] = 0x01;            
            png_image_data[81] = 0x6C;            
            png_image_data[82] = 0xF6;            
            png_image_data[83] = 0xCE;            
            png_image_data[84] = 0x4A;            
            png_image_data[85] = 0xDA;            
            png_image_data[86] = 0xC9;            
            png_image_data[87] = 0x9E;
            png_image_data[88] = 0x21;            
            png_image_data[89] = 0x80;            
            png_image_data[90] = 0xAA; 
            png_image_data[91] = 0xC8;          
            png_image_data[92] = 0x1F;            
            png_image_data[93] = 0x3F;            
            png_image_data[94] = 0x7E;            
            png_image_data[95] = 0x7C;
            png_image_data[96] = 0x1F;            
            png_image_data[97] = 0x3F;            
            png_image_data[98] = 0x22;            
            png_image_data[99] = 0x32;            
            png_image_data[100] = 0x3C;            
            png_image_data[101] = 0xFF;            
            png_image_data[102] = 0x0F;
#endif

            /* read block header from input stream. */
            bfinal = bit_read(png_image_data, byte_index, bit_index, 1);
            bit_index += 1;
            btype = bit_read(png_image_data, byte_index, bit_index, 2);
            bit_index += 2;

            printf("%02x %02x\n", bfinal, btype);

            if(btype == 0x00) {
                /* skip any remaining bits in current partially processed byte */
                //printf("%02x:%02x:%02x:%02x:%02x:%02x\n", png_image_data[0], png_image_data[1], png_image_data[2], png_image_data[3], png_image_data[4], png_image_data[5]);
                byte_index++;

                /* read LEN and NLEN (see next section) */
                len = (png_image_data[byte_index] << 8) | png_image_data[byte_index+1];
                byte_index += 2;
                nlen = (png_image_data[byte_index] << 8) | png_image_data[byte_index+1];
                byte_index += 2;
                //printf("%x : %x : %x\n", len, nlen, nlen ^ 0xFFFF);
                //printf("%d : %d : %d\n", len, nlen, nlen ^ 0xFFFF);

                /*
                   copy LEN bytes of data to output
                   */
                for(i = 0; i < len; i++) {
                    output_stream[write_byte_index] = png_image_data[byte_index];
                    write_byte_index += 1;
                    byte_index += 1;
                }
            } else {
                /* if compressed with dynamic Huffman codes */
                if(btype == 0x02) {
                    /*
                       read representation of code trees (see
                       subsection below)
                       */
                    hlit = bit_read(png_image_data, byte_index, bit_index, 5);
                    printf("hlit = %d\n", hlit);
                    bit_index += 5;
                    if(bit_index >= 8) {
                        bit_index %= 8;
                        byte_index += 1;
                    }
                    hdist = bit_read(png_image_data, byte_index, bit_index, 5);
                    printf("hdist = %d\n", hdist);
                    bit_index += 5;
                    if(bit_index >= 8) {
                        bit_index %= 8;
                        byte_index += 1;
                    }
                    hclen = bit_read(png_image_data, byte_index, bit_index, 4);
                    printf("hclen = %d\n", hclen);
                    bit_index += 4;
                    if(bit_index >= 8) {
                        bit_index %= 8;
                        byte_index += 1;
                    }
                    for(i = 0; i < 19; i++) {
                        hclens[i] = 0;
                    }
                    for(i = 0; i < (hclen+4); i++) {
                        hclens[hclens_index_table[i]] = bit_read(png_image_data, byte_index, bit_index, 3);
                        //printf("hclens[%d] = %d\n", hclens_index_table[i], hclens[hclens_index_table[i]]);
                        bit_index += 3;
                        if(bit_index >= 8) {
                            bit_index %= 8;
                            byte_index += 1;
                        }
                    }

                    //clen
                    for(i = 0; i < 19; i++) {
                        clen[i] = 0;
                        tree[i].len = hclens[i];
                    }
                    for(i = 0; i < 8; i++) {
                        bl_count[i] = 0;
                    }
                    for(i = 0; i < 19; i++) {
                        bl_count[hclens[i]] += 1;
                        //printf("debug blcount[hclens[i]] = %d, hclens[i] = %d, i = %d\n", bl_count[hclens[i]], hclens[i], i);
                    }
                    max_bits = 0;
                    for(i = 0; i < 8; i++) {
                        //printf("[%d] : %d\n", i, bl_count[i]);
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

                    //printf("max_bits : %d\n", max_bits);

#if 0
                    code = 0;
                    bl_count[0] = 0;
                    for (bits = 1; bits <= max_bits; bits++) {
                        code = (code + bl_count[bits-1]) << 1;
                        next_code[bits] = code;
                    }
#endif

                    for(i = 0; i < max_bits; i++) {
                        //printf("code[%d] : %d\n", i, next_code[i]);
                    }

                    for(i = 0; i < (sizeof(table)/sizeof(int)); i++) {
                        table[i] = -1;
                    }

                    min_len = 255;
                    for (i = 0; i < 19; i++) {
                        len = tree[i].len;
                        if (len != 0) {
                            tree[i].code = next_code[len];
                            next_code[len]++;
                            table[tree[i].code] = i;

                            //printf("%d : %d : %d\n", i, tree[i].len, tree[i].code);
                            if(len < min_len) {
                                min_len = len;
                            }
                        }
                    }

                    lit = hlit + 257;
                    dist = hdist + 1;
                    id = (uint8_t *)malloc(sizeof(uint8_t) * (lit+dist));
                    id_index = 0;

                    do {
                        code = 0;
                        code_len = 0;
                        do {
                            code <<= 1;
                            code |= huffman_bit_read(png_image_data, byte_index, bit_index, 1);
                            //printf("code=%d, code_len=%d, ", code, code_len);
                            //printf("byte_index=%d, bit_index=%d\n", byte_index, bit_index);
                            code_len += 1;
                            bit_index += 1;
                            if(bit_index >= 8) {
                                bit_index %= 8;
                                byte_index += 1;
                            }
                            for(i = 0; i < 19; i++) {
                                //printf("%d %d %d\n", i, tree[i].len, tree[i].code);
                                if(tree[i].len == code_len && tree[i].code == code) {
                                    break;
                                }
                            }
                            //printf("id_index = %d\n", id_index);

                            if(code_len >= 8) {
                                return 0;
                            }

                        } while(i == 19);
                        //printf("id_index = %d\n", id_index);
                        //printf("i = %d, ", i);
                        //printf("code = %d, ", code);
                        //printf("code_len = %d, ", code_len);
                        //printf("table[code] = %d\n", table[code]);

                        if(table[code] >= 0 && table[code] <= 15) {
                            id[id_index] = table[code];
                            id_index += 1;
                        } else if(table[code] == 16) {
                            repeat = bit_read(png_image_data, byte_index, bit_index, 2);
                            bit_index += 2;
                            if(bit_index >= 8) {
                                bit_index %= 8;
                                byte_index += 1;
                            }
                            last_id = id[id_index-1];
                            for(i = 0; i < (repeat + 3); i ++) {
                                id[id_index] = last_id;
                                id_index += 1;
                            }
                        } else if(table[code] == 17) {
                            repeat = bit_read(png_image_data, byte_index, bit_index, 3);
                            bit_index += 3;
                            if(bit_index >= 8) {
                                bit_index %= 8;
                                byte_index += 1;
                            }
                            //printf("repeat = %d\n", repeat + 3);
                            //last_id = table[0];
                            //last_id = tree[0].code;
                            last_id = 0;
                            for(i = 0; i < (repeat + 3); i ++) {
                                id[id_index] = last_id;
                                id_index += 1;
                                if(id_index >= (lit+dist)) {
                                    break;
                                }
                            }
                        } else if(table[code] == 18) {
                            repeat = bit_read(png_image_data, byte_index, bit_index, 7);
                            bit_index += 7;
                            if(bit_index >= 8) {
                                bit_index %= 8;
                                byte_index += 1;
                            }
                            //printf("repeat = %d\n", repeat + 11);
                            //last_id = table[0];
                            //last_id = tree[0].code;
                            last_id = 0;
                            for(i = 0; i < (repeat + 11); i ++) {
                                id[id_index] = last_id;
                                id_index += 1;
                                if(id_index >= (lit+dist)) {
                                    break;
                                }
                            }
                        }

                        //printf("id_index = %d\nlit = %d, dist = %d\n", id_index, lit, dist);
                        //printf("id_index = %d\n", id_index);

                    } while(id_index != (lit+dist));

                    //printf("lit = %d, dist = %d\n", lit, dist);

                    // lit
                    for(i = 0; i < lit; i++) {
                        liten[i] = 0;
                        tree[i].len = id[i];
                    }
                    for(i = 0; i < 286; i++) {
                        bl_count[i] = 0;
                    }
                    for(i = 0; i < lit; i++) {
                        bl_count[id[i]] += 1;
                        //printf("debug blcount[id[i]] = %d, id[i] = %d, i = %d\n", bl_count[id[i]], id[i], i);
                    }
                    max_bits = 0;
                    for(i = 0; i < 286; i++) {
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
                    for(i = 0; i < max_bits; i++) {
                        //printf("code[%d] : %d\n", i, next_code[i]);
                    }
                    for(i = 0; i < (sizeof(lit_table)/sizeof(int)); i++) {
                        lit_table[i] = -1;
                    }
                    //printf("maxbit %d\n", max_bits);
                    min_len = 255;
                    for (i = 0; i < lit; i++) {
                        len = tree[i].len;
                        if (len != 0) {
                            tree[i].code = next_code[len];
                            next_code[len]++;
                            //printf("len=%d, %d\n", len, tree[i].code);
                            //lit_table[tree[i].code] = i;

                            //printf("%d : %d : %d\n", i, tree[i].len, tree[i].code);

                            if(len < min_len) {
                                min_len = len;
                            }
                        }
                    }

                    //dist
                    for(i = 0; i < dist; i++) {
                        disten[i] = 0;
                        dtree[i].len = id[i + lit];
                        //printf("dist  [%d] : %d\n", i, id[i+lit]);
                    }
                    for(i = 0; i < 32; i++) {
                        bl_count[i] = 0;
                    }
                    for(i = 0; i < dist; i++) {
                        bl_count[id[i+lit]] += 1;
                    }
                    max_bits = 0;
                    for(i = 0; i < 32; i++) {
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
                    for(i = 0; i < (sizeof(dist_table)/sizeof(int)); i++) {
                        dist_table[i] = -1;
                    }
                    min_dlen = 255;
                    for (i = 0; i < dist; i++) {
                        len = dtree[i].len;
                        if (len != 0) {
                            dtree[i].code = next_code[len];
                            next_code[len]++;
                            //dist_table[dtree[i].code] = i;

                            //printf("%d : %d : %d\n", i, dtree[i].len, dtree[i].code);

                            if(len < min_dlen) {
                                min_dlen = len;
                            }
                        }
                    }
                } // 0x02

                /* loop (until end of block code recognized) */
                do {
                    /* decode literal/length value from input stream */
                    code = 0;
                    code_len = 0;
                    do {
                        code <<= 1;
                        code |= huffman_bit_read(png_image_data, byte_index, bit_index, 1);
                        code_len += 1;
                        bit_index += 1;
                        if(bit_index >= 8) {
                            bit_index %= 8;
                            byte_index += 1;
                        }
                        for(i = 0; i < lit; i++) {
                            if(tree[i].len == code_len && tree[i].code == code) {
                                break;
                            }
                        }
                        //printf("code = %d, code_len = %d\n", code, code_len);
                        //printf("lit[%d]  = %x(%d)\n", i, tree[i].code , tree[i].len);
                    } while(i == lit);
                    //printf("code = %d, code_len = %d, byte_index=%d, bit_index = %d\n", code, code_len, byte_index, bit_index);
                    //printf("lit[%d]  = %x(%d)\n", i, tree[i].code , tree[i].len);
                    // 0, 2, 4
                    value = i;

                    /* if value < 256 */
                    if(value < 256) {
                        /* copy value (literal byte) to output stream */
                        output_stream[write_byte_index] = value;
                        write_byte_index += 1;
                        //byte_index += 1;
                    }
                    /* otherwise */
                    else {
                        /* if value = end of block (256) */
                        if(value == 256) {
                            /* break from loop */
                            break;
                        }
                        /* otherwise (value = 257..285) */
                        else {
                            len_bit = len_block_bit[value - 257];
                            len_bit_value = 0;
                            if(len_bit != 0) {
                                len_bit_value = bit_read(png_image_data, byte_index, bit_index, len_bit);
                                //printf("len_bit = %d\n", len_bit);
                                bit_index += len_bit;
                                if(bit_index >= 8) {
                                    byte_index += (bit_index / 8);
                                    bit_index %= 8;
                                }
                            }
                            /* decode distance from input stream */
                            code = 0;
                            code_len = 0;
                            do {
                                code <<= 1;
                                code |= huffman_bit_read(png_image_data, byte_index, bit_index, 1);
                                code_len += 1;
                                bit_index += 1;
                                if(bit_index >= 8) {
                                    bit_index %= 8;
                                    byte_index += 1;
                                }
                                //for(i = 0; i < lit; i++) {
                                for(i = 0; i < 32; i++) {
                                    if(dtree[i].len == code_len && dtree[i].code == code) {
                                        break;
                                    }
                                }
                                //printf("code = %d, code_len = %d\n", code, code_len);
                                //printf("lit[%d]  = %x(%d)\n", i, tree[i].code , tree[i].len);
                            //} while(i == lit);
                            } while(i == 32);
                            dist = i;
                            //printf("dist = %d\n", dist);
                            dist_bit = dist_block_bit[dist];
                            dist_bit_value = 0;
                            if(dist_bit != 0) {
                                dist_bit_value = bit_read(png_image_data, byte_index, bit_index, dist_bit);
                                //printf("dist_bit = %d\n", dist_bit);
                                bit_index += dist_bit;
                                if(bit_index >= 8) {
                                    byte_index += (bit_index / 8);
                                    bit_index %= 8;
                                }
                            }

                            len  = len_block[value-257];
                            len += len_bit_value;
                            dist = dist_block[dist];
                            dist += dist_bit_value;
                            //printf("dist = %d\n", dist);
                            /* move backwards distance bytes in the output stream, and copy length bytes from this position to the output stream. */
                            //write_origin = write_byte_index - 
                            for(i = 0; i < len; i++) {
                                output_stream[write_byte_index] = output_stream[write_byte_index-dist];
                                write_byte_index += 1;
                            }
                        }
                    }

                    /* end loop */
                } while(1);
                }

#if 0
                for(i = 0; i < write_byte_index; i++) {
                    printf("[%d] %x\n", i, output_stream[i]);
                }
#endif

                /* while not last block */
                } while(bfinal == 0);

                printf("write_byte : %d\n", write_byte_index);
                write_byte_index = 0;
                image_data = (RGBTRIPLE **)malloc(sizeof(RGBTRIPLE *) * height);
                for(i = 0; i < height; i++) {
                    image_data[i] = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * width);
                    if(output_stream[write_byte_index] == 0) {
                        printf("[%d] %d\n", i, output_stream[write_byte_index]);
                        write_byte_index += 1;
                        for(int j = 0; j < width; j++) {
                            image_data[i][j].rgbtBlue = color_palette[output_stream[write_byte_index]].rgbtBlue;
                            image_data[i][j].rgbtGreen = color_palette[output_stream[write_byte_index]].rgbtGreen;
                            image_data[i][j].rgbtRed = color_palette[output_stream[write_byte_index]].rgbtRed;
                            printf("[%d][%d] : %03d,%03d,%03d\n", i, j, image_data[i][j].rgbtRed, image_data[i][j].rgbtGreen, image_data[i][j].rgbtBlue);
                            write_byte_index++;
                        }
                    } else if(output_stream[write_byte_index] >= 1) {
                        printf("[%d] %d\n", i, output_stream[write_byte_index]);
                        write_byte_index += 1;
                        for(int j = 0; j < width; j++) {
                            image_data[i][j].rgbtBlue = color_palette[output_stream[write_byte_index]].rgbtBlue;
                            image_data[i][j].rgbtGreen = color_palette[output_stream[write_byte_index]].rgbtGreen;
                            image_data[i][j].rgbtRed = color_palette[output_stream[write_byte_index]].rgbtRed;
                            printf("[%d][%d] : %03d,%03d,%03d\n", i, j, image_data[i][j].rgbtRed, image_data[i][j].rgbtGreen, image_data[i][j].rgbtBlue);
                            write_byte_index++;
                        }
                    }
                    //printf("[%d] : %d %d %d\n", i+1, color_palette[i].rgbtRed, color_palette[i].rgbtGreen, color_palette[i].rgbtBlue);
                }
                image_info.height = height;
                image_info.width = width;
                image_info.fileSize = height*width*3 + 54;
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
            //LoG_filter(&output_image_data, &image_data, &image_info, 1.4, 9);
           
            for(i = 0; i < image_info.height; i++) {
                for(int j = 0; j < image_info.width; j++) {
                    output_image_data[i][j] = image_data[i][j];
                }
            }

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
