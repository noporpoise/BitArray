**C code for bit arrays**
=========================

https://github.com/noporpoise/BitArray/  
License: Public Domain, no warranty  
Isaac Turner <turner.isaac@gmail.com>

[![Build Status](https://travis-ci.org/noporpoise/BitArray.png?branch=master)](https://travis-ci.org/noporpoise/BitArray)

About
=====

Bit arrays are arrays of bits (values zero or one).  This is a convenient and
efficient implementation for C/C++.  Arrays can be enlarged or shrunk as needed.

Bit arrays are initialised to zero when created or extended.  All operations
have their bounds checked - an "Out of bounds" error is printed if you try to
access a bit with index >= length. Arrays of length 0 are permitted. Indices
must be >= 0.

Please get in touch if you have suggestions / requests / bugs.  

Adapted from: http://stackoverflow.com/a/2633584/431087

Build
=====

To build the library:

    make

To build and run the test code:

    make test

Using bit_array in your code
============================

You are welcome to bundle bit_array with your own code. Add to the top of your code:

    #include "bit_array.h"

Add to your compiler arguments:

    BIT_ARR_PATH=path/to/bit_array/
    gcc ... -I$(BIT_ARR_PATH) -L$(BIT_ARR_PATH) -lbitarr

Shorter function names are provided in `bar.h`, which can be included instead of
`bit_array.h`:

    #include "bar.h"

Thread safety
-------------

You cannot safely access the same BitArray in multiple threads at once. Use a
lock to protect BitArray objects. The same methods can be safely called in
separate threads as long as they are not accessing the same BitArray struct.

Basics
------

Constructor - create a new bit array of length nbits

    BIT_ARRAY* bit_array_create(bit_index_t nbits)

Destructor - free the memory used for a bit array

    void bit_array_free(BIT_ARRAY* bitarray)

Alternatively, allocate / free using an existing struct

    BIT_ARRAY* bit_array_alloc(BIT_ARRAY* bitarr, bit_index_t nbits)
    void bit_array_dealloc(BIT_ARRAY* bitarr)

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

    void bit_array_toggle_bit(BIT_ARRAY* bitarr, bit_index_t b)

Assign a value to a bit.  If `c != 0` then set bit; otherwise clear bit.

    void bit_array_assign_bit(BIT_ARRAY* bitarr, bit_index_t b, char c)

Fast MACROs
-----------

You can also use the following which are implemented as MACROs without bounds
checking:

    bit_array_get(BIT_ARRAY *arr, bit_index_t i)
    bit_array_set(BIT_ARRAY *arr, bit_index_t i)
    bit_array_clear(BIT_ARRAY *arr, bit_index_t i)
    bit_array_toggle(BIT_ARRAY *arr, bit_index_t i)
    bit_array_assign(BIT_ARRAY *arr, bit_index_t i, char c)

Get a word_t with the bottom `nbits` set to 1, the rest to 0:

    word_t BIT_MASK(int nbits)

Combine two words with a mask `((a & abits) | (b & ~abits))`:

    word_t BIT_MASK_MERGE(word_t a, word_t b, int abits)

Set, clear and toggle several bits
----------------------------------

Note: variable args are of type unsigned int

Set multiple bits at once. 

    void bit_array_set_bits(BIT_ARRAY* bitarr, size_t n, ...)
    
    // e.g. set bits 1,20,31:
    bit_array_set_bits(bitarr, 3, 1,20,31);

Clear multiple bits at once.

    void bit_array_clear_bits(BIT_ARRAY* bitarr, size_t n, ...)

    // e.g. clear bits 1,20,31:
    bit_array_clear_bits(bitarr, 3, 1,20,31);

Toggle multiple bits at once

    void bit_array_toggle_bits(BIT_ARRAY* bitarr, size_t n, ...)

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

Toggle all the bits in the region `start` to `start+length-1` inclusive

    void bit_array_toggle_region(BIT_ARRAY* bitarr,
                                 bit_index_t start, bit_index_t length)

Set, clear and toggle all bits
------------------------------

Set all bits in this array to 0

    void bit_array_clear_all(BIT_ARRAY* bitarr)

Set all bits in this array to 1

    void bit_array_set_all(BIT_ARRAY* bitarr)

Set all 1 bits to 0, and all 0 bits to 1 (i.e. flip all the bits)

    void bit_array_toggle_all(BIT_ARRAY* bitarr)

Get / set a word
----------------

Get a word of a given size.  First bit is in the least significant bit position.
Index `start` must be within the range of the bit array (0 <= x < length)

    uint64_t bit_array_get_word64(const BIT_ARRAY* bitarr, bit_index_t start)
    uint32_t bit_array_get_word32(const BIT_ARRAY* bitarr, bit_index_t start)
    uint16_t bit_array_get_word16(const BIT_ARRAY* bitarr, bit_index_t start)
    uint8_t  bit_array_get_word8 (const BIT_ARRAY* bitarr, bit_index_t start)
    uint64_t bit_array_get_wordn (const BIT_ARRAY* bitarr, bit_index_t start, int n)

Set 64 bits at once from a particular start position

    void bit_array_set_word64(BIT_ARRAY* bitarr, bit_index_t start, uint64_t word)
    void bit_array_set_word32(BIT_ARRAY* bitarr, bit_index_t start, uint32_t word)
    void bit_array_set_word16(BIT_ARRAY* bitarr, bit_index_t start, uint16_t word)
    void bit_array_set_word8 (BIT_ARRAY* bitarr, bit_index_t start, uint8_t word)
    void bit_array_set_wordn (BIT_ARRAY* bitarr, bit_index_t start, uint64_t word, int n)

Count bits set
--------------

Get the number of bits set (hamming weight)

    bit_index_t bit_array_num_bits_set(const BIT_ARRAY* bitarr)

Get the number of bits set in on array and not the other.  This is equivalent
to hamming weight of the XOR of the two arrays.
e.g. 10101 vs 00111 => hamming distance 2 (XOR is 10010)

    bit_index_t bit_array_hamming_distance(const BIT_ARRAY* arr1,
                                           const BIT_ARRAY* arr2)

Get the number of bits not set (`length - hamming weight`)

    bit_index_t bit_array_num_bits_cleared(const BIT_ARRAY* bitarr)

Find the index of the first bit that is set.
Returns 1 if a bit is set, otherwise 0.
Index of first set bit is stored in the integer pointed to by `result`.
If no bits are set, value at `result` is not changed and zero is returned.

    char bit_array_find_first_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result)

Find the index of the first bit that is clear.
Returns 1 if a bit is clear, otherwise 0.
Index of first clear bit is stored in the integer pointed to by `result`.
If no bits are clear, zero is returned.

    char bit_array_find_first_clear_bit(const BIT_ARRAY* bitarr, bit_index_t* result)

Find the index of the last bit that is set.
Returns 1 if a bit is set, otherwise 0.
Index of last set bit is stored in the integer pointed to by `result`.
If no bits are set, value at `result` is not changed and zero is returned.

    char bit_array_find_last_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result)

