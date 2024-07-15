//
// Created by tuvshno on 7/15/2024.
//

#include "png_utils.h"

sh_png_chunk sh_png_read_chunk(uint8 *mem) {
    sh_png_chunk chunk = {};
    chunk.data_length = sh_get_uint16be(mem);
    SKIP_BYTES(mem, 4);

    *( (uint32 *)&chunk.type) = *((uint32 *) mem);
    SKIP_BYTES(mem, 4);

    chunk.data = sh_memalloc(chunk.data_length);

    sh_memcpy(mem, chunk.data, chunk.data_length);
    SKIP_BYTES(mem, chunk.data_length);

    chunk.crc32 = sh_get_uint32be(mem);

    return chunk;
}

sh_zlib_block sh_read_zlib_block(uint8 *mem, uint32 length) {
    sh_zlib_block zlib_block = {};
    zlib_block.cmf = *mem;
    SKIP_BYTES(mem, 1);

    zlib_block.extra_flags = *mem;
    SKIP_BYTES(mem, 1);

    zlib_block.data = sh_memalloc(length - 2 - 2);

    sh_memcpy(mem, zlib_block.data, length - 2);
    SKIP_BYTES(mem, length - 2);

    zlib_block.check_value = sh_get_uint16be(mem);

    return zlib_block;
}

void sh_png_get_bits(sh_png_bit_stream *bits, uint32 bits_required) {
    uint32 extra_bits_needed = (bits->bits_remaining > bits_required) ? (bits->bits_remaining - bits_required) : (bits_required - bits->bits_remaining);
    uint32 bytes_to_read = extra_bits_needed/8;

    if(extra_bits_needed%8) {
        bytes_to_read++;
    }

    for(uint32 i = 0; i < bytes_to_read; ++i) {
        uint32 byte = *bits->data_stream++;
        bits->bit_buffer |= byte << (i*8 + bits->bits_remaining); //we need to be careful to not overwrite the remaining bits if any
    }

    bits->bits_remaining += bytes_to_read*8;
}

uint32 sh_png_read_bits(sh_png_bit_stream *bits, uint32 bits_to_read) {

    uint32 result = 0;

    if(bits_to_read > bits->bits_remaining) {
        sh_png_get_bits(bits, bits_to_read);
    }

    for(uint32 i = 0; i < bits_to_read; ++i) {
        uint32 bit = bits->bit_buffer & (1 << i);
        result |= bit;
    }

    bits->bit_buffer >>= bits_to_read;
    bits->bits_remaining -= bits_to_read;

    return result;
}

uint8 sh_get_maximum_bit_length(uint8 *code_bit_lengths, uint32 len_of_array) {
    uint8 max_bit_length = 0;
    for(uint32 i = 0; i < len_of_array; ++i) {
        if(max_bit_length < code_bit_lengths[i]) {
            max_bit_length = code_bit_lengths[i];
        }
    }

    return max_bit_length;
}

void sh_get_bit_length_count(uint32 *code_count, uint8 *code_bit_length, uint32 bit_len_array_len) {
    for(uint32 i = 0; i < bit_len_array_len; ++i) {
        code_count[code_bit_length[i]]++;
    }
}

void sh_first_code_for_bitlen(uint32 *first_codes, uint32 *code_count, uint32 max_bit_length) {
    uint32 code = 0;
    for(uint32 i = 1; i <= max_bit_length; ++i) {
        code = ( code + code_count[i-1]) << 1;

        if(code_count[i] > 0) {
            first_codes[i] = code;
        }
    }
}

void sh_assign_Huffman_code(uint32 *assigned_codes, uint32 *first_codes, uint8 *code_bit_lengths, uint32 len_assign_code) {
    for(uint32 i = 0; i < len_assign_code; ++i) {
        if(code_bit_lengths[i]) {
            assigned_codes[i] = first_codes[code_bit_lengths[i]]++;
        }
    }
}

