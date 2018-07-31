#include "gif.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static uint8_t *lzw_table[4096];
static uint8_t lzw_table_data_size[4096];
static int lzw_table_size;
static uint8_t bit_length;

static void update_bit_length(void);
static void update_bit_length_for_decompress(void);
static void output_compress_data(uint8_t *compress_data, uint8_t *bit_lengths, int *compress_data_index, int output_code);
static int search_lzw_table(uint8_t *code, int size);
static void connect(uint8_t *connect, int *size, uint8_t *prefix, int prefix_size, uint8_t *suffix, int suffix_size);
static void copy(uint8_t *prefix, int *prefix_size, uint8_t *suffix, int suffix_size);
static void read_char(uint8_t *to, int *to_size, uint8_t *data, int *data_index, uint8_t *length, int *length_index, int *byte_pos, int *bit_pos);
static void entry_dict(uint8_t *com2, int com2_size);
static void update_bit_length(void);

static void update_bit_length(void)
{
    if((lzw_table_size-1) >= pow(2, bit_length)) {
        bit_length += 1;
    }
}

static void update_bit_length_for_decompress(void)
{
    if((lzw_table_size) >= pow(2, bit_length)) {
        bit_length += 1;
    }
}

static void output_compress_data(uint8_t *compress_data, uint8_t *bit_lengths, int *compress_data_index, int output_code)
{
    compress_data[*compress_data_index] = output_code;
    bit_lengths[*compress_data_index] = bit_length;
    *compress_data_index += 1;

    update_bit_length();
}

static int search_lzw_table(uint8_t *code, int size)
{
    int i;
    int j;

    if(size == 0) {
        for(i = 0; i < lzw_table_size; i++) {
            if(lzw_table[i] == code) {
                return i;
            }
        }
    } else {
        for(i = 0; i < lzw_table_size; i++) {
            for(j = 0; j < lzw_table_data_size[i]; j++) {
                if(code[j] != lzw_table[i][j]) {
                    break;
                }
            }
            if(j == lzw_table_data_size[i] && j == size) {
                return i;
            }
        }
    }

    return -1;
}

static void connect(uint8_t *connect, int *size, uint8_t *prefix, int prefix_size, uint8_t *suffix, int suffix_size)
{
    int i;

    for(i = 0; i < prefix_size; i++) {
        connect[*size] = prefix[i];
        *size += 1;
    }
    for(i = 0; i < suffix_size; i++) {
        connect[*size] = suffix[i];
        *size += 1;
    }
}

static void copy(uint8_t *to, int *to_size, uint8_t *from, int from_size)
{
    int i;
    for(i = 0; i < from_size; i++) {
        to[i] = from[i];
    }
    *to_size = from_size;
}

static void read_char(uint8_t *to, int *to_size, uint8_t *data, int *data_index, uint8_t *length, int *length_index, int *byte_pos, int *bit_pos)
{
    int bits;

    bits = bit_read(data, byte_pos, bit_pos, length[*length_index]);

    to[*to_size] = bits;
    *data_index += 1;
    *to_size += 1;
    *length_index += 1;
}

static void entry_dict(uint8_t *com2, int com2_size)
{
    int i;

    lzw_table[lzw_table_size] = (uint8_t *)malloc(sizeof(uint8_t) * com2_size);
    for(i = 0; i < com2_size; i++) {
        lzw_table[lzw_table_size][i] = com2[i];
    }
    lzw_table_data_size[lzw_table_size] = com2_size;
    lzw_table_size += 1;
}

void read_header(FILE *input)
{
    char signature[4];
    char version[4];

    fread(signature, 1, 3, input);
    signature[3] = '\0';
    assert(strcmp(signature, "GIF") == 0);

    fread(version, 1, 3, input);
    version[3] = '\0';
    assert(strcmp(version, "89a") == 0);
}

