/*
 bit_array.h
 project: bit array library
 author: Isaac Turner <turner.isaac@gmail.com>
 Copyright (C) 23-Dec-2011
 
 Project adapted from:
 http://stackoverflow.com/questions/2633400/c-c-efficient-bit-array
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BIT_ARRAY_HEADER_SEEN
#define BIT_ARRAY_HEADER_SEEN

#include <stdlib.h>

// 64 bit
typedef unsigned long word_t, word_addr_t, bit_index_t;
// 32 bit
//typedef unsigned int word_t, word_addr_t, bit_index_t;

typedef struct BIT_ARRAY BIT_ARRAY;

struct BIT_ARRAY {
  word_t* words;
  unsigned long num_of_bits;
};

BIT_ARRAY* bit_array_create(bit_index_t nbits);
void bit_array_free(BIT_ARRAY* bitarray);

void bit_array_set_bit(BIT_ARRAY* bitarr, bit_index_t b);
void bit_array_clear_bit(BIT_ARRAY* bitarr, bit_index_t b);

char bit_array_get_bit(BIT_ARRAY* bitarr, bit_index_t b);

void bit_array_fill_zeros(BIT_ARRAY* bitarr);
void bit_array_fill_ones(BIT_ARRAY* bitarr);

// Remember to free result!
char* bit_array_to_string(BIT_ARRAY* bitarr);

#endif