Find the index of the last bit that is NOT set.
Returns 1 if a bit is zero, otherwise 0.
Index of last zero bit is stored in the integer pointed to by `result`.
If no bits are zero, value at `result` is not changed and zero is returned.

    char bit_array_find_last_clear_bit(const BIT_ARRAY* bitarr, bit_index_t* result)

Find the index of the next bit that is set, at or after `offset`.
Returns 1 if a bit is set, otherwise 0.
Index of next set bit is stored in the integer pointed to by `result`.
If no next bit is set, value at `result` is not changed and 0 is returned.

    char bit_array_find_next_set_bit(const BIT_ARRAY* bitarr, bit_index_t offset,
                                     bit_index_t* result)

Find the index of the next bit that is clear, at or after `offset`.
Returns 1 if a bit is clear, otherwise 0.
Index of next clear bit is stored in the integer pointed to by `result`.
If no next bit is clear, 0 is returned.

    char bit_array_find_next_clear_bit(const BIT_ARRAY* bitarr, bit_index_t offset,
                                       bit_index_t* result)

Find the index of the previous bit that is set, before `offset`.
Note: 'before' does not include `offset`.
Returns 1 if a bit is set, otherwise 0
Index of previous set bit is stored in the integer pointed to by `result`
If no previous bit is set, value at `result` is not changed

    char bit_array_find_prev_set_bit(const BIT_ARRAY* bitarr, bit_index_t offset,
                                     bit_index_t* result)

