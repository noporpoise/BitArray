/*
 bit_array.h
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 author: Isaac Turner <turner.isaac@gmail.com>
 Copyright (C) 23-Dec-2011
 
 Project adapted from:
 http://stackoverflow.com/a/2633584/431087
 
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

// 64 bit
typedef unsigned long word_t, word_addr_t, bit_index_t;
// 32 bit
//typedef unsigned int word_t, word_addr_t, bit_index_t;

typedef struct BIT_ARRAY BIT_ARRAY;

struct BIT_ARRAY {
  word_t* words;
  unsigned long num_of_bits;
};

// Constructor - create a new bit array of length nbits
BIT_ARRAY* bit_array_create(bit_index_t nbits);

// Destructor - free the memory used for a bit array
void bit_array_free(BIT_ARRAY* bitarray);

// set a bit (to 1) at position b
void bit_array_set_bit(BIT_ARRAY* bitarr, bit_index_t b);

// clear a bit (to 0) at position b
void bit_array_clear_bit(BIT_ARRAY* bitarr, bit_index_t b);

// Get the value of a bit (returns 0 or 1)
char bit_array_get_bit(BIT_ARRAY* bitarr, bit_index_t b);

// Set all bits in this array to 0
void bit_array_fill_zeros(BIT_ARRAY* bitarr);

// Set all bits in this array to 1
void bit_array_fill_ones(BIT_ARRAY* bitarr);

// Get this array as a string (remember to free the result!)
char* bit_array_to_string(BIT_ARRAY* bitarr);

// Copy a BIT_ARRAY struct and the data it holds - returns pointer to new object
BIT_ARRAY* bit_array_copy(BIT_ARRAY* bitarr);

// Enlarge or shrink the size of a bit array
// Shrinking will free some memory if it is large
// Enlarging an array will add zeros to the end of it
// returns 1 on success, 0 on failure
char bit_array_resize(BIT_ARRAY* bitarr, bit_index_t new_num_of_bits);

#endif
