/*
 bit_array.c
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 Adapted from: http://stackoverflow.com/a/2633584/431087
 author: Isaac Turner <turner.isaac@gmail.com>

 Copyright (c) 2012, Isaac Turner
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

// 64 bit words
// Array length can be zero
// Unused top bits must be zero

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h> // memset

#include "bit_array.h"

#ifdef DEBUG
#include <execinfo.h>
void print_trace()
{
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace(array, 10);
  strings = backtrace_symbols (array, size);

  fprintf(stderr, "Obtained %zd stack frames.\n", size);

  for(i = 0; i < size; i++)
    fprintf(stderr, "%s\n", strings[i]);

  free(strings);
}
#endif

#define MIN(a, b)  (((a) <= (b)) ? (a) : (b))
#define MAX(a, b)  (((a) >= (b)) ? (a) : (b))

// Make this a power of two
#define INIT_CAPACITY_WORDS 2

// WORD_SIZE is the number of bits per word
// sizeof gives size in bytes (8 bits per byte)
//word_offset_t WORD_SIZE = (sizeof(word_t) * 8);
#define WORD_SIZE (sizeof(word_t) * 8)

// word of all 1s
#define WORD_MAX  (~(word_t)0)
#define BIT_MASK(length)  (WORD_MAX >> (WORD_SIZE-(length)))

#if defined(_WIN32)
#define TRAILING_ZEROS(x) _BitScanForward(x)
#define LEADING_ZEROS(x) _BitScanReverse(x)
#else
#define TRAILING_ZEROS(x) __builtin_ctzl(x)
#define LEADING_ZEROS(x) __builtin_clzl(x)
#endif

//
// For internal use
//

struct BIT_ARRAY {
  word_t* words;
  bit_index_t num_of_bits;
  // For more efficient allocation we use realloc only to double size --
  // not for adding every word.  Initial size is INIT_CAPACITY_WORDS.
  word_addr_t capacity_in_words;
};

char x[] = {1,2};

word_t reverse_table[256] =
 {0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
  0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
  0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
  0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
  0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
  0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
  0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
  0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
  0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
  0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
  0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
  0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
  0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
  0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
  0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
  0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
  0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

// Index of word
word_addr_t bindex(bit_index_t b) { return (b / WORD_SIZE); }

// Offset within a word (values up to 64 most likely)
word_offset_t boffset(bit_index_t b) { return (b % WORD_SIZE); }

// Number of words required to store a given number of bits
// 0..WORD_SIZE -> 1
// WORD_SIZE+1..2*WORD_SIZE -> 2 etc.
word_addr_t nwords(bit_index_t b)
{
  return (b == 0 ? 1 : (b + WORD_SIZE - 1) / WORD_SIZE);
}

// Number of bytes required to store a given number of bits
word_addr_t nbytes(bit_index_t b)
{
  return (b + 7) / 8;
}

word_offset_t _top_word_index(bit_index_t b)
{
  return (b == 0 ? 0 : nwords(b)-1);
}

word_offset_t _bits_in_top_word(bit_index_t b)
{
  return (b == 0 ? 0 : boffset(b - 1) + 1);
}

// Mostly used for debugging
void _bit_array_print_word(word_t word, FILE* out)
{
  word_offset_t i;
  for(i = 0; i < WORD_SIZE; i++)
  {
    fprintf(out, "%c", ((word >> i) & (word_t)0x1) == 0 ? '0' : '1');
  }
}

#ifdef DEBUG
// Mostly used for debugging
void _bit_array_check_top_word(BIT_ARRAY* bitarr)
{
  word_addr_t word_index = _top_word_index(bitarr->num_of_bits);
  word_t top_word = bitarr->words[word_index];

  word_offset_t num_of_bits_in_top_word = _bits_in_top_word(bitarr->num_of_bits);

  if(num_of_bits_in_top_word == WORD_SIZE)
  {
    return;
  }
  else if(num_of_bits_in_top_word > 0)
  {
    top_word >>= num_of_bits_in_top_word;
  }

  if(top_word > 0)
  {
    fprintf(stderr, "%s:%i:_bit_array_check_top_word(): Fail -- "
                    "expected %i bits in top word %lu)\n", __FILE__, __LINE__,
            (int)num_of_bits_in_top_word, (unsigned long)word_index);
    _bit_array_print_word(top_word, stderr);
    fprintf(stderr, "\n");
    print_trace();
    exit(EXIT_FAILURE);
  }
}
#endif

//
// Internal functions
//

// Reverse a word
word_t _bit_array_reverse_word(word_t word)
{
  word_t reverse = (reverse_table[(word)       & 0xff] << 56) |
                   (reverse_table[(word >>  8) & 0xff] << 48) | 
                   (reverse_table[(word >> 16) & 0xff] << 40) |
                   (reverse_table[(word >> 24) & 0xff] << 32) |
                   (reverse_table[(word >> 32) & 0xff] << 24) | 
                   (reverse_table[(word >> 40) & 0xff] << 16) |
                   (reverse_table[(word >> 48) & 0xff] << 8) |
                   (reverse_table[(word >> 56) & 0xff]);

  return reverse;
}

void _mask_top_word(BIT_ARRAY* bitarr, word_addr_t num_of_words)
{
  // Mask top word
  word_offset_t bits_active = _bits_in_top_word(bitarr->num_of_bits);
  bitarr->words[num_of_words-1] &= BIT_MASK(bits_active);
}

// Set 64 bits from a particular start position
void _bit_array_set_word(BIT_ARRAY* bitarr, bit_index_t start, word_t word)
{
  // Bounds checking
  if(start >= bitarr->num_of_bits)
  {
    fprintf(stderr, "%s:%i:_bit_array_set_word(): Out of bounds error "
                    "(index: %lu, length: %lu)\n", __FILE__, __LINE__,
            (unsigned long)start, (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }

  #ifdef DEBUG
  printf("setting pos: %lu to word: ", (unsigned long)start);
  _bit_array_print_word(word, stdout);
  printf("\n");
  #endif

  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  word_addr_t word_index = bindex(start);
  word_offset_t word_offset = boffset(start);

  if(word_offset == 0)
  {
    bitarr->words[word_index] = word;
  }
  else
  {
    bitarr->words[word_index]
      = (word << word_offset) |
        (bitarr->words[word_index] & BIT_MASK(word_offset));
  
    if(word_index+1 < num_of_words)
    {
      bitarr->words[word_index+1]
        = (word >> (WORD_SIZE - word_offset)) |
          (bitarr->words[word_index+1] & (WORD_MAX << word_offset));
    }
  }

  // Mask top word
  _mask_top_word(bitarr, num_of_words);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

word_t _bit_array_get_word(const BIT_ARRAY* bitarr, bit_index_t start)
{
  return bit_array_word64(bitarr, start);
}

// Wrap around
word_t _bit_array_get_word_cyclic(const BIT_ARRAY* bitarr, bit_index_t start)
{
  word_t word = bit_array_word64(bitarr, start);

  word_offset_t bits_taken = bitarr->num_of_bits - start;

  if(bits_taken < WORD_SIZE)
  {
    word |= (bitarr->words[0] << bits_taken);

    if(bitarr->num_of_bits < (bit_index_t)WORD_SIZE)
    {
      // Mask word to prevent repetition of the same bits
      word = word & BIT_MASK(bitarr->num_of_bits);
    }
  }

  return word;
}

// Wrap around
void _bit_array_set_word_cyclic(BIT_ARRAY* bitarr, bit_index_t start, word_t word)
{
  _bit_array_set_word(bitarr, start, word);

  word_offset_t bits_set = bitarr->num_of_bits - start;

  if(bits_set < WORD_SIZE)
  {
    // Prevent overwriting the bits we've just set
    // by setting 'start' as the upper bound for the number of bits to write
    word_offset_t bits_remaining = MIN((bit_index_t)(WORD_SIZE - bits_set), start);
    word_t mask = BIT_MASK(bits_remaining);

    bitarr->words[0] = ((word >> bits_remaining) & mask) |
                       (bitarr->words[0] & ~mask);
  }
}

void _bit_array_fill_region(BIT_ARRAY* bitarr,
                            bit_index_t start, bit_index_t length,
                            word_t fill, bit_index_t spacing)
{
  // Bounds checking
  if(start + length > bitarr->num_of_bits)
  {
    fprintf(stderr, "%s:%i:_bit_array_fill_region(): Out of bounds error "
                    "(start: %lu, length: %lu, size: %lu)\n", __FILE__, __LINE__,
            (unsigned long)start, (unsigned long)length,
            (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }
  else if(length == 0)
  {
    return;
  }

  bit_index_t pos = start;

  for(; pos+spacing <= start+length; pos += spacing)
  {
    _bit_array_set_word(bitarr, pos, fill);
  }

  word_offset_t bits_remaining = start+length - pos;

  if(bits_remaining > 0)
  {
    word_t mask = BIT_MASK(bits_remaining);
    word_t dest_word = _bit_array_get_word(bitarr, pos);
    word_t fill_word = (fill & mask) | (dest_word & ~mask);

    _bit_array_set_word(bitarr, pos, fill_word);
  }

  _mask_top_word(bitarr, nwords(bitarr->num_of_bits));

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}


//
// Constructor
//
BIT_ARRAY* bit_array_create(bit_index_t nbits)
{
  BIT_ARRAY* bitarr = (BIT_ARRAY*) malloc(sizeof(BIT_ARRAY));
  
  if(bitarr == NULL)
  {
    // error - could not allocate enough memory
    errno = ENOMEM;
    return NULL;
  }

  bitarr->num_of_bits = nbits;
  bitarr->capacity_in_words = INIT_CAPACITY_WORDS;

  // Create initial array and initialise to zero
  bitarr->words = (word_t*) calloc(bitarr->capacity_in_words, sizeof(word_t));

  if(bitarr->words == NULL)
  {
    // error - could not allocate enough memory
    free(bitarr);
    errno = ENOMEM;
    return NULL;
  }

  #ifdef DEBUG
  printf("Creating BIT_ARRAY (bits: %lu; allocated words: %lu; "
         "using words: %lu; WORD_SIZE: %i)\n",
         (unsigned long)nbits, (unsigned long)bitarr->capacity_in_words,
         (unsigned long)nwords(nbits), (int)WORD_SIZE);

  _bit_array_check_top_word(bitarr);
  #endif

  return bitarr;
}

//
// Destructor
//
void bit_array_free(BIT_ARRAY* bitarr)
{
  if(bitarr->words != NULL)
    free(bitarr->words);

  free(bitarr);
}

bit_index_t bit_array_length(const BIT_ARRAY* bit_arr)
{
  return bit_arr->num_of_bits;
}

// Enlarge or shrink the size of a bit array
// Shrinking will free some memory if it is large
// Enlarging an array will add zeros to the end of it
// returns 1 on success, 0 on failure
char bit_array_resize(BIT_ARRAY* bitarr, bit_index_t new_num_of_bits)
{
  bit_index_t old_num_of_bits = bitarr->num_of_bits;

  word_addr_t old_num_of_words = nwords(old_num_of_bits);
  word_addr_t new_num_of_words = nwords(new_num_of_bits);

  bitarr->num_of_bits = new_num_of_bits;

  #ifdef DEBUG
  printf("Resize: old_num_of_words: %i; new_num_of_words: %i\n",
         (int)old_num_of_words, (int)new_num_of_words);
  #endif

  if(new_num_of_words > bitarr->capacity_in_words)
  {
    // Need to change the amount of memory used
    size_t old_capacity_in_bytes = bitarr->capacity_in_words * sizeof(word_t);
    bitarr->capacity_in_words *= 2;
    size_t new_capacity_in_bytes = bitarr->capacity_in_words * sizeof(word_t);
    bitarr->words = (word_t*)realloc(bitarr->words, new_capacity_in_bytes);

    if(bitarr->words == NULL)
    {
      // error - could not allocate enough memory
      errno = ENOMEM;
      return 0;
    }

    // Need to zero new memory
    size_t num_bytes_to_zero = new_capacity_in_bytes - old_capacity_in_bytes;
    memset(bitarr->words + old_num_of_words, 0x0, num_bytes_to_zero);
  }
  else if(new_num_of_words < old_num_of_words)
  {
    // Shrunk -- need to zero old memory
    size_t num_bytes_to_zero = (old_num_of_words - new_num_of_words)*sizeof(word_t);
    memset(bitarr->words + new_num_of_words, 0x0, num_bytes_to_zero);
  }

  // Mask top word
  _mask_top_word(bitarr, new_num_of_words);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif

  return 1;
}


//
// Methods
//

char bit_array_get_bit(const BIT_ARRAY* bitarr, bit_index_t b)
{
  if(b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "%s:%i:bit_array_get_bit(): Out of bounds error "
                    "(index: %lu; length: %lu)\n", __FILE__, __LINE__,
            (unsigned long)b, (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }

  return (bitarr->words[bindex(b)] >> (boffset(b))) & 0x1;
}

void bit_array_set_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
  if(b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "%s:%i:bit_array_set_bit(): Out of bounds error "
                    "(index: %lu; length: %lu)\n", __FILE__, __LINE__,
            (unsigned long)b, (unsigned long)bitarr->num_of_bits);

    errno = EDOM;
    exit(EXIT_FAILURE);
  }

  bitarr->words[bindex(b)] |= ((word_t)0x1 << (boffset(b)));

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

void bit_array_clear_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
  if(b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "%s:%i:bit_array_clear_bit(): Out of bounds "
                    "(index: %lu; length: %lu)\n", __FILE__, __LINE__,
            (unsigned long)b, (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }

  bitarr->words[bindex(b)] &= ~((word_t)1 << (boffset(b)));

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

// If char c != 0, set bit; otherwise clear bit
void bit_array_assign_bit(BIT_ARRAY* bitarr, bit_index_t b, char c)
{
  c ? bit_array_set_bit(bitarr, b) : bit_array_clear_bit(bitarr, b);
}

// Set multiple bits at once. 
// usage: bit_array_set_bits(bitarr, 3, {1,20,31});
void bit_array_set_bits(BIT_ARRAY* bitarr, size_t n, ...)
{
  va_list argptr;
  va_start(argptr, n);

  size_t i;
  for(i = 0; i < n; i++)
  {
    bit_index_t bit_index = va_arg(argptr, int);
    bit_array_set_bit(bitarr, bit_index);
  }

  va_end(argptr);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

// Clear multiple bits at once.
// usage: bit_array_clear_bits(bitarr, 3, {1,20,31});
void bit_array_clear_bits(BIT_ARRAY* bitarr, size_t n, ...)
{
  va_list argptr;
  va_start(argptr, n);

  size_t i;
  for(i = 0; i < n; i++)
  {
    bit_index_t bit_index = va_arg(argptr, bit_index_t);
    bit_array_clear_bit(bitarr, bit_index);
  }

  va_end(argptr);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

// set all elements of data to one
void bit_array_set_all(BIT_ARRAY* bitarr)
{
  word_addr_t num_of_words = nwords(bitarr->num_of_bits);
  bit_index_t num_of_bytes = num_of_words * sizeof(word_t);
  memset(bitarr->words, 0xFF, num_of_bytes);

  _mask_top_word(bitarr, num_of_words);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

// set all elements of data to zero
void bit_array_clear_all(BIT_ARRAY* bitarr)
{
  #ifdef DEBUG
  printf("Clearing %i bits, in %i words\n", (int)bitarr->num_of_bits,
         (int)nwords(bitarr->num_of_bits));
  #endif

  bit_index_t num_of_bytes = nwords(bitarr->num_of_bits) * sizeof(word_t);
  memset(bitarr->words, 0, num_of_bytes);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

// Find the index of the first bit that is set.  
// Returns 1 if a bit is set, otherwise 0
// Index of first set bit is stored in the integer pointed to by result
// If not bit is set result is not changed
char bit_array_find_first_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result)
{
  // Find first word that is greater than zero
  word_addr_t num_of_words = nwords(bitarr->num_of_bits);
  word_addr_t i;
  
  for(i = 0; i < num_of_words; i++)
  {
    if(bitarr->words[i] > 0)
    {
      *result = i * WORD_SIZE + TRAILING_ZEROS(bitarr->words[i]);
      return 1;
    }
  }

  return 0;
}

// Find the index of the last bit that is set.  
// Returns 1 if a bit is set, otherwise 0
// Index of last set bit is stored in the integer pointed to by `result`
// If no bit is set result is not changed
char bit_array_find_last_set_bit(const BIT_ARRAY* bitarr, bit_index_t* result)
{
  // Find last word that is greater than zero
  word_addr_t num_of_words = nwords(bitarr->num_of_bits);
  word_addr_t i = num_of_words - 1;

  // check top word
  if(bitarr->words[i] > 0)
  {
    word_addr_t unused_bits = WORD_SIZE - _bits_in_top_word(bitarr->num_of_bits);
    word_t w = bitarr->words[i] << unused_bits;
    *result = bitarr->num_of_bits - LEADING_ZEROS(w) - 1;
    return 1;
  }

  i--;

  // i is unsigned so have to use break when i == 0
  while(1)
  {
    if(bitarr->words[i] > 0)
    {
      *result = (i+1) * WORD_SIZE - LEADING_ZEROS(bitarr->words[i]) - 1;
      return 1;
    }
    else if(i == 0)
    {
      return 0;
    }

    i--;
  }

  return 0;
}

// Parity - returns 1 if odd number of bits set, 0 if even
char bit_array_parity(BIT_ARRAY* bitarr)
{
  word_addr_t w;
  word_addr_t num_words = nwords(bitarr->num_of_bits);
  unsigned int parity = 0;

  for(w = 0; w < num_words; w++)
  {
    parity ^= __builtin_parityl(bitarr->words[w]);
  }

  return (char)parity;
}

// Get the number of bits set (hamming weight)
bit_index_t bit_array_num_bits_set(const BIT_ARRAY* bitarr)
{
  word_addr_t num_of_words = nwords(bitarr->num_of_bits);
  word_addr_t i;
  
  bit_index_t num_of_bits_set = 0;

  for(i = 0; i < num_of_words; i++)
  {
    if(bitarr->words[i] > 0)
    {
      num_of_bits_set += __builtin_popcountl(bitarr->words[i]);
      
      /*
      // Use if not using GCC or __builtin_popcount not available
      bit_index_t j;
      for(j = 0; j < WORD_SIZE; j++)
      {
        if((bitarr->words[i] >> (word_t)j) & (word_t)0x1)
        {
          num_of_bits_set++;
        }
      }
      */
    }
  }

  return num_of_bits_set;
}