Find the index of the previous bit that is NOT set, before `offset`.
Note: 'before' does not include `offset`.
Returns 1 if a bit is clear, otherwise 0
Index of previous zero bit is stored in the integer pointed to by `result`
If no previous bit is zero, value at `result` is not changed

    char bit_array_find_prev_clear_bit(const BIT_ARRAY* bitarr, bit_index_t offset,
                                       bit_index_t* result)

Parity / Permutation
--------------------

Get parity: returns 1 if odd number of bits set, 0 if even.

    char bit_array_parity(const BIT_ARRAY* bitarr)

Get the next permutation of an array with a fixed size and given number of
bits set.  Also known as next lexicographic permutation.
Given a bit array find the next lexicographic orginisation of the bits
Number of possible combinations given by `size choose bits_set` where `bits_set`
is the result of `bit_array_num_bits_set(bitarr)`. Example:
00011 -> 00101 -> 00110 -> 01001 -> 01010 ->
01100 -> 10001 -> 10010 -> 10100 -> 11000 -> 00011 (back to start)

    void bit_array_next_permutation(BIT_ARRAY* bitarr)

Sorting
-------

Put all the 0s before all the 1s

    void bit_array_sort_bits(BIT_ARRAY* bitarr)

Put all the 1s before all the 0s

    void bit_array_sort_bits_rev(BIT_ARRAY* bitarr)


String and printing functions
-----------------------------

To convert to/from string representations of an array, '1' and '0' are used by
default as on and off.

Create a bit array from a string of '0's and '1's e.g. "01001010110".

    void bit_array_from_str(BIT_ARRAY* bitarr, const char* bitstr)

Construct a BIT_ARRAY from a substring with given on and off characters.
`left_to_right` determines the order in which bits are printed.
Terminates string with '\0'.

    void bit_array_from_substr(BIT_ARRAY* bitarr, bit_index_t offset,
                               const char* str, size_t len,
                               const char *on, const char *off, char left_to_right)

To string method. Takes a char array to write to.
`str` must be bitarr->num_of_bits+1 in length.
Terminates string with '\0'.

    char* bit_array_to_str(const BIT_ARRAY* bitarr, char* str)

To construct a string in reverse (highest bit on the left, lowest on the right)

    bit_array_to_str_rev(const BIT_ARRAY* bitarr, char* str)

Get a string representations for a given region, using given on/off characters.
`left_to_right` determines the order in which bits are printed.
Note: does not null-terminate.

    void bit_array_to_substr(const BIT_ARRAY* bitarr,
                             bit_index_t start, bit_index_t length,
                             char* str, char on, char off, char left_to_right)

Print this array to a file stream.  Prints '0's and '1'.  Doesn't print newline.

    void bit_array_print(const BIT_ARRAY* bitarr, FILE* fout)

Print a string representations for a given region, using given on/off characters.
`left_to_right` determines the order in which bits are printed.

    void bit_array_print_substr(const BIT_ARRAY* bitarr,
                                bit_index_t start, bit_index_t length,
                                FILE* fout, char on, char off, char left_to_right)

Decimal
-------

Get bit array as decimal str e.g. 0b1101 -> "13".
`len` is the length of str char array.  `bit_array_to_decimal()` write at most
`len-1` chars to `str`.  Returns the number of characters that would have been
written to str -- return is the same as strlen(str) upon success.

    size_t bit_array_to_decimal(const BIT_ARRAY *bitarr, char *str, size_t len)

Example usage:

    char str[10];
    size_t len = bit_array_to_decimal(arr, str, 10);

    if(len > 9)
    {
      // str wasn't big enough
    }

Get bit array from decimal str (e.g. "13" -> 0b1101).
Returns number of characters used

    size_t bit_array_from_decimal(BIT_ARRAY *bitarr, const char* decimal)

Example usage:

    char *str = "1234";
    BIT_ARRAY *bitarr = bit_array_create(0);

    size_t len = bit_array_from_decimal(bitarr, str);

    if(len < strlen(str))
    {
      // Parsing ended prematurely (non-numeric characters encountered)
    }


