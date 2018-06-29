#include <cutter.h>
#include "png.h"
#include "gif.h"

void test_one_bit_read(void);
void test_docompress_fixed_huffman_codes();
void test_check_file_format_gif(void);
void test_lzw(void);

//static Stack *stack;

void cut_setup (void)
{
}

void cut_teardown (void)
{
}

void test_one_bit_read(void)
{
    int bit;
    uint8_t input[1];
    int byte_pos;
    int bit_pos;
    int i;

    input[0] = 0xFF;
    byte_pos = 0;
    bit_pos = 0;
    for(i = 0; i < 8; i++) {
        cut_assert(byte_pos == 0);
        cut_assert(bit_pos == i);
        bit = one_bit_read(input, &byte_pos, &bit_pos);
        cut_assert(bit == 1);
    }
    cut_assert(byte_pos == 1);
    cut_assert(bit_pos == 0);

    input[0] = 0x55;
    byte_pos = 0;
    bit_pos = 0;
    for(i = 0; i < 4; i++) {
        cut_assert(byte_pos == 0);
        cut_assert(bit_pos == i*2);
        bit = one_bit_read(input, &byte_pos, &bit_pos);
        cut_assert(bit == 1);

        cut_assert(byte_pos == 0);
        cut_assert(bit_pos == i*2+1);
        bit = one_bit_read(input, &byte_pos, &bit_pos);
        cut_assert(bit == 0);
    }
    cut_assert(byte_pos == 1);
    cut_assert(bit_pos == 0);
}

void test_docompress_fixed_huffman_codes()
{
    int lit;
    int dist;
    struct tree tree[288];
    struct tree dtree[32];
    int byte_index;
    int bit_index;
    uint8_t *png_image_data;

    decompress_fixed_huffman_codes(png_image_data, &byte_index, &bit_index, &lit, &dist, tree, dtree);

    cut_assert(lit == 288);
    cut_assert(tree[0].code == 0x30);
    cut_assert(tree[0].len == 8);


    cut_assert(dist == 32);
    cut_assert(dtree[0].code == 0);
    cut_assert(dtree[0].len == 5);
    cut_assert(dtree[29].code == 29);
    cut_assert(dtree[29].len == 5);
    cut_assert(dtree[30].code == 30);
    cut_assert(dtree[30].len == 5);
    cut_assert(dtree[31].code == 31);
    cut_assert(dtree[31].len == 5);
}

void test_check_file_format_dummy_gif(void)
{
    FILE *input;
    int file_format;
    IMAGEINFO image_info;
    RGBTRIPLE **image_data;

    input = fopen("./test/dummy.gif", "rb");
    file_format = check_file_format(input);
    cut_assert(file_format == GIF);

    decode_gif(input, &image_info, &image_data);
    cut_assert(image_info.width == 1);
    cut_assert(image_info.height == 1);

    fclose(input);
}

void test_check_file_format_ok_gif(void)
{
    FILE *input;
    int file_format;
    IMAGEINFO image_info;
    RGBTRIPLE **image_data;

    input = fopen("./test/ok.gif", "rb");
    file_format = check_file_format(input);
    cut_assert(file_format == GIF);

    decode_gif(input, &image_info, &image_data);
    cut_assert(image_info.width == 1);
    cut_assert(image_info.height == 1);

    fclose(input);
}

void test_lzw(void)
{
    /* http://www.geocities.co.jp/NatureLand/2023/reference/Compression/lzw.html */
    uint16_t original_data[11] =
        { 0xA4, 0xA4, 0xA4, 0xA4, 0x10, 0x36, 0xB0, 0xA3, 0xC5, 0xC5, 0xC5 };
    uint16_t compress_data[11] =
        { 0, };

    init_table();

    cut_assert(get_data(0x00)[0] == 0x00);
    cut_assert(get_data(0xff)[0] == 0xff);
    cut_assert(get_data(0x100)   == (uint8_t *)CLEAR);
    cut_assert(get_data(0x101)   == (uint8_t *)END);

    compress(compress_data, 11, original_data, 11);

    cut_assert(compress_data[0] == 0xA4);
    cut_assert(compress_data[1] == 0x102);
    cut_assert(compress_data[2] == 0x102);
    cut_assert(compress_data[3] == 0x10);
    cut_assert(compress_data[4] == 0x36);
    cut_assert(compress_data[5] == 0xB0);
    cut_assert(compress_data[6] == 0xCF);
    cut_assert(compress_data[7] == 0xCF);
    cut_assert(compress_data[8] == 0xD5);
    cut_assert(compress_data[9] == 0xD5);

    cut_assert(get_data(0x102)[0] == 0xA4);
    cut_assert(get_data(0x102)[1] == 0xA4);
}
