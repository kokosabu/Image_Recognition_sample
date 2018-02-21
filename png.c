#include "png.h"
#include <string.h>
#include <stdlib.h>

void decode_png(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data)
{
    int i;
    uint8_t *png_image_data;
    uint8_t *tmp;
    uint8_t byte;
    uint32_t size;
    uint32_t palette_size;
    uint32_t width;
    uint32_t height;
    uint8_t bps;
    uint8_t color_type;
    uint8_t compress_type;
    uint8_t filter_type;
    uint8_t interlace_type;
    uint32_t crc_32;
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
    char chunk[5];
    int bit_index;
    int byte_index;
    int write_byte_index;
    uint8_t *alpha_index;
    uint16_t alpha_gray;
    uint16_t alpha_red;
    uint16_t alpha_green;
    uint16_t alpha_blue;
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
    int repeat;
    int last_id;
    int len_bit;
    int len_bit_value;
    int dist_bit;
    int dist_bit_value;
    uint8_t old_red;
    uint8_t old_green;
    uint8_t old_blue;
    int k;
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
    uint32_t idat_size;

    printf("PNG\n");

    for(i = 0; i < 8; i++) {
        fread(&byte, 1, 1, input);
    }

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

            output_stream = (uint8_t *)malloc(sizeof(uint8_t) * (width+1) * height);

        } else if(strcmp(chunk, "IDAT") == 0) {
            idat_size += size;
            if(size == idat_size) {
                png_image_data = (uint8_t *)malloc(sizeof(uint8_t) * size);
                fread(png_image_data+idat_size-size, 1, size, input);
            } else {
                tmp = (uint8_t *)realloc(png_image_data, sizeof(uint8_t) * idat_size);
                png_image_data = tmp;
                fread(png_image_data+idat_size-size, 1, size, input);
            }
            crc_32 = read_4bytes(input);
            printf("size:%d\n", size);
            printf("chunk:%s\n", chunk);
            printf("crc-32: %xh\n", crc_32);
        } else if(strcmp(chunk, "PLTE") == 0) {
            color_palette = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * size);
            palette_size = size;

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
            //VirtualPageUnits = read_4bytes(input);
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
        } else {
            /* if compressed with dynamic Huffman codes */
            if(btype == 0x02) {
                /* read representation of code trees (see subsection below) */
                hlit = bit_read(png_image_data, &byte_index, &bit_index, 5);
                printf("hlit = %d\n", hlit);
                hdist = bit_read(png_image_data, &byte_index, &bit_index, 5);
                printf("hdist = %d\n", hdist);
                hclen = bit_read(png_image_data, &byte_index, &bit_index, 4);
                printf("hclen = %d\n", hclen);
                for(i = 0; i < 19; i++) {
                    hclens[i] = 0;
                }
                for(i = 0; i < (hclen+4); i++) {
                    hclens[hclens_index_table[i]] = bit_read(png_image_data, &byte_index, &bit_index, 3);
                    //printf("hclens[%d] = %d\n", hclens_index_table[i], hclens[hclens_index_table[i]]);
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
                        code |= huffman_bit_read(png_image_data, &byte_index, &bit_index, 1);
                        code_len += 1;
                        //printf("code=%d, code_len=%d, ", code, code_len);
                        //printf("byte_index=%d, bit_index=%d\n", byte_index, bit_index);
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
                        repeat = bit_read(png_image_data, &byte_index, &bit_index, 2);
                        last_id = id[id_index-1];
                        for(i = 0; i < (repeat + 3); i ++) {
                            id[id_index] = last_id;
                            id_index += 1;
                        }
                    } else if(table[code] == 17) {
                        repeat = bit_read(png_image_data, &byte_index, &bit_index, 3);
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
                        repeat = bit_read(png_image_data, &byte_index, &bit_index, 7);
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
                //printf("maxbit %d\n", max_bits);
                min_len = 255;
                for (i = 0; i < lit; i++) {
                    len = tree[i].len;
                    if (len != 0) {
                        tree[i].code = next_code[len];
                        next_code[len]++;
                        //printf("len=%d, %d\n", len, tree[i].code);

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
                min_dlen = 255;
                for (i = 0; i < dist; i++) {
                    len = dtree[i].len;
                    if (len != 0) {
                        dtree[i].code = next_code[len];
                        next_code[len]++;

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
                    code |= huffman_bit_read(png_image_data, &byte_index, &bit_index, 1);
                    code_len += 1;
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
                value = i;

                /* if value < 256 */
                if(value < 256) {
                    /* copy value (literal byte) to output stream */
                    output_stream[write_byte_index] = value;
                    write_byte_index += 1;
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
                            len_bit_value = bit_read(png_image_data, &byte_index, &bit_index, len_bit);
                            //printf("len_bit = %d\n", len_bit);
                        }
                        /* decode distance from input stream */
                        code = 0;
                        code_len = 0;
                        do {
                            code <<= 1;
                            code |= huffman_bit_read(png_image_data, &byte_index, &bit_index, 1);
                            code_len += 1;
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
                            dist_bit_value = bit_read(png_image_data, &byte_index, &bit_index, dist_bit);
                            //printf("dist_bit = %d\n", dist_bit);
                        }

                        len  = len_block[value-257];
                        len += len_bit_value;
                        printf("dist = %d\n", dist);
                        dist = dist_block[dist];
                        printf("dist(block) = %d\n", dist);
                        dist += dist_bit_value;
                        printf("dist(block+dist_bit) = %d\n", dist);
                        printf("write_byte:%d (%d)(%d)\n", write_byte_index, write_byte_index/(width+1), write_byte_index%(width+1));
                        /* move backwards distance bytes in the output stream, and copy length bytes from this position to the output stream. */
                        for(i = 0; i < len; i++) {
                            output_stream[write_byte_index] = output_stream[write_byte_index-dist];
                            write_byte_index += 1;
                        }
                    }
                }

                /* end loop */
            } while(1);
        }

        /* while not last block */
    } while(bfinal == 0);

    printf("write_byte : %d\n", write_byte_index);
    write_byte_index = 0;
    image_data = (RGBTRIPLE **)malloc(sizeof(RGBTRIPLE *) * height);
    for(i = 0; i < height; i++) {
        image_data[i] = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * width);
        printf("[%d] %d\n", i, output_stream[write_byte_index]);
        if(output_stream[write_byte_index] == 0) {
            write_byte_index += 1;
            for(int j = 0; j < width; j++) {
                image_data[i][j].rgbtBlue = color_palette[output_stream[write_byte_index]].rgbtBlue;
                image_data[i][j].rgbtGreen = color_palette[output_stream[write_byte_index]].rgbtGreen;
                image_data[i][j].rgbtRed = color_palette[output_stream[write_byte_index]].rgbtRed;
                //printf("[%d][%d] : %3d,%3d,%3d\n", i, j, image_data[i][j].rgbtRed, image_data[i][j].rgbtGreen, image_data[i][j].rgbtBlue);
                write_byte_index++;
            }
        } else if(output_stream[write_byte_index] == 1) {
            write_byte_index += 1;
            old_blue = 0;
            old_green = 0;
            old_red = 0;
            for(int j = 0; j < width; j++) {
                image_data[i][j].rgbtBlue = color_palette[output_stream[write_byte_index]].rgbtBlue   + old_blue;
                image_data[i][j].rgbtGreen = color_palette[output_stream[write_byte_index]].rgbtGreen + old_green;;
                image_data[i][j].rgbtRed = color_palette[output_stream[write_byte_index]].rgbtRed     + old_red;
                old_blue  = image_data[i][j].rgbtBlue;
                old_green = image_data[i][j].rgbtGreen;
                old_red   = image_data[i][j].rgbtRed;
                //printf("[%d][%d] : %3d,%3d,%3d\n", i, j, image_data[i][j].rgbtRed, image_data[i][j].rgbtGreen, image_data[i][j].rgbtBlue);
                write_byte_index++;
            }
        } else if(output_stream[write_byte_index] == 2) {
            write_byte_index += 1;
            for(int j = 0; j < width; j++) {
                if(i == 0) {
                    old_blue = 0;
                    old_green = 0;
                    old_red = 0;
                } else {
                    old_blue  = image_data[i-1][j].rgbtBlue;
                    old_green = image_data[i-1][j].rgbtGreen;
                    old_red   = image_data[i-1][j].rgbtRed;
                }
                image_data[i][j].rgbtBlue = color_palette[output_stream[write_byte_index]].rgbtBlue   + old_blue;
                image_data[i][j].rgbtGreen = color_palette[output_stream[write_byte_index]].rgbtGreen + old_green;;
                image_data[i][j].rgbtRed = color_palette[output_stream[write_byte_index]].rgbtRed     + old_red;
                //printf("[%d][%d] : %3d,%3d,%3d\n", i, j, image_data[i][j].rgbtRed, image_data[i][j].rgbtGreen, image_data[i][j].rgbtBlue);
                write_byte_index++;
            }
        } else if(output_stream[write_byte_index] == 3) {
            write_byte_index += 1;
            old_blue = 0;
            old_green = 0;
            old_red = 0;
            for(int j = 0; j < width; j++) {
                if(i == 0) {
                    old_blue += 0;
                    old_green += 0;
                    old_red += 0;
                } else {
                    old_blue  += image_data[i-1][j].rgbtBlue;
                    old_green += image_data[i-1][j].rgbtGreen;
                    old_red   += image_data[i-1][j].rgbtRed;
                }
                image_data[i][j].rgbtBlue = color_palette[output_stream[write_byte_index]].rgbtBlue   + old_blue/2;
                image_data[i][j].rgbtGreen = color_palette[output_stream[write_byte_index]].rgbtGreen + old_green/2;
                image_data[i][j].rgbtRed = color_palette[output_stream[write_byte_index]].rgbtRed     + old_red/2;
                old_blue  = image_data[i][j].rgbtBlue;
                old_green = image_data[i][j].rgbtGreen;
                old_red   = image_data[i][j].rgbtRed;
                //printf("[%d][%d] : %3d,%3d,%3d\n", i, j, image_data[i][j].rgbtRed, image_data[i][j].rgbtGreen, image_data[i][j].rgbtBlue);
                write_byte_index++;
            }
        } else if(output_stream[write_byte_index] == 4) {
        } else {
            printf("undefined filter type\n");
            return 0;
        }
        //printf("[%d] : %d %d %d\n", i+1, color_palette[i].rgbtRed, color_palette[i].rgbtGreen, color_palette[i].rgbtBlue);
    }
    image_info->height = height;
    image_info->width = width;
    image_info->fileSize = height*width*3 + 54;
}