Hexidecimal
-----------

Loads array from hex string
Returns the number of bits loaded (will be chars rounded up to multiple of 8)
(0 on failure)

    bit_index_t bit_array_from_hex(BIT_ARRAY* bitarr, bit_index_t offset,
                                   const char* str, size_t len)

Returns number of characters written

    size_t bit_array_to_hex(const BIT_ARRAY* bitarr,
                            bit_index_t start, bit_index_t length,
                            char* str, char uppercase)

Print bit array as hex

    size_t bit_array_print_hex(const BIT_ARRAY* bitarr,
                               bit_index_t start, bit_index_t length,
                               FILE* fout, char uppercase)

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

Shift array left/right with a given `fill` (0 or 1)

    void bit_array_shift_right(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)
    void bit_array_shift_left(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)

To shift and add digits instead of losing data, use the extend left shift
function:

    void bit_array_shift_left_extend(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)

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

Comparison functions return:
* > 0 iff bitarr1 > bitarr2
*   0 iff bitarr1 == bitarr2
* < 0 iff bitarr1 < bitarr2

Compare two bit arrays by value stored, with index 0 being the Least
Significant Bit (LSB). 

Arrays do not have to be the same length.
Example: ..0101 (5) > ...0011 (3) [index 0 is LSB at right hand side].

    int bit_array_cmp(const BIT_ARRAY* bitarr1, const BIT_ARRAY* bitarr2)

Compare two bit arrays by value stored, with index 0 being the Most Significant
Bit (MSB). Sorts on length if all zeros: (0,0) < (0,0,0)

Arrays do not have to be the same length.
Example: 10.. > 01.. [index 0 is MSB at left hand side]

    int bit_array_cmp_big_endian(const BIT_ARRAY* bitarr1, const BIT_ARRAY* bitarr2)

Compare `bitarr` with `(bitarr2 << pos)`.  Does not use array length, only value
stored.

    int bit_array_cmp_words(const BIT_ARRAY *bitarr,
                            bit_index_t pos, const BIT_ARRAY *bitarr2)

Compare value stored against an unsigned long (treats `bitarr` as large unsigned
integer type):

    int bit_array_compare_num(BIT_ARRAY* bitarr, unsigned long value)

Arithmetic
----------

Bit arrays can be interpretted as arbitrarily large unsigned integers. To do this
the bit at index 0 is treated as the least significant bit.  BitArrays provide
functions for arithmetic between a BitArray & a long, and between BitArrays.

Get the value of this number in an unsigned long.
Returns 1 on sucess, 0 if value in array is too big.

    char bit_array_as_num(BIT_ARRAY* bitarr, unsigned long* result)

(Note: see also `bit_array_compare_num(BIT_ARRAY*, unsigned long)`)

Add to an array.  `bitarr` will be extended if needed.

    void bit_array_add_uint64(BIT_ARRAY* bitarr, unsigned long value)

Add `add` to `bitarr` at `pos` -- same as:
  bitarr + (add << pos)
where pos can be bigger than the length of the array (bitarr will be resized)

    void bit_array_add_word(BIT_ARRAY *bitarr, bit_index_t pos, uint64_t add)

Add `add << pos` to `bitarr`

    void bit_array_add_words(BIT_ARRAY *bitarr, bit_index_t pos, BIT_ARRAY *add)

Subtract from an array. If `value` is greater than `bitarr`, `bitarr` is not
changed and `0` is returned. Returns `1` on success, `0` if `value > bitarr`

    char bit_array_sub_uint64(BIT_ARRAY* bitarr, unsigned long value)

Minus `minus << pos` from `bitarr`

    char bit_array_sub_words(BIT_ARRAY* bitarr, bit_index_t pos,
                               BIT_ARRAY* minus)

Multiply by some value

    void bit_array_mul_uint64(BIT_ARRAY *bitarr, uint64_t multiplier)

Add two bit arrays together and store the result.  `src1` and `src2` do not have
to be the same length. `src1`, `src2` and `dst` can all be the same or different
`BIT_ARRAY`s. If `dst` is shorter than either of `src1` or `src2`, it is enlarged
to be as long as the longest.

    void bit_array_add(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)

