#include <cutter.h>
#include <png.h>

void test_one_bit_read(void);

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
