#include <cutter.h>
#include "png.h"
#include "gif.h"

void test_one_bit_read(void);
void test_docompress_fixed_huffman_codes();
void test_check_file_format_gif(void);

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

void test_check_file_format_gif(void)
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
