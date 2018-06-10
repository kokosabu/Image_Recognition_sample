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
    int a;
    uint8_t input[1];
    int byte_pos;
    int bit_pos;

    input[0] = 0xFF;
    byte_pos = 0;
    bit_pos = 0;
    a = one_bit_read(input, &byte_pos, &bit_pos);
    cut_assert(a == 1);
}
