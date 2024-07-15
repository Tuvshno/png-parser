//
// Created by tuvshno on 7/15/2024.
//

#ifndef PNG_UTILS_H
#define PNG_UTILS_H

#include "memory_utils.h"

struct sh_png_chunk {
    uint32 data_length;
    uint8 type[4];
    uint8  *data;
    uint32 crc32;
};

struct sh_zlib_block {
    uint8 cmf;
    uint8 extra_flags;
    uint8 *data;
    uint16 check_value;
};

struct sh_png_bit_stream {
    uint8 *data_stream;
    uint32 bit_buffer;
    uint32 bits_remaining;
}

sh_png_chunk sh_png_read_chunk(uint8 *mem);



#endif //PNG_UTILS_H