uint32* sh_build_huffman_code(uint8 *code_bit_lengths, uint32 len_code_bit_lengths) {
    uint32 max_bit_length = sh_get_maximum_bit_length(code_bit_lengths, len_code_bit_lengths);

    uint32 *code_counts = (uint32 *)sh_memalloc(sizeof(uint32)*( max_bit_length + 1 ));
    uint32 *first_codes = (uint32 *)sh_memalloc(sizeof(uint32)*(max_bit_length + 1));
    //we have to assign code to every element in the alphabet, even if we have to assign zero
    uint32 *assigned_codes = (uint32 *)sh_memalloc(sizeof(uint32)*(len_code_bit_lengths));


    sh_get_bit_length_count(code_counts,  code_bit_lengths, len_code_bit_lengths);
    //in the real world, when a code of the alphabet has zero bit length, it means it doesn't occur in the data thus we have to reset the count for the zero bit length codes to 0.
    code_counts[0] = 0;

    sh_first_code_for_bitlen(first_codes, code_counts, max_bit_length);
    sh_assign_Huffman_code(assigned_codes, first_codes, code_bit_lengths, len_code_bit_lengths);


    return assigned_codes;
}

uint32 sh_peak_bits_reverse(sh_png_bit_stream *bits, uint32 bits_to_peak) {
    if(bits_to_peak > bits->bits_remaining) {
        sh_png_get_bits(bits, bits_to_peak);
    }

    uint32 result = 0; //this could potentially cause problems,
    for(uint32 i = 0; i < bits_to_peak; ++i) {
        result <<= 1;
        uint32 bit = bits->bit_buffer & (1 << i);
        result |= (bit > 0) ? 1 : 0;
    }

    return result;
}


uint32 sh_decode_huffman(sh_png_bit_stream *bits, uint32 *assigned_codes, uint8 *code_bit_lengths, uint32 assigned_code_length) {
    for(uint32 i = 0; i < assigned_code_length; ++i) {
        uint32 code = sh_peak_bits_reverse(bits, code_bit_lengths[i]);
        if(assigned_codes[i] == code) {
            bits->bit_buffer >>= code_bit_lengths[i];
            bits->bits_remaining -= code_bit_lengths[i];
            return i;
        }
    }

    return 0;
}

uint8* sh_zlib_deflate_block(
        sh_png_bit_stream *bits,
        uint32 *literal_tree, uint8 *lit_code_bit_len, uint32 lit_arr_len,
        uint32 *distance_tree, uint8 *dist_tree_bit_len, uint32 dist_arr_len,
        uint32 *bytes_read)
{
    //1 MB data for the uncompressed block, you can pre allocate a giant memory
    // that you can pass in, the size of the memory would probably be something like
    // bytes_per_pixel * width * height + height*filter_byte
    //each row has a filter byte(s) in the beginning that you have to account for
    //when you decompress

    uint8 *decompressed_data = sh_memalloc(1024*1024);
    uint32 data_index = 0;
    while(true) {
        uint32 decoded_value = sh_decode_huffman(bits, literal_tree, lit_code_bit_len, lit_arr_len);

        if(decoded_value == 256) break;
        if(decoded_value < 256) { //its a literal so just output it
            decompressed_data[data_index++] = decoded_value;
            continue;
        }

        if(decoded_value < 286 && decoded_value > 256) {
            uint32 base_index = decoded_value - 257;
            uint32 duplicate_length = base_lengths[base_index] + sh_png_read_bits(bits, base_length_extra_bit[base_index]);;

            uint32 distance_index = sh_decode_Huffman(bits, distance_tree, dist_tree_bit_len, dist_arr_len);
            uint32 distance_length = dist_bases[distance_index] + sh_png_read_bits(bits, dist_extra_bits[distance_index]);

            uint32 back_pointer_index = data_index - distance_length;
            while(duplicate_length--) {
                decompressed_data[Data_index++] = decompressed_data[back_pointer_index++];
            }

        }

    }

    *bytes_read = data_index;
    uint8 *fit_image = sh_memalloc(data_index);
    sh_memcpy(decompressed_data, fit_image, data_index);

    sh_memfree(decompressed_data);

    return fit_image;
}