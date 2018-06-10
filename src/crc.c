#include "crc.h"

static uint32_t crc_table[256];

void make_crc_table(void) {
    uint32_t i;
    uint32_t c;
    int j;

    for (i = 0; i < 256; i++) {
        c = i;
        for (j = 0; j < 8; j++) {
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        }
        crc_table[i] = c;
    }
}

uint32_t crc32(uint8_t *buf, size_t len, uint32_t c) {
    size_t i;
    for (i = 0; i < len; i++) {
        c = crc_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
    }
    return c;
}

uint32_t crc32_4bytes(uint32_t *buf, uint32_t c) {
    c = crc32(((uint8_t *)buf)+3, 1, c);
    c = crc32(((uint8_t *)buf)+2, 1, c);
    c = crc32(((uint8_t *)buf)+1, 1, c);
    c = crc32(((uint8_t *)buf)+0, 1, c);
    return c;
}