void read_logical_screen_descriptor(FILE *input, IMAGEINFO *image_info, unsigned char *global_color_table_flag, unsigned char *size_of_global_color_table)
{
    unsigned char byte;
    unsigned char color_resolution;
    unsigned char sort_flag;
    unsigned char background_color_index;
    unsigned char pixel_aspect_ratio;

    fread(&byte, 1, 1, input);
    image_info->width = byte;
    fread(&byte, 1, 1, input);
    image_info->width += ((unsigned int)byte) << 8;

    fread(&byte, 1, 1, input);
    image_info->height = byte;
    fread(&byte, 1, 1, input);
    image_info->height += ((unsigned int)byte) << 8;

    fread(&byte, 1, 1, input);
    *global_color_table_flag    = (byte & 0x80) >> 7;
    color_resolution            = (byte & 0x70) >> 4;
    sort_flag                   = (byte & 0x08) >> 3;
    *size_of_global_color_table = (byte & 0x07);

    fread(&background_color_index, 1, 1, input);
    fread(&pixel_aspect_ratio, 1, 1, input);
}

void read_global_color_table(FILE *input, RGBTRIPLE **global_color_table, unsigned char global_color_table_flag, unsigned char size_of_global_color_table)
{
    int i;

    if(global_color_table_flag == 1) {
        *global_color_table  = (RGBTRIPLE *)malloc(sizeof(RGBTRIPLE) * (uint8_t)pow(2, size_of_global_color_table+1));
        for(i = 0; i < (uint8_t)pow(2, size_of_global_color_table+1); i++) {
            fread(&((*global_color_table)[i].rgbtRed),   1, 1, input);
            fread(&((*global_color_table)[i].rgbtGreen), 1, 1, input);
            fread(&((*global_color_table)[i].rgbtBlue),  1, 1, input);
        }
    } else {
        *global_color_table = NULL;
    }
}

void read_image_descriptor(FILE *input)
{
    unsigned char byte;
    uint16_t image_left_position;
    uint16_t image_top_position;
    uint16_t image_width;
    uint16_t image_height;
    uint8_t local_color_table_flag;
    uint8_t interlace_flag;
    uint8_t local_sort_flag;
    uint8_t reserved;
    uint8_t size_of_local_color_table;
    RGBTRIPLE *local_color_table;
    int i;

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
}

void read_graphic_control_extension(FILE *input)
{
    unsigned char byte;
    uint8_t reserved;
    uint8_t disposal_mothod;
    uint8_t user_input_flag;
    uint8_t transparent_color_flag;
    uint8_t transparent_color_index;
    uint16_t delay_time;
    uint8_t block_terminator;

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
}

void read_comment_extension(FILE *input)
{
    unsigned char byte;
    uint8_t block_size;
    uint8_t *comment_data;

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
}

void read_plain_text_extension(FILE *input)
{
    uint8_t block_size;
    unsigned char byte;
    uint16_t text_grid_left_position;
    uint16_t text_grid_top_position;
    uint16_t text_grid_width;
    uint16_t text_grid_height;
    uint8_t character_cell_width;
    uint8_t character_cell_height;
    uint8_t text_foreground_color_index;
    uint8_t text_background_color_index;
    uint8_t *plain_text_data;

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
}

void read_application_extension(FILE *input)
{
    uint8_t block_size;
    uint8_t application_identifier[8];
    uint8_t application_authentication_code[3];
    unsigned char byte;
    uint8_t *application_data;

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
}

