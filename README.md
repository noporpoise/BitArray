**C code for bit arrays**
=========================

https://github.com/noporpoise/BitArray/

License: revised BSD

Isaac Turner <turner.isaac@gmail.com>

About
=====

Bit arrays are arrays of bits (values zero or one).  This is a convenient and
efficient implementation for C/C++.  Arrays can be enlarged or shrunk as needed.

Bit arrays are initialised to zero when created or extended.  All operations
have their bounds checked - an "Out of bounds" error is printed if you try to
access a bit with index >= length. Arrays of length 0 are permitted. Indices
must be >= 0. This implementation is not thread-safe.

Please get in touch if you have suggestions / requests / bugs.  

Build
=====

To build and run the test code:

    make
    ./bit_array_test

Using bit_array in your code
============================

You are welcome to bundle bit_array with your own code. Add to the top of your code:

    #include "bit_array.h"

Add to your compiler arguments

    BIT_ARR_PATH=path/to/bit_array/
    gcc ... -I$(BIT_ARR_PATH) -L$(BIT_ARR_PATH) -libbitarr

Basics
------

Constructor - create a new bit array of length nbits

    BIT_ARRAY* bit_array_create(bit_index_t nbits)

Destructor - free the memory used for a bit array

    void bit_array_free(BIT_ARRAY* bitarray)

Get length of bit array

    bit_index_t bit_array_length(const BIT_ARRAY* bit_arr)

Change the size of a bit array. Enlarging an array will add zeros to
the end of it. Returns 1 on success, 0 on failure (e.g. not enough memory)

    char bit_array_resize(BIT_ARRAY* bitarr, bit_index_t new_num_of_bits)

Set/Get bits
------------

Get the value of a bit (returns 0 or 1)

    char bit_array_get_bit(const BIT_ARRAY* bitarr, bit_index_t b)

Set a bit (to 1) at position `b`

    void bit_array_set_bit(BIT_ARRAY* bitarr, bit_index_t b)

Clear a bit (to 0) at position `b`

    void bit_array_clear_bit(BIT_ARRAY* bitarr, bit_index_t b)

Toggle a bit. If bit is 0 change to 1; if bit is 1 change to 0. Also known as
a complement function.

    void bit_array_toggle_bit(BIT_ARRAY* bitarr, bit_index_t b);

Assign a value to a bit.  If `c != 0` then set bit; otherwise clear bit.

    void bit_array_assign_bit(BIT_ARRAY* bitarr, bit_index_t b, char c)

Set, clear and toggle several bits
----------------------------------

Set multiple bits at once. 

    void bit_array_set_bits(BIT_ARRAY* bitarr, size_t n, ...)
    
    // e.g. set bits 1,20,31:
    bit_array_set_bits(bitarr, 3, 1,20,31);

Clear multiple bits at once.

    void bit_array_clear_bits(BIT_ARRAY* bitarr, size_t n, ...)

    // e.g. clear bits 1,20,31:
    bit_array_clear_bits(bitarr, 3, 1,20,31);

Toggle multiple bits at once

    void bit_array_toggle_bits(BIT_ARRAY* bitarr, size_t n, ...);

    // e.g. toggle bits 1,20,31:
    bit_array_toggle_bits(bitarr, 3, 1,20,31);

Set, clear and toggle a region
------------------------------

Clear all the bits in the region `start` to `start+length-1` inclusive

    void bit_array_clear_region(BIT_ARRAY* bitarr,
                                bit_index_t start, bit_index_t length)

Set all the bits in the region `start` to `start+length-1` inclusive

    void bit_array_set_region(BIT_ARRAY* bitarr,
                              bit_index_t start, bit_index_t length)

Toggle all the bits in a region

    void bit_array_toggle_region(BIT_ARRAY* bitarr,
                                 bit_index_t start, bit_index_t length)

Set, clear and toggle all bits
------------------------------

Set all bits in this array to 0

    void bit_array_clear_all(BIT_ARRAY* bitarr)

Set all bits in this array to 1

    void bit_array_set_all(BIT_ARRAY* bitarr)

Set all 1 bits to 0, and all 0 bits to 1 -- i.e. flip all the bits

    void bit_array_toggle_all(BIT_ARRAY* bitarr)

Get / set a word
----------------

Get a word of a given size.  First bit is in the least significant bit position
start index must be within the range of the bit array (0 <= x < length)

    uint64_t bit_array_word64(const BIT_ARRAY* bitarr, bit_index_t start)
    uint32_t bit_array_word32(const BIT_ARRAY* bitarr, bit_index_t start)
    uint16_t bit_array_word16(const BIT_ARRAY* bitarr, bit_index_t start)
    uint8_t  bit_array_word8 (const BIT_ARRAY* bitarr, bit_index_t start)

