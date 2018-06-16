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

int one_bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos)
{
    uint8_t pattern[8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
    };
    uint8_t bit;

    bit   = input_stream[*byte_pos];
    bit  &= pattern[*bit_pos];
    bit >>= *bit_pos;

    *bit_pos += 1;
    if(*bit_pos >= 8) {
        *byte_pos += 1;
        *bit_pos = 0;
    }

    return bit;
}

int bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos, int bit_len)
{
    uint16_t byte;
    int i;

    byte = 0;
    for(i = 0; i < bit_len; i++) {
        byte |= one_bit_read(input_stream, byte_pos, bit_pos) << i;
    }

    return byte;
}

int huffman_bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos, int bit_len)
{
    uint8_t pattern[8] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
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

int image_bit_read(uint8_t *input_stream, int *byte_pos, int *bit_pos, int bit_len)
{
    uint8_t pattern[8] = {
        0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
    };
    uint8_t byte;

    byte = input_stream[*byte_pos];
    byte &= pattern[*bit_pos];
    byte >>= (7 - *bit_pos);

    *bit_pos += bit_len;
    if(*bit_pos >= 8) {
        *byte_pos += (*bit_pos) / 8;
        *bit_pos %= 8;
    }

    return byte;
}

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
            return UNKOWN_FORMAT;
        }
    } else if(read_byte == 0x89) {
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'P') {
            fseek(input, 0, SEEK_SET);
            return UNKOWN_FORMAT;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'N') {
            fseek(input, 0, SEEK_SET);
            return UNKOWN_FORMAT;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'G') {
            fseek(input, 0, SEEK_SET);
            return UNKOWN_FORMAT;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != '\r') {
            fseek(input, 0, SEEK_SET);
            return UNKOWN_FORMAT;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != '\n') {
            fseek(input, 0, SEEK_SET);
            return UNKOWN_FORMAT;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != 0x1a) {
            fseek(input, 0, SEEK_SET);
            return UNKOWN_FORMAT;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != '\n') {
            fseek(input, 0, SEEK_SET);
            return UNKOWN_FORMAT;
        }
        fseek(input, 0, SEEK_SET);
        return PNG;
    } else if(read_byte == 'G') {
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'I') {
            fseek(input, 0, SEEK_SET);
            return UNKOWN_FORMAT;
        }
        fread(&read_byte, 1, 1, input);
        if(read_byte != 'F') {
            fseek(input, 0, SEEK_SET);
            return UNKOWN_FORMAT;
        }
        fseek(input, 0, SEEK_SET);
        return GIF;
    }

    fseek(input, 0, SEEK_SET);
    return UNKOWN_FORMAT;
}
