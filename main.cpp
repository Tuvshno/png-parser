#include <cstdint>
#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}

struct sh_png_chunk {
    uint32 data_length;
    uint8 type[4];
    uint8 *data;
    uint32 crc32;
};

/**
 * Converts from bytes to big endian
 *
 * @param mem
 * @return big endian array
 */