Set 64 bits at once from a particular start position

    void bit_array_set_word64(BIT_ARRAY* bitarr, bit_index_t start, uint64_t word)

Count bits set
--------------

Get the number of bits set (hamming weight)

    bit_index_t bit_array_num_bits_set(const BIT_ARRAY* bitarr)

Get the number of bits set in on array and not the other.  This is equivalent
to hamming weight of the XOR of the two arrays.
e.g. 10101 vs 00111 => hamming distance 2 (XOR is 10010)

    bit_index_t bit_array_hamming_distance(const BIT_ARRAY* arr1,
                                           const BIT_ARRAY* arr2)

Get the number of bits not set (length - hamming weight)

    bit_index_t bit_array_num_bits_cleared(const BIT_ARRAY* bitarr)

Find the index of the first bit that is set.
Returns 1 if a bit is set, otherwise 0.
Index of first set bit is stored in the integer pointed to by `result`.
If no bit is set `result` is not changed and zero is returned.

    char bit_array_find_first_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result)

Find the index of the last bit that is set.
Returns 1 if a bit is set, otherwise 0.
Index of last set bit is stored in the integer pointed to by `result`.
If no bit is set result is not changed and zero is returned.

    char bit_array_find_last_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result)

Get parity: returns 1 if odd number of bits set, 0 if even.

    char bit_array_parity(const BIT_ARRAY* bitarr)

Get the next permutation of an array with a fixed size and given number of
bits set.  Also known as next lexicographic permutation.
Given a bit array find the next lexicographic orginisation of the bits
Number of possible combinations given by `size choose bits_set` where `bits_set`
is the result of `bit_array_num_bits_set(bitarr)`. Example:
00011 -> 00101 -> 00110 -> 01001 -> 01010 ->
01100 -> 10001 -> 10010 -> 10100 -> 11000 -> 00011 (back to start)

    void bit_array_next_permutation(BIT_ARRAY* bitarr);

Sorting
-------

Put all the 0s before all the 1s

    void bit_array_sort_bits(BIT_ARRAY* bitarr)

Put all the 1s before all the 0s

    void bit_array_rev_sort_bits(BIT_ARRAY* bitarr)


String and printing functions
-----------------------------

To convert to/from string representations of an array, '1' and '0' are used by
default as on and off.

Create a bit array from a string of '0's and '1's e.g. "01001010110".

    void bit_array_from_str(BIT_ARRAY* bitarr, const char* bitstr)

Construct a BIT_ARRAY from a substring with given on and off characters.
Reverse reads from highest to lowest -- this is useful for loading binary numbers.

    void bit_array_from_substr(BIT_ARRAY* bitarr, const char* str, size_t len,
                               char on, char off, char reverse)

To string method. Takes a char array to write to.
`str` must be bitarr->num_of_bits+1 in length.
Terminates string with '\0'.

    char* bit_array_to_str(const BIT_ARRAY* bitarr, char* str)

Get a string representations for a given region, using given on/off characters.
Does not null-terminate string.
Reverse prints from highest to lowest -- this is useful for writing binary numbers.

    void bit_array_to_substr(const BIT_ARRAY* bitarr, char* str,
                             bit_index_t start, bit_index_t length,
                             char on, char off, char reverse)

Print this array to a file stream.  Prints '0's and '1'.  Doesn't print newline.

    void bit_array_print(const BIT_ARRAY* bitarr, FILE* fout)

Print a string representations for a given region, using given on/off characters.
Reverse prints from highest to lowest -- this is useful printing for binary numbers

    void bit_array_print_substr(const BIT_ARRAY* bitarr, FILE* fout,
                                bit_index_t start, bit_index_t length,
                                char on, char off, char reverse)

Clone/copy
----------

Copy a BIT_ARRAY struct and the data it holds - returns pointer to new object

    BIT_ARRAY* bit_array_clone(const BIT_ARRAY* bitarr)

Copy bits from one array to another.
Destination and source can be the same bit_array and src/dst regions can overlap

    void bit_array_copy(BIT_ARRAY* dst, bit_index_t dstindx,
                        const BIT_ARRAY* src, bit_index_t srcindx,
                        bit_index_t length)

Logic operators and shifts
--------------------------

