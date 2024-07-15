//
// Created by tuvshno on 7/15/2024.
//

#ifndef MEMORY_UTILS_H
#define MEMORY_UTILS_H

#include <stdint.h>
#include <windows.h>

//Macro to skip bytes
#define SKIP_BYTES(mem, byte_num) (mem += byte_num)

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int32_t int32;

uint8* sh_memalloc(uint32 bytes_to_allocate);
uint8 sh_memfree(uint8 *mem_pointer);
void sh_memcpy(uint8 *from, uint8 *to, uint32 bytes_to_copy);
void sh_memset(uint8 *mem, uint8 value_to_use, uint32 bytes_to_set);
uint16 sh_get_uint16be(uint8 *mem);
uint32 sh_get_uint32be(uint8 *mem);
uint8* sh_read_file(const char *file_name);

#endif //MEMORY_UTILS_H