Subtract on BIT_ARRAY from another. `src1`, `src2` and `dst` can all be the same
or different `BIT_ARRAY`s. If dst is shorter than src1, it will be extended to
be as long as `src1`. `src1` must be greater than or equal to `src2` (`src1 >= src2`).

    void bit_array_subtract(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)

dst = src1 * src2
Pointers cannot all point to the same BIT_ARRAY

    void bit_array_multiply(BIT_ARRAY *dst, BIT_ARRAY *src1, BIT_ARRAY *src2)

Divide a BitArray by a BitArray; returns:
* `quotient = dividend / divisor`
* `dividend = dividend % divisor`

Dividend is used to return the remainder.

    void bit_array_divide(BIT_ARRAY *dividend, BIT_ARRAY *quotient,
                          BIT_ARRAY *divisor)


Read/Write bit_array to a file
------------------------------

File format is [8 bytes: for number of elements in array][data].
Number of bytes of data is: `(int)((num_of_bits + 7) / 8)` -- i.e.
`roundup(num_of_bits/8)`

Saves bit array to a file.  Returns the number of bytes written

    bit_index_t bit_array_save(const BIT_ARRAY* bitarr, FILE* f)

Reads bit array from a file. `bitarr` is resized and filled with data from the file.
Returns 1 on success, 0 on failure.

    char bit_array_load(BIT_ARRAY* bitarr, FILE* f)


Hash Value
----------

Get a hash value for this array. Pass `seed` as `0` on first call, pass previous
hash value if rehashing due to a collision. Uses Bob Jenkins hash lookup3 function
(http://burtleburtle.net/bob/hash/index.html)

    uint64_t bit_array_hash(const BIT_ARRAY* bitarr, uint64_t seed)

Randomness
----------

Set bits randomly with probability prob (where `0 <= prob <= 1`)

    void bit_array_random(BIT_ARRAY* bitarr, float prob)

Shuffle the bits in an array randomly

    void bit_array_shuffle(BIT_ARRAY* bitarr)

    // e.g. If you want exactly 9 random bits set in an array, use:
    bit_array_set_region(arr, 0, 9); // set the first 9 bits
    bit_array_shuffle(arr);          // shuffle the array

Useful functions
----------------

The file `bit_macros.h` contains many useful macros for bit arrays. Simple bit
array functions can be implemented with this file alone.

Generalised 'binary to string' function.
Adds bits to the string in order of lsb to msb
e.g. 0b11010 (26 in decimal) would come out as "01011"

    char* bit_array_word2str(const void *ptr, size_t num_of_bits, char *str);

    // Same as above but in reverse
    char* bit_array_word2str_rev(const void *ptr, size_t num_of_bits, char *str);

For those who hate all that typing: the file "bar.h" contains macros to
supply short "bar*" names for the most used bit array operations.
This is meant to be similar to the "str*" function names for string
manipulation.

Constants
---------

`BIT_INDEX_MIN` and `BIT_INDEX_MAX` define the min and max values of datatype
`bit_index_t`.  These are defined as `0` and `2^63 - 1`.

Contributing
============

Please feel free to submit issues and pull requests. I appreciate bug reports.

Methods are named:
* `_name()` indicates only used internally
* `bit_array_name()` exported as is

Testing on different platforms is especially appreciated. I only have access
to Mac OS X and Linux.

License
=======

This software is in the *Public Domain*. That means you can do whatever you like
with it. That includes being used in proprietary products without attribution or
restrictions. There are no warranties and there may be bugs. 

Formally we are using CC0 - a Creative Commons license to place this work in the
public domain. A copy of CC0 is in the LICENSE file. 

    "CC0 is a public domain dedication from Creative Commons. A work released
    under CC0 is dedicated to the public domain to the fullest extent permitted
    by law. If that is not possible for any reason, CC0 also provides a lax,
    permissive license as a fallback. Both public domain works and the lax
    license provided by CC0 are compatible with the GNU GPL."
      - http://www.gnu.org/licenses/license-list.html#CC0

Development
===========

To do:
* search function: `int bit_array_search(const BIT_ARRAY *arr, const BIT_ARRAY *query);`
* windows support
* 32 bit support
* faster multiply / divide? (i.e. Karatsuba)