Destination and source bit arrays must be of the same length, however they may
point to the same object

    void bit_array_and(BIT_ARRAY* dest, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
    void bit_array_or(BIT_ARRAY* dest, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
    void bit_array_xor(BIT_ARRAY* dest, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
    void bit_array_not(BIT_ARRAY* dest, const BIT_ARRAY* src)

'Flip' the bits in a particular regions -- apply `not`

    void bit_array_complement_region(BIT_ARRAY* dst, bit_index_t start, bit_index_t len)

Shift array left/right with a given `fill` (0 or 1)

    void bit_array_shift_right(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)
    void bit_array_shift_left(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)

Circular or cycle shifts.  Bits wrap around once shifted off the end

    void bit_array_cycle_right(BIT_ARRAY* bitarr, bit_index_t dist)
    void bit_array_cycle_left(BIT_ARRAY* bitarr, bit_index_t dist)

Interleave bits
---------------
Copy bits from two arrays into another, alternating between taking a bit from each.
In other words, two arrays a,b,c,d and 1,2,3,4 -> a,1,b,2,c,3,d,4. Examples:
* 0011 0000 -> 00001010
* 1111 0000 -> 10101010
* 0101 1010 -> 01100110

`dst` cannot point to the same bit array as `src1` or `src2`. However `src1` and
`src2` may point to the same bit array.

    void bit_array_interleave(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)

Reverse
-------

Reverse the whole array or part of it.

    void bit_array_reverse(BIT_ARRAY* bitarr)
    void bit_array_reverse_region(BIT_ARRAY* bitarr,
                                  bit_index_t start, bit_index_t length)

Comparing
---------

Compare two bit arrays by value stored, with index 0 being the Least
Significant Bit (LSB). Returns:
* >0 iff bitarr1 > bitarr2
*  0 iff bitarr1 == bitarr2
* <0 iff bitarr1 < bitarr2

Arrays do not have to be the same length.
Example: ..0101 (5) > ...0011 (3) [index 0 is LSB at right hand side].

    int bit_array_cmp(const BIT_ARRAY* bitarr1, const BIT_ARRAY* bitarr2)

Compare two bit arrays by value stored, with index 0 being the Most Significant
Bit (MSB). Sorts on length if all zeros: (0,0) < (0,0,0)
Returns:
* >0 iff bitarr1 > bitarr2
*  0 iff bitarr1 == bitarr2
* <0 iff bitarr1 < bitarr2

Arrays do not have to be the same length.
Example: 10.. > 01.. [index 0 is MSB at left hand side]

    int bit_array_other_endian_cmp(const BIT_ARRAY* bitarr1, const BIT_ARRAY* bitarr2)


Adding / Subtracting
--------------------

To be interpretted as a number, bit at index 0 is treated as the least
significant bit.

Add two bit arrays together and store the result.  `src1` and `src2` do not have
to be the same length. `src1`, `src2` and `dst` can all be the same or different
`BIT_ARRAY`s. If `dst` is shorter than either of `src1` or `src2`, it is enlarged
to be as long as the longest.

    void bit_array_add(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)

Subtract on BIT_ARRAY from another. `src1`, `src2` and `dst` can all be the same
or different `BIT_ARRAY`s. If dst is shorter than src1, it will be extended to
be as long as `src1`. `src1` must be greater than or equal to `src2` (`src1 >= src2`).

    void bit_array_subtract(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)

Add one to a `BIT_ARRAY`.
If `dst` is too small it will be resized to hold the highest set bit.

    void bit_array_increment(BIT_ARRAY* bitarr)

Subtract one to a `BIT_ARRAY`. If there is an underflow, bit array will be set
to all 0s and 0 is returned. Returns 1 on success, 0 if there was an underflow.

    char bit_array_decrement(BIT_ARRAY* bitarr)


Read/Write bit_array to a file
------------------------------

File format is [8 bytes: for number of elements in array][data].
Number of bytes of data is: (int)((num_of_bits + 7) / 8) -- i.e. roundup(num_of_bits/8)

Saves bit array to a file.  Returns the number of bytes written

    bit_index_t bit_array_save(const BIT_ARRAY* bitarr, FILE* f)

Reads bit array from a file.  Returns bit array or NULL on failure

    BIT_ARRAY* bit_array_load(FILE* f)

Hash Value
----------

Get a hash value for this array. Pass `seed` as `0` on first call, pass previous
hash value if rehashing due to a collision. Uses Bob Jenkins hash lookup3 function
-- see http://burtleburtle.net/bob/hash/index.html

    uint64_t bit_array_hash(const BIT_ARRAY* bitarr, uint64_t seed)

Randomness
----------

Set bits randomly with probability prob : `0 <= prob <= 1`

    void bit_array_random(BIT_ARRAY* bitarr, float prob)

Shuffle the bits in an array randomly

    void bit_array_shuffle(BIT_ARRAY* bitarr)

    // e.g. If you want exactly 9 random bits set in an array, use:
    bit_array_set_region(arr, 0, 9);
    bit_array_shuffle();

Constants
---------

`BIT_INDEX_MIN` and `BIT_INDEX_MAX` define the min and max values of datatype
`bit_index_t`.  These are defined as `0` and `2^63 - 1`.

Coming soon
-----------

Under development

    void bit_array_next_permutation(BIT_ARRAY* bitarr)

Revised BSD License
===================

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

Development
===========

To do:
* write more test cases (test cases to go in bit_array_test.c)
* optimisations?
