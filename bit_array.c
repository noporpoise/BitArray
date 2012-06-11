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
#include <stdio.h>
#include <errno.h>
#include <limits.h> // ULONG_MAX
#include <string.h> // memset

#include "bit_array.h"

#define MIN(a, b)  (((a) <= (b)) ? (a) : (b))
#define MAX(a, b)  (((a) >= (b)) ? (a) : (b))

//
// For internal use
//

struct BIT_ARRAY {
  word_t* words;
  bit_index_t num_of_bits;
};

// sizeof gives size in bytes (8 bits per byte)
int WORD_SIZE = sizeof(word_t) * 8;

// Index of word
inline word_addr_t bindex(bit_index_t b) { return b / WORD_SIZE; }

// Offset within a word (values up to 64 most likely)
inline unsigned int boffset(bit_index_t b) { return b % WORD_SIZE; }

// Number of words required to store a given number of bits
inline word_addr_t nwords(bit_index_t b)
{
  return (b + WORD_SIZE - 1) / WORD_SIZE;
}

// Number of bytes required to store a given number of bits
inline word_addr_t nbytes(bit_index_t b)
{
  return (b + 7) / 8;
}

// word of all 1s
#define WORD_MAX  (~(word_t)0)
#define BIT_MASK(length)  (WORD_MAX >> (WORD_SIZE-(length)))

inline void mask_top_word(BIT_ARRAY* bitarr, word_addr_t num_of_words)
{
  // Mask top word
  bitarr->words[num_of_words-1] &= BIT_MASK(boffset(bitarr->num_of_bits-1)+1);
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

  word_addr_t num_of_words = nwords(nbits);

  #ifdef DEBUG
  printf("Creating BIT_ARRAY (bits: %lu; words: %lu; WORD_SIZE: %i)\n",
         (unsigned long)nbits, (unsigned long)num_of_words, WORD_SIZE);
  #endif

  //bitarr->num_of_words = num_of_words;
  bitarr->num_of_bits = nbits;
  bitarr->words = (word_t*) malloc(sizeof(word_t) * num_of_words);

  if(bitarr->words == NULL)
  {
    // error - could not allocate enough memory
    free(bitarr);
    errno = ENOMEM;
    return NULL;
  }

  // Initialise to zero
  bit_array_fill_zeros(bitarr);

  return bitarr;
}

//
// Destructor
//
void bit_array_free(BIT_ARRAY* bitarr)
{
  free(bitarr->words);
  free(bitarr);
}


//
// Methods
//
void bit_array_set_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
  if(b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "bit_array.c: bit_array_set_bit() - "
            "out of bounds error (index: %lu; length: %lu)\n",
            (unsigned long)b, (unsigned long)bitarr->num_of_bits);

    errno = EDOM;

    exit(EXIT_FAILURE);
  }

  bitarr->words[bindex(b)] |= ((word_t)0x1 << (boffset(b)));
}

void bit_array_clear_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
  if(b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "bit_array.c: bit_array_clear_bit() - "
                    "out of bounds error (index: %lu; length: %lu)\n",
            (unsigned long)b, (unsigned long)bitarr->num_of_bits);

    errno = EDOM;

    exit(EXIT_FAILURE);
  }

  bitarr->words[bindex(b)] &= ~((word_t)1 << (boffset(b)));
}

char bit_array_get_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
  if(b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "bit_array.c: bit_array_get_bit() - "
                    "out of bounds error (index: %lu; length: %lu)\n",
            (unsigned long)b, (unsigned long)bitarr->num_of_bits);

    errno = EDOM;

    exit(EXIT_FAILURE);
  }

  return (bitarr->words[bindex(b)] >> (boffset(b))) & 0x1;
}

/* set all elements of data to zero */
void bit_array_fill_zeros(BIT_ARRAY* bitarr)
{
  bit_index_t num_of_bytes = nwords(bitarr->num_of_bits) * sizeof(word_t);
  memset(bitarr->words, 0, num_of_bytes);
}