// Get the number of bits not set (1 - hamming weight)
bit_index_t bit_array_num_bits_cleared(const BIT_ARRAY* bitarr)
{
  return bitarr->num_of_bits - bit_array_num_bits_set(bitarr);
}

// Put all the 0s before all the 1s
void bit_array_sort_bits(BIT_ARRAY* bitarr)
{
  bit_index_t num_of_bits_set = bit_array_num_bits_set(bitarr);
  bit_index_t num_of_bits_cleared = bitarr->num_of_bits - num_of_bits_set;

  #ifdef DEBUG
  printf("sort_bits (bits set: %lu, bits unset: %lu)\n",
         (unsigned long)num_of_bits_set, (unsigned long)num_of_bits_cleared);
  #endif

  bit_array_set_all(bitarr);
  bit_array_clear_region(bitarr, 0, num_of_bits_cleared);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

// Put all the 1s before all the 0s
void bit_array_rev_sort_bits(BIT_ARRAY* bitarr)
{
  bit_index_t num_of_bits_set = bit_array_num_bits_set(bitarr);

  #ifdef DEBUG
  printf("rev_sort_bits (bits set: %lu)\n", (unsigned long)num_of_bits_set);
  #endif

  bit_array_clear_all(bitarr);
  bit_array_set_region(bitarr, 0, num_of_bits_set);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

// Clear all the bits in a region
void bit_array_clear_region(BIT_ARRAY* bitarr,
                            bit_index_t start, bit_index_t length)
{
  _bit_array_fill_region(bitarr, start, length, (word_t)0x0, WORD_SIZE);
}

// Set all the bits in a region
void bit_array_set_region(BIT_ARRAY* bitarr,
                          bit_index_t start, bit_index_t length)
{
  _bit_array_fill_region(bitarr, start, length, WORD_MAX, WORD_SIZE);
}


//
// Strings and printing
//

// Construct a BIT_ARRAY from a substring with given on and off characters.
BIT_ARRAY* bit_array_from_substr(const char* str, size_t len,
                                 char on, char off, char reverse)
{
  size_t i;
  for(i = 0; i < len; i++)
  {
    if(str[i] != on && str[i] != off)
    {
      fprintf(stderr, "%s:%i:bit_array_from_substr(): Invalid char '%c' "
                      "(on: %c; off: %c)\n",
              __FILE__, __LINE__, str[i], on, off);
      errno = EDOM;
      return NULL;
    }
  }

  BIT_ARRAY* bitarr = bit_array_create(len);

  // Return null if not enough memory to allocate bit array
  if(bitarr == NULL)
  {
    errno = ENOMEM;
    return NULL;
  }

  // BitArray is all 0s by default -- just set the 1s
  bit_index_t j;

  for(i = 0; i < len; i++)
  {
    j = (reverse ? len - 1 - i : i);

    if(str[j] == on)
    {
      bit_array_set_bit(bitarr, i);
    }
  }

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif

  return bitarr;
}

// From string method (remember to free the result!)
// Returns NULL if not enough memory to allocate the array
BIT_ARRAY* bit_array_from_str(const char* str)
{
  return bit_array_from_substr(str, strlen(str), '1', '0', 0);
}

// Takes a char array to write to.  `str` must be bitarr->num_of_bits+1 in length
// Terminates string with '\0'
char* bit_array_to_str(const BIT_ARRAY* bitarr, char* str)
{
  bit_index_t i;
  
  for(i = 0; i < bitarr->num_of_bits; i++)
  {
    str[i] = bit_array_get_bit(bitarr, i) ? '1' : '0';
  }

  str[bitarr->num_of_bits] = '\0';

  return str;
}

// Get a string representations for a given region, using given on/off characters.
// Does not null-terminate string.
void bit_array_to_substr(const BIT_ARRAY* bitarr, char* str,
                         bit_index_t start, bit_index_t length,
                         char on, char off, char reverse)
{
  if(start + length > bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "%s:%i: bit_array_substr() - "
                    "out of bounds error "
                    "(start: %lu; length: %lu; num_of_bits: %lu)\n",
            __FILE__, __LINE__,
            (unsigned long)start, (unsigned long)length,
            (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }

  bit_index_t i, j;
  bit_index_t end = start + length - 1;

  for(i = 0; i < length; i++)
  {
    j = (reverse ? end - i : start+i);
    str[i] = bit_array_get_bit(bitarr, j) ? on : off;
  }
}

// Print this array to a file stream.  Prints '0's and '1'.  Doesn't print newline.
void bit_array_print(const BIT_ARRAY* bitarr, FILE* fout)
{
  bit_index_t i;
  
  for(i = 0; i < bitarr->num_of_bits; i++)
  {
    fprintf(fout, "%c", bit_array_get_bit(bitarr, i) ? '1' : '0');
  }
}

// Print a string representations for a given region, using given on/off characters.
void bit_array_print_substr(const BIT_ARRAY* bitarr, FILE* fout,
                            bit_index_t start, bit_index_t length,
                            char on, char off, char reverse)
{
  if(start + length > bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "%s:%i:bit_array_print_substr(): "
                    "out of bounds error "
                    "(start: %lu; length: %lu; num_of_bits: %lu)\n",
            __FILE__, __LINE__,
            (unsigned long)start, (unsigned long)length,
            (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }

  bit_index_t i, j;
  bit_index_t end = start + length - 1;
  
  for(i = 0; i < length; i++)
  {
    j = (reverse ? end - i : start+i);
    fprintf(fout, "%c", bit_array_get_bit(bitarr, j) ? on : off);
  }
}


//
// Clone and copy
//

BIT_ARRAY* bit_array_clone(const BIT_ARRAY* bitarr)
{
  BIT_ARRAY* cpy = bit_array_create(bitarr->num_of_bits);

  // Copy across bits
  memcpy(cpy->words, bitarr->words, nwords(bitarr->num_of_bits) * sizeof(word_t));

  #ifdef DEBUG
  _bit_array_check_top_word(cpy);
  #endif

  return cpy;
}

// destination and source may be the same bit_array
// and src/dst regions may overlap
void bit_array_copy(BIT_ARRAY* dst, bit_index_t dstindx,
                    const BIT_ARRAY* src, bit_index_t srcindx,
                    bit_index_t length)
{
  // Bounds checking
  if(srcindx + length > src->num_of_bits ||
     dstindx + length > dst->num_of_bits)
  {
    fprintf(stderr, "%s:%i:bit_array_copy(): Out of bounds error "
                    "(dstindx: %lu, srcindx: %lu, length: %lu)\n",
            __FILE__, __LINE__,
            (unsigned long)dstindx, (unsigned long)srcindx,
            (unsigned long)length);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }
  
  #ifdef DEBUG
  printf("bit_array_copy(dst: %lu, src: %lu, length: %lu)\n",
         (unsigned long)dstindx, (unsigned long)srcindx, (unsigned long)length);
  #endif

  // Num of full words to copy
  word_addr_t num_of_full_words = length / WORD_SIZE;
  word_addr_t i;

  word_offset_t bits_in_last_word = _bits_in_top_word(length);

  if(dst == src && srcindx > dstindx)
  {
    // Work left to right
    #ifdef DEBUG
    printf("copy left to right");
    #endif

    for(i = 0; i < num_of_full_words; i++)
    {
      word_t word = _bit_array_get_word(src, srcindx+i*WORD_SIZE);
      _bit_array_set_word(dst, dstindx+i*WORD_SIZE, word);
    }

    if(bits_in_last_word > 0)
    {
      word_t src_word = _bit_array_get_word(src, srcindx+i*WORD_SIZE);
      word_t dst_word = _bit_array_get_word(dst, dstindx+i*WORD_SIZE);

      word_t mask = BIT_MASK(bits_in_last_word);
      word_t word = (dst_word & ~mask) | (src_word & mask);

      _bit_array_set_word(dst, dstindx+num_of_full_words*WORD_SIZE, word);
    }
  }
  else
  {
    // Work right to left
    #ifdef DEBUG
    printf("copy right to left");
    #endif

    for(i = num_of_full_words; i > 0; i--)
    {
      word_t word = _bit_array_get_word(src, srcindx+length-i*WORD_SIZE);
      _bit_array_set_word(dst, dstindx+length-i*WORD_SIZE, word);
    }

    #ifdef DEBUG
    printf("Copy %i,%i to %i\n", (int)srcindx, (int)bits_in_last_word,
                                 (int)dstindx);
    #endif

    if(bits_in_last_word > 0)
    {
      word_t src_word = _bit_array_get_word(src, srcindx);
      word_t dst_word = _bit_array_get_word(dst, dstindx);

      word_t mask = BIT_MASK(bits_in_last_word);
      word_t word = (dst_word & ~mask) | (src_word & mask);
      _bit_array_set_word(dst, dstindx, word);
    }
  }

  _mask_top_word(dst, nwords(dst->num_of_bits));

  #ifdef DEBUG
  _bit_array_check_top_word(dst);
  #endif
}


//
// Logic operators
//

// Destination can be the same as one or both of the sources
void bit_array_and(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
  // Ensure dst array is big enough
  word_addr_t max_bits = MAX(src1->num_of_bits, src2->num_of_bits);

  if(dst->num_of_bits < max_bits)
  {
    bit_array_resize(dst, max_bits);
  }

  word_addr_t num_of_words1 = nwords(src1->num_of_bits);
  word_addr_t num_of_words2 = nwords(src2->num_of_bits);

  word_addr_t min_words = MIN(num_of_words1, num_of_words2);

  word_addr_t i;

  for(i = 0; i < min_words; i++)
  {
    dst->words[i] = src1->words[i] & src2->words[i];
  }

  // Set remaining bits to zero
  word_addr_t dst_words = nwords(dst->num_of_bits);

  for(i = min_words; i < dst_words; i++)
  {
    dst->words[i] = (word_t)0;
  }

  #ifdef DEBUG
  _bit_array_check_top_word(dst);
  #endif
}

// Destination can be the same as one or both of the sources
void bit_array_or_xor(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2, char xor)
{
  // Ensure dst array is big enough
  word_addr_t max_bits = MAX(src1->num_of_bits, src2->num_of_bits);

  if(dst->num_of_bits < max_bits)
  {
    bit_array_resize(dst, max_bits);
  }

  word_addr_t num_of_words1 = nwords(src1->num_of_bits);
  word_addr_t num_of_words2 = nwords(src2->num_of_bits);

  word_addr_t min_words = MIN(num_of_words1, num_of_words2);
  word_addr_t max_words = MAX(num_of_words1, num_of_words2);

  word_addr_t i;

  if(xor)
  {
    for(i = 0; i < min_words; i++)
      dst->words[i] = src1->words[i] ^ src2->words[i];
  }
  else
  {
    for(i = 0; i < min_words; i++)
      dst->words[i] = src1->words[i] | src2->words[i];
  }

  // Copy remaining bits from longer src array
  if(min_words != max_words)
  {
    const BIT_ARRAY* longer = src1->num_of_bits > src2->num_of_bits ? src1 : src2;

    for(i = min_words; i < max_words; i++)
    {
      dst->words[i] = longer->words[i];
    }
  }

  // Set remaining bits to zero
  word_addr_t dst_words = nwords(dst->num_of_bits);

  for(i = max_words; i < dst_words; i++)
  {
    dst->words[i] = (word_t)0;
  }

  #ifdef DEBUG
  _bit_array_check_top_word(dst);
  #endif
}

void bit_array_or(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
  bit_array_or_xor(dst, src1, src2, 0);
}

// Destination can be the same as one or both of the sources
void bit_array_xor(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
  bit_array_or_xor(dst, src1, src2, 1);
}

void bit_array_not(BIT_ARRAY* dst, const BIT_ARRAY* src)
{
  if(dst->num_of_bits < src->num_of_bits)
  {
    bit_array_resize(dst, src->num_of_bits);
  }

  word_addr_t src_words = nwords(src->num_of_bits);
  word_addr_t dst_words = nwords(dst->num_of_bits);

  word_addr_t i;

  for(i = 0; i < src_words; i++)
  {
    dst->words[i] = ~(src->words[i]);
  }

  for(i = src_words; i < dst_words; i++)
  {
    dst->words[i] = WORD_MAX;
  }

  _mask_top_word(dst, dst_words);

  #ifdef DEBUG
  _bit_array_check_top_word(dst);
  #endif
}

// Flip bits is a region
void bit_array_complement_region(BIT_ARRAY* dst, bit_index_t start, bit_index_t len)
{
  if(start + len > dst->num_of_bits)
  {
    fprintf(stderr, "%s:%i: Error -- bit_array_complement_region(%lu,%lu) out "
                    "of bounds [length: %lu]\n", __FILE__, __LINE__,
            (unsigned long)start, (unsigned long)len,
            (unsigned long)dst->num_of_bits);

    exit(EXIT_FAILURE);
  }
  else if(len == 0)
  {
    return;
  }

  word_t w, mask;

  word_addr_t first_word = start/WORD_SIZE;
  bit_index_t first_word_index = first_word * WORD_SIZE;
  word_offset_t bits_in_first_word = first_word_index + WORD_SIZE - start;

  if(bits_in_first_word >= len)
  {
    // All bits are in the first word
    w = dst->words[first_word];
    mask = BIT_MASK(len) << (WORD_SIZE - bits_in_first_word);
    dst->words[first_word] = (w & ~mask) | (~w & mask);

    // Mask top word
    _mask_top_word(dst, nwords(dst->num_of_bits));

    return;
  }

  if(bits_in_first_word < WORD_SIZE)
  {
    // Deal with first partial word
    w = dst->words[first_word];
    mask = BIT_MASK(bits_in_first_word) << (WORD_SIZE - bits_in_first_word);
    dst->words[first_word] = (w & ~mask) | (~w & mask);
    first_word++;
  }

  word_addr_t last_word = (start+len)/WORD_SIZE;
  bit_index_t last_word_index = last_word * WORD_SIZE;
  word_offset_t bits_in_last_word = len - last_word_index;

  //printf("last word: %i, last word index: %i, bits in last word: %i\n",
  //       (int)last_word, (int)last_word_index, (int)bits_in_last_word);

  word_addr_t i;
  for(i = first_word; i < last_word; i++)
  {
    // Deal with whole words
    dst->words[i] = ~dst->words[i];
  }

  if(bits_in_last_word > 0)
  {
    // Deal with last partial word
    w = dst->words[last_word];
    mask = BIT_MASK(bits_in_last_word);
    dst->words[last_word] = (w & ~mask) | (~w & mask);
  }

  // Mask top word
  _mask_top_word(dst, nwords(dst->num_of_bits));

  #ifdef DEBUG
  _bit_array_check_top_word(dst);
  #endif
}

// Compare two bit arrays by value stored
// arrays do not have to be the same length (e.g. 101 (5) > 00000011 (3))
// bits are compared from highest index bit (msb) to lowest (lsb)
// returns:
//   1 iff bitarr1 > bitarr2
//   0 iff bitarr1 == bitarr2
//  -1 iff bitarr1 < bitarr2
int bit_array_cmp(const BIT_ARRAY* bitarr1, const BIT_ARRAY* bitarr2)
{
  word_addr_t nwords1 = nwords(bitarr1->num_of_bits);
  word_addr_t nwords2 = nwords(bitarr2->num_of_bits);

  word_addr_t max_words = MAX(nwords1, nwords2);

  word_addr_t i;
  word_t word1, word2;

  // i is unsigned to break when i == 0
  for(i = max_words-1; ; i--)
  {
    word1 = (i < nwords1 ? bitarr1->words[i] : 0);
    word2 = (i < nwords2 ? bitarr2->words[i] : 0);

    if(word1 > word2)
    {
      return 1;
    }
    else if(word1 < word2)
    {
      return -1;
    }
    else if(i == 0)
    {
      return 0;
    }
  }

  return 0;
}

void bit_array_set_word64(BIT_ARRAY* bitarr, bit_index_t start, uint64_t word)
{
  _bit_array_set_word(bitarr, start, (word_t)word);
}

uint64_t bit_array_word64(const BIT_ARRAY* bitarr, bit_index_t start)
{
  // Bounds checking
  if(start >= bitarr->num_of_bits)
  {
    fprintf(stderr, "%s:%i:bit_array_word64() - out of bounds error "
                    "(index: %lu, length: %lu)\n", __FILE__, __LINE__,
           (unsigned long)start, (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }

  word_addr_t word_index = bindex(start);
  word_offset_t word_offset = boffset(start);

  uint64_t result = bitarr->words[word_index] >> word_offset;

  word_offset_t bits_taken = WORD_SIZE - word_offset;

  // word_offset is now the number of bits we need from the next word
  // Check the next word has at least some bits
  if(word_offset > 0 && start + bits_taken < bitarr->num_of_bits)
  {
    result |= bitarr->words[word_index+1] << (WORD_SIZE - word_offset);
  }

  return result;
}

uint32_t bit_array_word32(const BIT_ARRAY* bitarr, bit_index_t start)
{
  return (uint32_t)bit_array_word64(bitarr, start);
}

uint16_t bit_array_word16(const BIT_ARRAY* bitarr, bit_index_t start)
{
  return (uint16_t)bit_array_word64(bitarr, start);
}

uint8_t bit_array_word8(const BIT_ARRAY* bitarr, bit_index_t start)
{
  return (uint8_t)bit_array_word64(bitarr, start);
}

void bit_array_shift_right(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)
{
  bit_index_t cpy_length = bitarr->num_of_bits - shift_dist;
  bit_array_copy(bitarr, shift_dist, bitarr, 0, cpy_length);
  
  if(fill)
  {
    bit_array_set_region(bitarr, 0, shift_dist);
  }
  else
  {
    bit_array_clear_region(bitarr, 0, shift_dist);
  }
}

void bit_array_shift_left(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)
{
  bit_index_t cpy_length = bitarr->num_of_bits - shift_dist;
  bit_array_copy(bitarr, 0, bitarr, shift_dist, cpy_length);
  
  if(fill)
  {
    bit_array_set_region(bitarr, cpy_length, shift_dist);
  }
  else
  {
    bit_array_clear_region(bitarr, cpy_length, shift_dist);
  }
}


//
// Adding / Subtracting
//

// src1, src2 and dst can all be the same BIT_ARRAY
void _bit_array_arithmetic(BIT_ARRAY* dst,
                           const BIT_ARRAY* src1, const BIT_ARRAY* src2,
                           char subtract)
{
  word_addr_t nwords1 = nwords(src1->num_of_bits);
  word_addr_t nwords2 = nwords(src2->num_of_bits);

  word_addr_t max_words = MAX(nwords1, nwords2);

  // Adding: dst_words >= max(src1 words, src2 words)
  // Subtracting: dst_words is >= nwords1
  word_addr_t dst_words = nwords(dst->num_of_bits);

  char carry = subtract ? 1 : 0;

  word_addr_t i;
  word_t word1, word2;
  
  for(i = 0; i < max_words; i++)
  {
    word1 = (i < nwords1 ? src1->words[i] : 0);
    word2 = (i < nwords2 ? src2->words[i] : 0);

    if(subtract)
      word2 = ~word2;

    dst->words[i] = word1 + word2 + carry;
    // Update carry
    carry = WORD_MAX - word1 < word2 || WORD_MAX - word1 - word2 < (word_t)carry;
  }

  if(subtract)
  {
    carry = 0;
  }
  else
  {
    // Check last word
    word_offset_t bits_on_last_word = _bits_in_top_word(dst->num_of_bits);

    if(bits_on_last_word < WORD_SIZE)
    {
      word_t mask = BIT_MASK(bits_on_last_word);

      if(dst->words[max_words-1] > mask)
      {
        // Array has overflowed, increase size
        dst->num_of_bits++;
      }
    }
    else if(carry)
    {
      // Carry onto a new word
      if(dst_words == max_words)
      {
        // Need to resize for the carry bit
        if(!bit_array_resize(dst, dst->num_of_bits+1))
        {
          fprintf(stderr, "%s:%i:_bit_array_arithmetic(): ran out of memory\n",
                  __FILE__, __LINE__);
          exit(EXIT_FAILURE);
        }
      }

      dst->words[max_words] = (word_t)1;
    }
  }

  // Zero the rest of dst array
  for(i = max_words+carry; i < dst_words; i++)
  {
    dst->words[i] = (word_t)0;
  }

  #ifdef DEBUG
  _bit_array_check_top_word(dst);
  #endif
}

// src1, src2 and dst can all be the same BIT_ARRAY
// If dst is shorter than either of src1, src2, it is enlarged
void bit_array_add(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
  bit_index_t max_src_bits = MAX(src1->num_of_bits, src2->num_of_bits);

  if(dst->num_of_bits < max_src_bits)
  {
    if(!bit_array_resize(dst, max_src_bits))
    {
      fprintf(stderr, "%s:%i:bit_array_add(): ran out of memory\n",
              __FILE__, __LINE__);
      exit(EXIT_FAILURE);
    }
  }

  _bit_array_arithmetic(dst, src1, src2, 0);
}

// dst = src1 - src2
// src1, src2 and dst can all be the same BIT_ARRAY
// If dst is shorter than src1, it will be extended to be as long as src1
// src1 must be greater than or equal to src2 (src1 >= src2)
void bit_array_subtract(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
  // subtraction by method of complements:
  // a - b = a + ~b + 1 = src1 + ~src2 +1

  // src1 must be >= src2
  if(bit_array_cmp(src1, src2) < 0)
  {
    // Error
    fprintf(stderr, "%s:%i:bit_array_subtract(): bit_array_substract "
                    "requires src1 >= src2\n", __FILE__, __LINE__);
    exit(EXIT_FAILURE);
  }

  if(dst->num_of_bits < src1->num_of_bits)
  {
    if(!bit_array_resize(dst, src1->num_of_bits))
    {
      fprintf(stderr, "%s:%i:bit_array_subtract(): ran out of memory\n",
              __FILE__, __LINE__);
      exit(EXIT_FAILURE);
    }
  }

  _bit_array_arithmetic(dst, src1, src2, 1);
}

// Add one to a bit array
// If dst is too small it will be resized to hold the highest set bit
void bit_array_increment(BIT_ARRAY* bitarr)
{
  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  char carry = 1;

  word_addr_t i;
  for(i = 0; i < num_of_words-1; i++)
  {
    if(bitarr->words[i] == WORD_MAX)
    {
      // Carry continues
      bitarr->words[i] = 0;
    }
    else
    {
      // Carry is absorbed
      bitarr->words[i]++;
      carry = 0;
      break;
    }
  }

  // Deal with last word
  if(carry)
  {
    word_offset_t bits_in_last_word = _bits_in_top_word(bitarr->num_of_bits);
    word_t mask = BIT_MASK(bits_in_last_word);
    word_t final_word = bitarr->words[num_of_words-1];

    if(final_word < mask)
    {
      bitarr->words[num_of_words-1] = final_word + 1;
    }
    else if(bits_in_last_word < WORD_SIZE)
    {
      bitarr->num_of_bits++;
      bitarr->words[num_of_words-1]++;
    }
    else
    {
      // Bit array full, need another word
      bit_array_resize(bitarr, bitarr->num_of_bits + 1);
      bitarr->words[num_of_words-1] = 0;
      bitarr->words[num_of_words] = 1;
    }
  }

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

// If there is an underflow, bit array will be set to all 0s and 0 is returned
// Returns 1 on success, 0 if there was an underflow
char bit_array_decrement(BIT_ARRAY* bitarr)
{
  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  word_addr_t i;
  for(i = 0; i < num_of_words; i++)
  {
    if(bitarr->words[i] > 0)
    {
      bitarr->words[i]--;

      for(; i > 0; i--)
      {
        bitarr->words[i-1] = ~0;
      }
      
      return 1;
    }
  }

  // if prev_last_word == 0
  // underflow -> number left unchanged

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif

  return 0;
}

//
// Read/Write from files
//
// file format is [8 bytes: for number of elements in array][data]
//

// Saves bit array to a file. Returns the number of bytes written
bit_index_t bit_array_save(const BIT_ARRAY* bitarr, FILE* f)
{
  bit_index_t num_of_bytes = nbytes(bitarr->num_of_bits);
  bit_index_t bytes_written = 0;

  // Write 8 bytes to store the number of bits in the array
  bytes_written += fwrite(&bitarr->num_of_bits, 8, 1, f);
  // Write the array
  bytes_written += fwrite(bitarr->words, 1, num_of_bytes, f);

  return bytes_written;
}

// Reads bit array from a file. Returns bit array or NULL on failure
BIT_ARRAY* bit_array_load(FILE* f)
{
  bit_index_t items_read;

  // Read in number of bits
  bit_index_t num_bits = 0;
  items_read = fread(&num_bits, sizeof(bit_index_t), 1, f);

  if(items_read != 1)
  {
    return NULL;
  }

  // Create bit_array
  BIT_ARRAY* bitarr = bit_array_create(num_bits);

  if(bitarr == NULL)
  {
    return NULL;
  }

  // Have to calculate how many bytes are needed for the file
  // (Note: this may be different from num_of_words * sizeof(word_t))
  bit_index_t num_of_bytes_in_file = nbytes(bitarr->num_of_bits);

  items_read = fread(bitarr->words, 1, num_of_bytes_in_file, f);

  if(items_read != num_of_bytes_in_file)
  {
    free(bitarr->words);
    free(bitarr);
    return NULL;
  }

  // Mask top word
  _mask_top_word(bitarr, nwords(bitarr->num_of_bits));

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif

  return bitarr;
}



//
// Experimental - development - full of bugs
//

void bit_array_reverse_region(BIT_ARRAY* bitarr,
                              bit_index_t start_indx,
                              bit_index_t end_indx)
{
  bit_index_t left = start_indx;
  bit_index_t right;
  
  if(end_indx < (bit_index_t)WORD_SIZE)
  {
    right = end_indx + bitarr->num_of_bits - WORD_SIZE + 1;
  }
  else
  {
    right = end_indx - WORD_SIZE + 1;
  }

  word_t left_word, right_word;

  bit_index_t dist = (left <= right) ? right - left
                                     : right + (bitarr->num_of_bits - left);

  while(dist > (bit_index_t)WORD_SIZE)
  {
    left_word = _bit_array_get_word_cyclic(bitarr, left);
    right_word = _bit_array_get_word_cyclic(bitarr, right);

    _bit_array_set_word_cyclic(bitarr, left, _bit_array_reverse_word(right_word));
    _bit_array_set_word_cyclic(bitarr, right, _bit_array_reverse_word(left_word));

    if(dist <= (bit_index_t)WORD_SIZE)
    {
      break;
    }
    else
    {
      dist -= WORD_SIZE;
    }

    // move the left and right towards each other
    left += WORD_SIZE;

    if(left >= bitarr->num_of_bits)
    {
      left -= bitarr->num_of_bits;
    }

    if(right < (bit_index_t)WORD_SIZE)
    {
      right += bitarr->num_of_bits;
    }

    right -= WORD_SIZE;
  }

  if(left == right)
  {
    // Flip the word
    left_word = _bit_array_get_word_cyclic(bitarr, left);
    _bit_array_set_word_cyclic(bitarr, left, _bit_array_reverse_word(left_word));
  }
  else
  {
    // bits remaining is 'dist'
    word_t word = _bit_array_get_word_cyclic(bitarr, left);
    word_t word_rev = _bit_array_reverse_word(word);
    word_t mask = BIT_MASK(dist);

    word = (word & mask) | (word_rev & ~mask);
    _bit_array_set_word_cyclic(bitarr, left, word);
  }
}

void bit_array_reverse(BIT_ARRAY* bitarr)
{
  bit_array_reverse_region(bitarr, 0, bitarr->num_of_bits-1);
}

/*
 data =0123456789
 word =2
 shift=3
 
 + => starting pos

 0123456789
 7890123456
 ..|..|..|.
 0123456789
 +  01
       34 6
 7
   +  2  5
  8
 7890123456
*/
void bit_array_cycle_right(BIT_ARRAY* bitarr, bit_index_t cycle_dist)
{
  bit_index_t cycle = cycle_dist % bitarr->num_of_bits;

  if(cycle == 0)
  {
    return;
  }
  else if(bitarr->num_of_bits <= (bit_index_t)(2 * WORD_SIZE))
  {
    // cycle < WORD_SIZE

    bit_index_t cpy_from, cpy_to, cpy_length;
    word_t tmp, mask;

    if(cycle <= (bit_index_t)WORD_SIZE)
    {
      // cycle is small
      cpy_from = 0;
      cpy_to = cycle;
      cpy_length = bitarr->num_of_bits - cycle;

      tmp = _bit_array_get_word(bitarr, cpy_to);
      bit_array_copy(bitarr, cpy_to, bitarr, cpy_from, cpy_length);

      mask = BIT_MASK(cpy_to);
      bitarr->words[0] = (tmp & mask) | (bitarr->words[0] & ~mask);
    }
    else
    {
      // cycle is big
      cpy_from = bitarr->num_of_bits - cycle;
      cpy_to = 0;
      cpy_length = cycle;

      word_offset_t top_word_offset = cycle - WORD_SIZE;
      tmp = bitarr->words[0] << top_word_offset;
      bit_array_copy(bitarr, cpy_to, bitarr, cpy_from, cpy_length);

      mask = BIT_MASK(cpy_from) << top_word_offset;

      bitarr->words[1] = (tmp & mask) | (bitarr->words[1] & ~mask);
      _mask_top_word(bitarr, 2);
    }
  }
  else
  {
    word_addr_t num_of_full_words_in_cycle = cycle / WORD_SIZE;
    word_addr_t i;

    word_t mask = BIT_MASK(cycle - num_of_full_words_in_cycle*WORD_SIZE);

    for(i = 0; i <= num_of_full_words_in_cycle; i++)
    {
      bit_index_t cycle_offset = i * num_of_full_words_in_cycle * WORD_SIZE;
      bit_index_t move_from = cycle_offset;
      bit_index_t move_to = move_from + cycle;

      word_t last_word = _bit_array_get_word_cyclic(bitarr, move_from);

      while(1)
      {
        word_t next_word = _bit_array_get_word_cyclic(bitarr, move_to);

        if(i == num_of_full_words_in_cycle)
        {
          last_word = (last_word & mask) | (next_word & ~mask);
        }

        _bit_array_set_word_cyclic(bitarr, move_to, last_word);

        #ifdef DEBUG
        printf("\n");
        printf("move_from: %i; word: ", (int)move_from);
        _bit_array_print_word(last_word, stdout);
        printf("\n");
        
        printf("move_to: %i; word: ", (int)move_to);
        _bit_array_print_word(next_word, stdout);
        printf("\n");

        bit_array_print(bitarr, stdout);
        printf("\n");
        #endif

        if(move_to < move_from)
        {
          break;
        }

        last_word = next_word;
        move_from = move_to;
        move_to += cycle;
        
        if(move_to >= bitarr->num_of_bits)
        {
          // Cycle round
          move_to -= bitarr->num_of_bits;
        }
      }
    }

    
  }

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

void bit_array_cycle_left(BIT_ARRAY* bitarr, bit_index_t cycle_dist)
{
  bit_array_cycle_right(bitarr, bitarr->num_of_bits - cycle_dist);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}
