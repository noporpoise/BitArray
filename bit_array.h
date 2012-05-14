/*
 bit_array.h
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 Adapted from: http://stackoverflow.com/a/2633584/431087
 author: Isaac Turner <turner.isaac@gmail.com>

 Copyright (c) 2011, Isaac Turner
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BIT_ARRAY_HEADER_SEEN
#define BIT_ARRAY_HEADER_SEEN

#include<stdio.h>

// 64 bit
typedef unsigned long word_t, word_addr_t, bit_index_t;
// 32 bit
//typedef unsigned int word_t, word_addr_t, bit_index_t;

struct BIT_ARRAY {
  word_t* words;
  unsigned long num_of_bits;
};


typedef struct BIT_ARRAY BIT_ARRAY;

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

// Get length of bit array
bit_index_t bit_array_length(BIT_ARRAY* bit_arr);

// Get this array as a string (remember to free the result!)
char* bit_array_to_string(BIT_ARRAY* bitarr);

// Copy a BIT_ARRAY struct and the data it holds - returns pointer to new object
BIT_ARRAY* bit_array_clone(BIT_ARRAY* bitarr);

// Copy the data it holds
// Handles overlaps properly if dest == src
//void bit_array_copy(BIT_ARRAY* dest, bit_index_t dstindx,
//                    BIT_ARRAY* src, bit_index_t srcindx, bit_index_t length);


// Enlarge or shrink the size of a bit array
// Shrinking will free some memory if it is large
// Enlarging an array will add zeros to the end of it
// returns 1 on success, 0 on failure
char bit_array_resize(BIT_ARRAY* bitarr, bit_index_t new_num_of_bits);

//
// Logic operators
//   destination and source bit arrays must be of the same length,
//   however, they may point to the same object
//
void bit_array_and(BIT_ARRAY* dest, BIT_ARRAY* src1, BIT_ARRAY* src2);
void bit_array_or(BIT_ARRAY* dest, BIT_ARRAY* src1, BIT_ARRAY* src2);
void bit_array_xor(BIT_ARRAY* dest, BIT_ARRAY* src1, BIT_ARRAY* src2);
void bit_array_not(BIT_ARRAY* dest, BIT_ARRAY* src);

// Compare two bit arrays by value stored
// arrays do not have to be the same length (e.g. 101 (5) > 00000011 (3))
int bit_array_compare(BIT_ARRAY* bitarr1, BIT_ARRAY* bitarr2);

// Return 0 if there was an overflow error
char bit_array_add(BIT_ARRAY* dest, BIT_ARRAY* src1, BIT_ARRAY* src2);

// If there is an overflow, bit array will be set to all 1s and 0 is returned
// Returns 0 if there was an overflow, 1 otherwise
char bit_array_increment(BIT_ARRAY* bitarr);

// If there is an underflow, bit array will be set to all 0s and 0 is returned
// Returns 0 if there was an underflow, 1 otherwise
char bit_array_decrement(BIT_ARRAY* bitarr);

// start index must be within the range of the bit array (0 <= x < length)
long bit_array_get_long(BIT_ARRAY* bitarr, bit_index_t start);
int  bit_array_get_int (BIT_ARRAY* bitarr, bit_index_t start);
char bit_array_get_char(BIT_ARRAY* bitarr, bit_index_t start);

// store and load bit array from a file
BIT_ARRAY* bit_array_load(FILE* f);
void bit_array_save(BIT_ARRAY* bitarr, FILE* f);

#endif