void decode_gif(FILE *input, IMAGEINFO *image_info, RGBTRIPLE ***image_data)
{
    unsigned char byte;
    unsigned char global_color_table_flag;
    unsigned char size_of_global_color_table;
    uint8_t LZW_minimum_code_size;
    uint8_t block_size;
    uint8_t block_image_data[255];
    uint8_t block_terminator;
    uint8_t original_data[2048];
    RGBTRIPLE *global_color_table;

    read_header(input);
    read_logical_screen_descriptor(input, image_info, &global_color_table_flag, &size_of_global_color_table);
    read_global_color_table(input, &global_color_table, global_color_table_flag, size_of_global_color_table);

    do {
        fread(&byte, 1, 1, input);
        if(byte == 0x2c) {
            /* Image Block */
            read_image_descriptor(input);

            fread(&LZW_minimum_code_size, 1, 1, input);
            init_table(LZW_minimum_code_size+1);
            fread(&block_size, 1, 1, input);
            do {
                fread(block_image_data, 1, block_size, input);
                decompress(block_image_data, block_size, original_data, sizeof(original_data));
#if 0
                printf("width: %d, height: %d\n", image_info->width, image_info->height);
                for(int i = 0; i < image_info->width * image_info->height; i++) {
                    printf("[%d] %d\n", i, original_data[i]);
                }
#endif

                fread(&block_terminator, 1, 1, input);
                if(block_terminator == 0x00) {
                    break;
                }
                block_size = block_terminator;
            } while(1);
        } else if(byte == 0x21) {
            fread(&byte, 1, 1, input);

            if(byte == 0xf9) {
                read_graphic_control_extension(input);
            } else if(byte == 0xfe) {
                read_comment_extension(input);
            } else if(byte == 0x01) {
                read_plain_text_extension(input);
            } else if(byte == 0xff) {
                read_application_extension(input);
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
        lzw_table_data_size[i] = 1;
    }

    lzw_table[i] = (uint8_t *)CLEAR;
    lzw_table_data_size[i] = 0;
    i += 1;

    lzw_table[i] = (uint8_t *)END;
    lzw_table_data_size[i] = 0;
    i += 1;

    lzw_table_size = i;
    bit_length = bit;
}

uint8_t *get_data(int index)
{
    return lzw_table[index];
}

void compress(uint8_t *compress_data, int compress_data_size, uint8_t *original_data, int original_data_size, uint8_t *bit_lengths, int bit_lengths_size)
{
    int i;
    uint8_t prefix[1024];
    uint8_t suffix[1024];
    uint8_t com1[1024];
    uint8_t com2[1024];
    int compress_data_index;
    int original_data_index;
    int bit_length_index;
    int prefix_size;
    int suffix_size;
    int com1_size;
    int com2_size;
    int output_code;
    int byte_pos;
    int bit_pos;
    uint8_t bits;

    compress_data_index = 0;
    original_data_index = 0;
    bit_length_index = 0;
    byte_pos = 0;
    bit_pos = 0;
    bits = 8;

    /* 1:クリアコードの出力 */
    output_code = search_lzw_table((uint8_t *)CLEAR, 0);
    output_compress_data(compress_data, bit_lengths, &compress_data_index, output_code);

    /* 2:圧縮対象の文字列(数字)から一文字を読み込みprefix変数に格納する */
    prefix_size = 0;
    read_char(prefix, &prefix_size, original_data, &original_data_index, &bits, &bit_length_index, &byte_pos, &bit_pos);
    bit_length_index = 0;

    do {
        /* 3:次の一文字を読み込んでsuffix変数に格納する */
        suffix_size = 0;
        read_char(suffix, &suffix_size, original_data, &original_data_index, &bits, &bit_length_index, &byte_pos, &bit_pos);
        bit_length_index = 0;

        /* 4-1:prefix変数 + suffix変数を連結した文字列をcom1変数に格納する。 */
        com1_size = 0;
        connect(com1, &com1_size, prefix, prefix_size, suffix, suffix_size);

        /* 4-2:次にcom1の内容が辞書に登録されているかを確認をする */
        output_code = search_lzw_table(com1, com1_size);

        if(output_code != -1) {
            /* [登録済] */
FIVE:
            if(original_data_index == original_data_size) {
                output_compress_data(compress_data, bit_lengths, &compress_data_index, output_code);
                break;
            }
            /* 5-1:次の一文字をsuffixに格納する。 */
            suffix_size = 0;
            read_char(suffix, &suffix_size, original_data, &original_data_index, &bits, &bit_length_index, &byte_pos, &bit_pos);
            bit_length_index = 0;

            /* 5-2:com1 + suffixを連結した文字列をcom2変数に格納する。 */
            com2_size = 0;
            connect(com2, &com2_size, com1, com1_size, suffix, suffix_size);

            /* 5-3:次にcom2が辞書に登録されているかを確認をする */
            output_code = search_lzw_table(com2, com2_size);

            if(output_code != -1) {
                /* [登録済] */
                /* 6:com2をcom1に格納して5に戻る */
                copy(com1, &com1_size, com2, com2_size);
                goto FIVE;
            } else {
                /* [未登録] */
                /* 7-1:com2を辞書に登録する。 */
                entry_dict(com2, com2_size);

                /* 7-2:com1の辞書番号を出力する。 */
                output_code = search_lzw_table(com1, com1_size);
                output_compress_data(compress_data, bit_lengths, &compress_data_index, output_code);

                if(original_data_index == original_data_size) {
                    for(i = 0; i < lzw_table_size; i++) {
                        if(lzw_table[i][0] == original_data[original_data_index-1]) {
                            output_compress_data(compress_data, bit_lengths, &compress_data_index, i);
                            break;
                        }
                    }
                    break;
                }

                /* 7-3:prefixにsuffixを格納して3に戻る */
                copy(prefix, &prefix_size, suffix, suffix_size);
            }
        } else {
            /* [未登録] */
            /* com1を辞書に登録して、prefixの辞書番号を出力する。 */
            entry_dict(com1, com1_size);

            output_code = search_lzw_table(prefix, prefix_size);
            output_compress_data(compress_data, bit_lengths, &compress_data_index, output_code);

            /* suffixをprefixに格納して3に戻る */
            copy(prefix, &prefix_size, suffix, suffix_size);
        }
    } while(1);

    /* 8:全ての文字列を圧縮した後にエンドコードを出力する */
    output_code = search_lzw_table((uint8_t *)END, 0);
    output_compress_data(compress_data, bit_lengths, &compress_data_index, output_code);
}

void decompress(uint8_t *compress_data, int compress_data_size, uint8_t *original_data, int original_data_size)
{
    uint8_t prefix[1024];
    uint8_t suffix[1024];
    uint8_t com1[1024];
    uint8_t com2[1024];
    uint8_t com3[1024];
    int prefix_size;
    int suffix_size;
    int com1_size;
    int com2_size;
    int com3_size;
    int compress_data_index;
    int original_data_index;
    int bit_length_index;
    int output_code;
    int output_code1;
    int output_code2;
    int i;
    int byte_pos;
    int bit_pos;

    compress_data_index = 0;
    original_data_index = 0;
    bit_length_index = 0;
    byte_pos = 0;
    bit_pos = 0;

    output_code = search_lzw_table((uint8_t *)CLEAR, 0);
    prefix_size = 0;
    read_char(prefix, &prefix_size, compress_data, &compress_data_index, &bit_length, &bit_length_index, &byte_pos, &bit_pos);
    bit_length_index = 0;
    if(output_code != prefix[0]) {
        goto PASS;
        printf("format error\n");
        exit(-1);
    }

    /* a.最初の数を出力数に、次の数を待機数に読み込みます。辞書を初期化します。 */
    prefix_size = 0;
    read_char(prefix, &prefix_size, compress_data, &compress_data_index, &bit_length, &bit_length_index, &byte_pos, &bit_pos);
    bit_length_index = 0;
PASS:
    suffix_size = 0;
    read_char(suffix, &suffix_size, compress_data, &compress_data_index, &bit_length, &bit_length_index, &byte_pos, &bit_pos);
    bit_length_index = 0;

    do {
        /* b.辞書の出力数のページの値と辞書の待機数のページにある値の最初の文字を並べた数を辞書の新しいページに書き込みます。 */
        output_code1 = prefix[0];
        output_code2 = suffix[0];
        copy(com1, &com1_size, lzw_table[output_code1], lzw_table_data_size[output_code1]);
        if(output_code2 < lzw_table_size) {
            copy(com2, &com2_size, lzw_table[output_code2], lzw_table_data_size[output_code2]);
        } else {
            com2[0] = com1[0];
        }
        com3_size = 0;
        connect(com3, &com3_size, com1, com1_size, com2, 1);
        entry_dict(com3, com3_size);
        update_bit_length_for_decompress();

        /* c.辞書の出力数のページに書かれている値を書き出します。 */
        for(i = 0; i < com1_size; i++) {
            original_data[original_data_index] = com1[i];
            original_data_index += 1;
        }

        /* d.待機数を出力数に、新しく一つ読み込んで待機数に入れます。 */
        copy(prefix, &prefix_size, suffix, suffix_size);
        suffix_size = 0;
        read_char(suffix, &suffix_size, compress_data, &compress_data_index, &bit_length, &bit_length_index, &byte_pos, &bit_pos);
        bit_length_index = 0;

        /* e.以下、b〜dの繰り返し */
        if(suffix[0] == search_lzw_table((uint8_t *)END, 0)) {
            for(i = 0; i < lzw_table_data_size[prefix[0]]; i++) {
                original_data[original_data_index] = lzw_table[prefix[0]][i];
                original_data_index += 1;
            }
            break;
        }
    } while(1);
}