/* set all elements of data to one */
void bit_array_fill_ones(BIT_ARRAY* bitarr)
{
  bit_index_t num_of_words = nwords(bitarr->num_of_bits);
  bit_index_t num_of_bytes = num_of_words * sizeof(word_t);
  memset(bitarr->words, 0xFF, num_of_bytes);

  mask_top_word(bitarr, num_of_words);
}

// To string method (remember to free the result!)
char* bit_array_to_string(BIT_ARRAY* bitarr)
{
  char* str = (char*) malloc(sizeof(char) * (bitarr->num_of_bits + 1));

  bit_index_t i;
  
  for(i = 0; i < bitarr->num_of_bits; i++)
  {
    str[i] = bit_array_get_bit(bitarr, i) ? '1' : '0';
  }

  str[bitarr->num_of_bits] = '\0';

  return str;
}

BIT_ARRAY* bit_array_clone(BIT_ARRAY* bitarr)
{
  BIT_ARRAY* cpy = (BIT_ARRAY*) malloc(sizeof(BIT_ARRAY));

  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  cpy->num_of_bits = bitarr->num_of_bits;
  cpy->words = (word_t*) malloc(sizeof(word_t) * num_of_words);

  // Copy across bits
  memcpy(cpy->words, bitarr->words, num_of_words*sizeof(word_t));

  return cpy;
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

  if(new_num_of_words != old_num_of_words)
  {
    // Need to change the amount of memory used
    bitarr->words = realloc(bitarr->words, new_num_of_words * sizeof(word_t));
    
    if(bitarr->words == NULL)
    {
      // error - could not allocate enough memory
      errno = ENOMEM;
      return 0;
    }

    if(new_num_of_words > old_num_of_words)
    {
      bit_index_t new_bytes = (new_num_of_words - old_num_of_words)
                              * sizeof(word_t);
      memset(bitarr->words + old_num_of_words, 0x0, new_bytes);
    }
  }

  // Mask top word
  mask_top_word(bitarr, new_num_of_words);

  return 1;
}

//
// Logic operators
//

