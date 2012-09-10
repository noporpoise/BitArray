**C code for bit arrays**
https://github.com/noporpoise/BitArray/
revised BSD

Isaac Turner <turner.isaac@gmail.com>

About
=====

Bit arrays are arrays of bits (values zero or one).  This is a convenient and
efficient implementation for C/C++.  

Bit arrays are initialised to zero when created or extended.  All operations
have their bounds checked - an "Out of bounds" error is printed if you try to
access a bit with index >= length.

Please get in touch if you have suggestions / requests / bugs.  

== Build ==

To build and run the test code:

$ make
$ ./bit_array_test

== Methods ==

Add to the top of your code:

#include "bit_array.h"

You can then call the following methods:

// Constructor - create a new bit array of length nbits
BIT_ARRAY* bit_array_create(const bit_index_t nbits);

// Destructor - free the memory used for a bit array
void bit_array_free(BIT_ARRAY* bitarray);

// Get the value of a bit (returns 0 or 1)
char bit_array_get_bit(const BIT_ARRAY* bitarr, const bit_index_t b);

// set a bit (to 1) at position b
void bit_array_set_bit(BIT_ARRAY* bitarr, const bit_index_t b);

// clear a bit (to 0) at position b
void bit_array_clear_bit(BIT_ARRAY* bitarr, const bit_index_t b);

// Set multiple bits at once. 
// usage: bit_array_set_bits(bitarr, 3, {1,20,31});
void bit_array_set_bits(BIT_ARRAY* bitarr, const size_t n, ...);

// Clear multiple bits at once.
// usage: bit_array_clear_bits(bitarr, 3, {1,20,31});
void bit_array_clear_bits(BIT_ARRAY* bitarr, const size_t n, ...);

// Set all bits in this array to 0
void bit_array_clear_all(BIT_ARRAY* bitarr);

// Set all bits in this array to 1
void bit_array_set_all(BIT_ARRAY* bitarr);

// Get the number of bits set (hamming weight)
bit_index_t bit_array_num_bits_set(const BIT_ARRAY* bitarr);

// Get the number of bits not set (1 - hamming weight)
bit_index_t bit_array_num_bits_cleared(const BIT_ARRAY* bitarr);

// Put all the 0s before all the 1s
void bit_array_sort_bits(BIT_ARRAY* bitarr);

// Clear all the bits in a region
void bit_array_clear_region(BIT_ARRAY* bitarr,
                            const bit_index_t start, const bit_index_t length);

// Set all the bits in a region
void bit_array_set_region(BIT_ARRAY* bitarr,
                          const bit_index_t start, const bit_index_t length);

// Get length of bit array
bit_index_t bit_array_length(const BIT_ARRAY* bit_arr);

// Get this array as a string (remember to free the result!)
char* bit_array_to_string(const BIT_ARRAY* bitarr);

// Warning: does not null-terminate string!
void bit_array_cpy_to_string(const BIT_ARRAY* bitarr, char* str,
                             const bit_index_t start, const bit_index_t length);

// Get this array as a string (remember to free the result!)
void bit_array_print(const BIT_ARRAY* bitarr, FILE* fout);

// From string method (remember to free the result!)
BIT_ARRAY* bit_array_from_string(const char* bitstr);

// Copy a BIT_ARRAY struct and the data it holds - returns pointer to new object
BIT_ARRAY* bit_array_clone(const BIT_ARRAY* bitarr);

// Copy bits from one array to another
// Destination and source can be the same bit_array and
// src/dst regions can overlap
void bit_array_copy(BIT_ARRAY* dst, const bit_index_t dstindx,
                    const BIT_ARRAY* src, const bit_index_t srcindx,
                    const bit_index_t length);

// Enlarge or shrink the size of a bit array
// Shrinking will free some memory if it is large
// Enlarging an array will add zeros to the end of it
// returns 1 on success, 0 on failure
char bit_array_resize(BIT_ARRAY* bitarr, const bit_index_t new_num_of_bits);

//
// Logic operators
//   destination and source bit arrays must be of the same length,
//   however, they may point to the same object
//
void bit_array_and(BIT_ARRAY* dest, const BIT_ARRAY* src1, const BIT_ARRAY* src2);
void bit_array_or(BIT_ARRAY* dest, const BIT_ARRAY* src1, const BIT_ARRAY* src2);
void bit_array_xor(BIT_ARRAY* dest, const BIT_ARRAY* src1, const BIT_ARRAY* src2);
void bit_array_not(BIT_ARRAY* dest, const BIT_ARRAY* src);

// Compare two bit arrays by value stored
// arrays do not have to be the same length (e.g. 101 (5) > 00000011 (3))
int bit_array_cmp(const BIT_ARRAY* bitarr1, const BIT_ARRAY* bitarr2);

// start index must be within the range of the bit array (0 <= x < length)
uint64_t bit_array_word64(const BIT_ARRAY* bitarr, const bit_index_t start);
uint32_t bit_array_word32(const BIT_ARRAY* bitarr, const bit_index_t start);
uint16_t bit_array_word16(const BIT_ARRAY* bitarr, const bit_index_t start);
uint8_t  bit_array_word8 (const BIT_ARRAY* bitarr, const bit_index_t start);

// Find the index of the first bit that is set.  
// Returns 1 if a bit is set, otherwise 0
// Index of first set bit is stored in the integer pointed to by result
// If not bit is set result is not changed
char bit_array_find_first_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result);

// Shift array left/right with a given fill
void bit_array_shift_right(BIT_ARRAY* bitarr, const bit_index_t shift_dist,
                           const char fill);
void bit_array_shift_left(BIT_ARRAY* bitarr, const bit_index_t shift_dist,
                          const char fill);

//
// Read/Write bit_array to a file
//
// File format is [8 bytes: for number of elements in array][data]
// Number of bytes of data is: (int)((num_of_bits + 7) / 8)
//

// Saves bit array to a file
// returns the number of bytes written
bit_index_t bit_array_save(const BIT_ARRAY* bitarr, FILE* f);

// Reads bit array from a file
// returns bit array or NULL on failure
BIT_ARRAY* bit_array_load(FILE* f);

//
// Coming soon
//

//void bit_array_cycle_right(BIT_ARRAY* bitarr, const bit_index_t dist);
//void bit_array_cycle_left(BIT_ARRAY* bitarr, const bit_index_t dist);

== Revised BSD License ==

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

== Development ==

To do:
* cycle left / right
* reverse array, reverse region

Need re-writing:
* bit_array_add
* bit_array_increment
* bit_array_decrement

Also:
* write test cases for each method (test cases to go in bit_array_test.c)
