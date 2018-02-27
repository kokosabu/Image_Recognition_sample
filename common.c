#include "common.h"

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

int bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos, int bit_len)
{
    uint8_t pattern[8] = {
        0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
    };
    uint8_t byte;
    uint8_t next_byte;

    byte      = input_stream[*byte_pos];
    next_byte = input_stream[(*byte_pos)+1];

    if((8-(*bit_pos)) >= bit_len) {
        byte >>= *bit_pos;
        byte &= pattern[bit_len-1];
    } else {
        byte >>= *bit_pos;
        byte &= pattern[(8-(*bit_pos))-1];
        next_byte &= pattern[(*bit_pos) - (8-bit_len) - 1];
        byte = byte | ( next_byte << (8-(*bit_pos)) );
    }

    *bit_pos += bit_len;
    if(*bit_pos >= 8) {
        *byte_pos += (*bit_pos) / 8;
        *bit_pos %= 8;
    }

    return byte;
}

int huffman_bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos, int bit_len)
{
    uint8_t pattern[8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
            //0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
    };
    uint8_t byte;

    byte = input_stream[*byte_pos];
    byte &= pattern[*bit_pos];
    byte >>= *bit_pos;

    *bit_pos += bit_len;
    if(*bit_pos >= 8) {
        *byte_pos += (*bit_pos) / 8;
        *bit_pos %= 8;
    }

    return byte;
}