// Destination can be the same as one or both of the sources
void bit_array_and(BIT_ARRAY* dst, BIT_ARRAY* src1, BIT_ARRAY* src2)
{
  if(dst->num_of_bits != src1->num_of_bits ||
     src1->num_of_bits != src2->num_of_bits)
  {
    // error
    fprintf(stderr, "bit_array.c: bit_array_and() : "
                    "dst, src1 and src2 must be of the same length\n");
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(src1->num_of_bits);
  word_addr_t i;

  for(i = 0; i < num_of_words; i++)
  {
    dst->words[i] = src1->words[i] & src2->words[i];
  }
}

// Destination can be the same as one or both of the sources
void bit_array_or(BIT_ARRAY* dst, BIT_ARRAY* src1, BIT_ARRAY* src2)
{
  if(dst->num_of_bits != src1->num_of_bits ||
     src1->num_of_bits != src2->num_of_bits)
  {
    // error
    fprintf(stderr, "bit_array.c: bit_array_and() : "
                    "dst, src1 and src2 must be of the same length\n");
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(src1->num_of_bits);
  word_addr_t i;

  for(i = 0; i < num_of_words; i++)
  {
    dst->words[i] = src1->words[i] | src2->words[i];
  }
}

// Destination can be the same as one or both of the sources
void bit_array_xor(BIT_ARRAY* dst, BIT_ARRAY* src1, BIT_ARRAY* src2)
{
  if(dst->num_of_bits != src1->num_of_bits ||
     src1->num_of_bits != src2->num_of_bits)
  {
    // error
    fprintf(stderr, "bit_array.c: bit_array_and() : "
                    "dst, src1 and src2 must be of the same length\n");
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(src1->num_of_bits);
  word_addr_t i;

  for(i = 0; i < num_of_words; i++)
  {
    dst->words[i] = src1->words[i] ^ src2->words[i];
  }
}

// Destination can be the same as the source
void bit_array_not(BIT_ARRAY* dst, BIT_ARRAY* src)
{
  if(dst->num_of_bits != src->num_of_bits)
  {
    // error
    fprintf(stderr, "bit_array.c: bit_array_and() : "
                    "dst and src1 must be of the same length\n");
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(dst->num_of_bits);
  word_addr_t i;

  for(i = 0; i < num_of_words; i++)
  {
    dst->words[i] = ~(src->words[i]);
  }
}

// Compare two bit arrays by value stored
// arrays do not have to be the same length (e.g. 101 (5) > 00000011 (3))
int bit_array_compare(BIT_ARRAY* bitarr1, BIT_ARRAY* bitarr2)
{
  word_addr_t nwords1 = nwords(bitarr1->num_of_bits);
  word_addr_t nwords2 = nwords(bitarr2->num_of_bits);

  word_addr_t max_words = MAX(nwords1, nwords2);

  word_addr_t i;
  word_t word1, word2;

  for(i = max_words-1; i >= 0; i--)
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
  }

  return 0;
}

uint64_t bit_array_word64(BIT_ARRAY* bitarr, bit_index_t start)
{
  // Bounds checking
  if(start >= bitarr->num_of_bits)
  {
    fprintf(stderr, "bit_array.c: bit_array_word64() - out of bounds error "
                    "(index: %lu, length: %lu)\n",
                    (unsigned long)start, (unsigned long)bitarr->num_of_bits);
    exit(EXIT_FAILURE);
  }

  word_addr_t word_index = bindex(start);
  word_t start_offset = boffset(start);

  uint64_t result = bitarr->words[word_index] >> start_offset;

  if(start + start_offset < bitarr->num_of_bits)
  {
    result |= bitarr->words[word_index+1] << (WORD_SIZE - start_offset);
  }

  return result;
}

uint32_t bit_array_word32(BIT_ARRAY* bitarr, bit_index_t start)
{
  return (uint32_t)bit_array_word64(bitarr, start);
}

uint16_t bit_array_word16(BIT_ARRAY* bitarr, bit_index_t start)
{
  return (uint16_t)bit_array_word64(bitarr, start);
}

uint8_t bit_array_word8(BIT_ARRAY* bitarr, bit_index_t start)
{
  return (uint8_t)bit_array_word64(bitarr, start);
}

word_t bit_array_get_word(BIT_ARRAY* bitarr, bit_index_t start)
{
  return (word_t)bit_array_word64(bitarr, start);
}

void bit_array_set_word(BIT_ARRAY* bitarr, bit_index_t start, word_t word)
{
  // Bounds checking
  if(start >= bitarr->num_of_bits)
  {
    fprintf(stderr, "bit_array.c: bit_array_set_word() - out of bounds error "
                    "(index: %lu, length: %lu)\n",
                    (unsigned long)start, (unsigned long)bitarr->num_of_bits);
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = bindex(bitarr->num_of_bits);

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
        = (word >> word_offset) |
          (bitarr->words[word_index] & (WORD_MAX << word_offset));
    }
  }

  // Mask top word
  mask_top_word(bitarr, num_of_words);
}

// destination and source may be the same bit_array
// and src/dst regions may overlap
void bit_array_copy(BIT_ARRAY* dst, bit_index_t dstindx,
                    BIT_ARRAY* src, bit_index_t srcindx,
                    bit_index_t length)
{
  // Bounds checking
  if(srcindx + length >= src->num_of_bits ||
     dstindx + length >= dst->num_of_bits)
  {
    fprintf(stderr, "bit_array.c: bit_array_copy() - out of bounds error "
                    "(dstindx: %lu, srcindx: %lu, length: %lu)\n",
                    (unsigned long)dstindx, (unsigned long)srcindx,
                    (unsigned long)length);
    exit(EXIT_FAILURE);
  }

  // Num of full words to copy
  word_addr_t num_of_words = length / WORD_SIZE;
  word_addr_t i;

  word_offset_t bits_in_last_word = boffset(length);

  if(dst == src && srcindx < dstindx)
  {
    // Work left to right
    for(i = 0; i < num_of_words; i++)
    {
      word_t word = bit_array_get_word(src, srcindx+i*WORD_SIZE);
      bit_array_set_word(dst, dstindx+i*WORD_SIZE, word);
    }

    if(bits_in_last_word > 0)
    {
      word_t src_word = bit_array_get_word(src, srcindx+i*WORD_SIZE);
      word_t dst_word = bit_array_get_word(dst, dstindx+i*WORD_SIZE);

      word_t mask = BIT_MASK(bits_in_last_word);
      word_t word = (dst_word & ~mask) | (src_word & mask);

      bit_array_set_word(dst, dstindx+num_of_words*WORD_SIZE, word);
    }

    mask_top_word(dst, bindex(dst->num_of_bits));
  }
  else
  {
    // Work right to left
    for(i = num_of_words-1; i >= 0; i--)
    {
      word_t word = bit_array_get_word(src, srcindx+length-i*WORD_SIZE);
      bit_array_set_word(dst, dstindx+length-i*WORD_SIZE, word);
    }

    if(bits_in_last_word > 0)
    {
      word_t src_word = bit_array_get_word(src, srcindx);
      word_t dst_word = bit_array_get_word(dst, dstindx);

      word_t mask = BIT_MASK(bits_in_last_word)
                    << (WORD_SIZE - bits_in_last_word);

      word_t word = (dst_word & ~mask) | (src_word & mask);

      bit_array_set_word(dst, dstindx, word);
    }
  }
}

void bit_array_fill_region(BIT_ARRAY* bitarr,
                           bit_index_t start, bit_index_t length,
                           word_t fill, bit_index_t spacing)
{
  // Bounds checking
  if(start + length >= bitarr->num_of_bits)
  {
    fprintf(stderr, "bit_array.c: bit_array_zero_region() - out of bounds error "
                    "(start: %lu, length: %lu, size: %lu)\n",
                    (unsigned long)start, (unsigned long)length,
                    (unsigned long)bitarr->num_of_bits);
    exit(EXIT_FAILURE);
  }

  bit_index_t pos = start;

  for(; pos+spacing <= start+length; pos += spacing)
  {
    bit_array_set_word(bitarr, pos, fill);
  }

  word_offset_t bits_remaining = start+length - pos;

  if(bits_remaining > 0)
  {
    word_t mask = BIT_MASK(bits_remaining);
    word_t dest_word = bit_array_get_word(bitarr, pos);
    word_t fill_word = (fill & mask) | (dest_word & ~mask);

    bit_array_set_word(bitarr, pos, fill_word);
  }

  mask_top_word(bitarr, bindex(bitarr->num_of_bits));
}

// Clear all the bits in a region
void bit_array_clear_region(BIT_ARRAY* bitarr,
                            bit_index_t start, bit_index_t length)
{
  bit_array_fill_region(bitarr, start, length, (word_t)0x0, sizeof(word_t)*8);
}

// Set all the bits in a region
void bit_array_set_region(BIT_ARRAY* bitarr,
                          bit_index_t start, bit_index_t length)
{
  bit_array_fill_region(bitarr, start, length, WORD_MAX, sizeof(word_t)*8);
}

// Saves bit array to a file
// file format is [8 bytes: for number of elements in array][data]
// returns the number of bytes written
bit_index_t bit_array_save(BIT_ARRAY* bitarr, FILE* f)
{
  bit_index_t num_of_bytes = nbytes(bitarr->num_of_bits);
  bit_index_t bytes_written = 0;

  // Write 8 bytes to store the number of bits in the array
  bytes_written += fwrite(&bitarr->num_of_bits, 8, 1, f);
  // Write the array
  bytes_written += fwrite(bitarr->words, 1, num_of_bytes, f);

  return bytes_written;
}

// Reads bit array from a file
// file format is [8 bytes: for number of elements in array][data]
// returns bit array or NULL on failure
BIT_ARRAY* bit_array_load(FILE* f)
{
  bit_index_t items_read;

  // Create bit_array
  BIT_ARRAY* bitarr = (BIT_ARRAY*) malloc(sizeof(BIT_ARRAY));

  // Read in number of bits
  items_read = fread(&bitarr->num_of_bits, sizeof(bit_index_t), 1, f);

  if(items_read != 1)
  {
    free(bitarr);
    return NULL;
  }

  // Calculate the number of bytes required
  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  // Malloc memory for bit_array and read in
  bitarr->words = malloc(num_of_words * sizeof(word_t));

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
  mask_top_word(bitarr, num_of_words);

  return bitarr;
}


//
// Experimental - haven't check below here
//

// Return 0 if there was an overflow error, 1 otherwise
char bit_array_add(BIT_ARRAY* dst, BIT_ARRAY* src1, BIT_ARRAY* src2)
{
  word_addr_t nwords1 = nwords(src1->num_of_bits);
  word_addr_t nwords2 = nwords(src2->num_of_bits);

  word_addr_t max_words = MAX(nwords1, nwords2);

  word_addr_t dst_words = nwords(dst->num_of_bits);

  char carry = 0;

  word_addr_t i;
  word_t word1, word2;
  
  for(i = 0; i < max_words; i++)
  {
    word1 = (i < nwords1 ? src1->words[i] : 0);
    word2 = (i < nwords2 ? src2->words[i] : 0);

    word_t result = word1 + word2 + carry;
    carry = (result < src1->words[i] || result < src2->words[i]) ? 1 : 0;

    // Check we can store this result
    if(i < dst_words-1)
    {
      dst->words[i] = result;
    }
    else if(i >= dst_words || carry)
    {
      // overflow error
      return 0;
    }
    else
    {
      // Check last word (i == dst_words-1)
      unsigned int bits_on_last_word = boffset(dst->num_of_bits);

      if(bits_on_last_word > 0 && BIT_MASK(bits_on_last_word) < result)
      {
        // overflow error
        return 0;
      }
    }
  }

  if(carry)
  {
    dst->words[max_words] = 1;
  }

  // Zero the rest of dst
  for(i = max_words+1; i < dst_words; i++)
  {
    dst->words[i] = 0;
  }

  return 1;
}

bit_index_t bit_array_length(BIT_ARRAY* bit_arr)
{
  return bit_arr->num_of_bits;
}

// If there is an overflow, bit array will be set to all 1s and 0 is returned
// Returns 0 if there was an overflow, 1 otherwise
char bit_array_increment(BIT_ARRAY* bitarr)
{
  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  char carry = 1;

  word_addr_t i;
  for(i = 0; i < num_of_words-1; i++)
  {
    if(bitarr->words[i]+1 < bitarr->words[i])
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
    unsigned int bits_in_last_word = boffset(bitarr->num_of_bits-1)+1;
    word_t mask = BIT_MASK(bits_in_last_word);
    word_t prev_last_word = bitarr->words[num_of_words-1] & mask;

    // Increment
    bitarr->words[num_of_words-1] = (prev_last_word+1) & mask;

    if(prev_last_word == mask)
    {
      // overflow
      return 0;
    }
  }

  return 1;
}

// If there is an underflow, bit array will be set to all 0s and 0 is returned
// Returns 0 if there was an underflow, 1 otherwise
char bit_array_decrement(BIT_ARRAY* bitarr)
{
  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  word_addr_t i;
  for(i = 0; i < num_of_words-1; i++)
  {
    if(bitarr->words[i] > 0)
    {
      bitarr->words[i]--;

      i--;
      while(i >= 0)
      {
        bitarr->words[i--] = ~0;
      }
      
      return 1;
    }
  }
  
  // Must subtract from last word
  unsigned int bits_in_last_word = boffset(bitarr->num_of_bits-1)+1;
  word_t mask = BIT_MASK(bits_in_last_word);
  word_t prev_last_word = bitarr->words[num_of_words-1] & mask;
  
  if(prev_last_word == 0)
  {
    // underflow
    // number unchanged
    return 0;
  }
  else
  {
    bitarr->words[num_of_words-1] = prev_last_word - 1;
    return 1;
  }
}
