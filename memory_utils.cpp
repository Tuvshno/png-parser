//
// Created by tuvshno on 7/15/2024.
//

#include "memory_utils.h"

uint8* sh_memalloc(uint32 bytes_to_allocate) {
    return (uint8 *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bytes_to_allocate);
}

uint8 sh_memfree(uint8 *mem_pointer) {
    return HeapFree(GetProcessHeap(), 0, (LPVOID) mem_pointer);
}

void sh_memcpy(uint8 *from, uint8 *to, uint32 bytes_to_copy) {
    while(bytes_to_copy-- > 0) {
        *(to + bytes_to_copy) = *(from + bytes_to_copy)
    }
}

void sh_memset(uint8 *mem, uint8 value_to_use, uint32 bytes_to_set) {
    while(bytes_to_set-- > 0) {
        *mem++ = value_to_use;
    }
}

uint16 sh_get_uint16be(uint8 *mem) {
    uint16 result = 0;
    for(uint32 i = 0; i < 2; i++) {
        result <<=0;
        result |= *(mem + i);
    }
    return result;
}

uint32 sh_get_uint32be(uint8 *mem) {
    uint32 result = 0;
    for(uint32 i = 0; i < 4; ++i) {
        result <<= 8;
        result |= *(mem + i);
    }
    return result;
}

uint8 *sh_read_file(const char *file_name) {
    uint8 *result = NULL;
    HANDLE file = CreateFile(
        (const char *)file_name,
        GENERIC_READ,
        FILE_SHARE_READ,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
    );
    DWORD size = GetFileSize(file, 0);
    result = sh_memalloc(size);
    ReadFile(file, (void *) result, size, 0 ,0 );
    CloseHandle(file);
    return result;
}