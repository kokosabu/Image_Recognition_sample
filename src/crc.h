#ifndef CRC_H
#define CRC_H

#include <stdio.h>
#include <stdint.h>

void make_crc_table(void);
uint32_t crc32(uint8_t *buf, size_t len, uint32_t c);
uint32_t crc32_4bytes(uint32_t *buf, uint32_t c);

#endif /* CRC_H */
