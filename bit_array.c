/*
 bit_array.c
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
  unsigned long num_of_bits;
};

// sizeof gives size in bytes (8 bits per byte)
int WORD_SIZE = sizeof(word_t) * 8;

// Index of word
inline word_addr_t bindex(bit_index_t b) { return b / WORD_SIZE; }

// Offset within a word (values up to 64 most likely)
inline unsigned int boffset(bit_index_t b) { return b % WORD_SIZE; }

// Number of words required to store so many bits
inline word_addr_t nwords(bit_index_t b)
{
  return (b + WORD_SIZE - 1) / WORD_SIZE;
}

//#define BIT_MASK(length)  (((word_t)1 << (length))-1) // overflows
#define BIT_MASK(length)  ((word_t)ULONG_MAX >> (WORD_SIZE-(length)))

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
  if(b < 0 || b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "bit_array.c: bit_array_set_bit() - "
            "out of bounds error (index: %lu; length: %lu)\n",
            b, bitarr->num_of_bits);

    errno = EDOM;

    exit(EXIT_FAILURE);
  }

  bitarr->words[bindex(b)] |= ((word_t)1 << (boffset(b)));
}

void bit_array_clear_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
  if(b < 0 || b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "bit_array.c: bit_array_clear_bit() - "
                    "out of bounds error (index: %lu; length: %lu)\n",
            b, bitarr->num_of_bits);

    errno = EDOM;

    exit(EXIT_FAILURE);
  }

  bitarr->words[bindex(b)] &= ~((word_t)1 << (boffset(b)));
}

char bit_array_get_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
  if(b < 0 || b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "bit_array.c: bit_array_get_bit() - "
                    "out of bounds error (index: %lu; length: %lu)\n",
            b, bitarr->num_of_bits);

    errno = EDOM;

    exit(EXIT_FAILURE);
  }

  return (bitarr->words[bindex(b)] >> (boffset(b))) & 0x1;
}

/* set all elements of data to zero */
void bit_array_fill_zeros(BIT_ARRAY* bitarr)
{
  //size_t num_of_bytes = (bitarr->num_of_bits / 8) + 1;
  size_t num_of_bytes = nwords(bitarr->num_of_bits) * sizeof(word_t);
  memset(bitarr->words, 0, num_of_bytes);
}

/* set all elements of data to one */
void bit_array_fill_ones(BIT_ARRAY* bitarr)
{
  size_t num_of_bytes = nwords(bitarr->num_of_bits) * sizeof(word_t);
  memset(bitarr->words, 0xFF, num_of_bytes);
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

/*
void bit_array_copy(BIT_ARRAY* dest, bit_index_t dstindx,
                    BIT_ARRAY* src, bit_index_t srcindx, bit_index_t length)
{
  
}
*/


// Enlarge or shrink the size of a bit array
// Shrinking will free some memory if it is large
// Enlarging an array will add zeros to the end of it
// returns 1 on success, 0 on failure
char bit_array_resize(BIT_ARRAY* bitarr, bit_index_t new_num_of_bits)
{
  bit_index_t old_num_of_bits = bitarr->num_of_bits;
  bitarr->num_of_bits = new_num_of_bits;

  word_addr_t old_num_of_words = nwords(old_num_of_bits);
  word_addr_t new_num_of_words = nwords(new_num_of_bits);

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
  }

  // if we are growing - need to zero new bits
  if(new_num_of_bits > old_num_of_bits)
  {
    // zero entire words first
    if(new_num_of_words > old_num_of_words)
    {
      memset(bitarr->words + old_num_of_words, 0,
             (new_num_of_words - old_num_of_words) * sizeof(word_t));
    }
    
    // zero bits on the end of what used to be the last word
    unsigned int bits_on_last_word = boffset(old_num_of_bits);

    if(bits_on_last_word > 0)
    {
      bitarr->words[old_num_of_words-1] &= BIT_MASK(bits_on_last_word);
    }
  }
  
  return 1;
}

//
// Logic operators
//

void bit_array_and(BIT_ARRAY* dest, BIT_ARRAY* src1, BIT_ARRAY* src2)
{
  if(dest->num_of_bits != src1->num_of_bits ||
     src1->num_of_bits != src2->num_of_bits)
  {
    // error
    fprintf(stderr, "bit_array.c: bit_array_and() : "
                    "dest, src1 and src2 must be of the same length\n");
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(src1->num_of_bits);

  word_addr_t i;
  for(i = 0; i < num_of_words; i++)
  {
    dest->words[i] = src1->words[i] & src2->words[i];
  }
}

void bit_array_or(BIT_ARRAY* dest, BIT_ARRAY* src1, BIT_ARRAY* src2)
{
  if(dest->num_of_bits != src1->num_of_bits ||
     src1->num_of_bits != src2->num_of_bits)
  {
    // error
    fprintf(stderr, "bit_array.c: bit_array_and() : "
                    "dest, src1 and src2 must be of the same length\n");
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(src1->num_of_bits);

  word_addr_t i;
  for(i = 0; i < num_of_words; i++)
  {
    dest->words[i] = src1->words[i] | src2->words[i];
  }
}

void bit_array_xor(BIT_ARRAY* dest, BIT_ARRAY* src1, BIT_ARRAY* src2)
{
  if(dest->num_of_bits != src1->num_of_bits ||
     src1->num_of_bits != src2->num_of_bits)
  {
    // error
    fprintf(stderr, "bit_array.c: bit_array_and() : "
                    "dest, src1 and src2 must be of the same length\n");
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(src1->num_of_bits);

  word_addr_t i;
  for(i = 0; i < num_of_words; i++)
  {
    dest->words[i] = src1->words[i] ^ src2->words[i];
  }
}

void bit_array_not(BIT_ARRAY* dest, BIT_ARRAY* src)
{
  if(dest->num_of_bits != src->num_of_bits)
  {
    // error
    fprintf(stderr, "bit_array.c: bit_array_and() : "
                    "dest and src1 must be of the same length\n");
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(dest->num_of_bits);

  word_addr_t i;
  for(i = 0; i < num_of_words; i++)
  {
    dest->words[i] = ~(src->words[i]);
  }
}


inline word_t get_word(BIT_ARRAY* bitarr, word_addr_t word_index,
                       word_addr_t nwords)
{
  if(word_index >= nwords)
  {
    return 0;
  }
  
  unsigned int offset;
  
  if(word_index == nwords - 1 &&
     (offset = boffset(bitarr->num_of_bits)) != 0)
  {
    // get masked
    return bitarr->words[word_index] & BIT_MASK(offset);
  }
  else
  {
    return bitarr->words[word_index];
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
    word1 = get_word(bitarr1, i, nwords1);
    word2 = get_word(bitarr2, i, nwords2);

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

// Return 0 if there was an overflow error, 1 otherwise
char bit_array_add(BIT_ARRAY* dest, BIT_ARRAY* src1, BIT_ARRAY* src2)
{
  word_addr_t nwords1 = nwords(src1->num_of_bits);
  word_addr_t nwords2 = nwords(src2->num_of_bits);

  word_addr_t max_words = MAX(nwords1, nwords2);

  word_addr_t dest_words = nwords(dest->num_of_bits);

  char carry = 0;

  word_addr_t i;
  word_t word1, word2;
  
  for(i = 0; i < max_words; i++)
  {
    word1 = get_word(src1, i, nwords1);
    word2 = get_word(src2, i, nwords2);

    word_t result = word1 + word2 + carry;
    carry = (result < src1->words[i] || result < src2->words[i]) ? 1 : 0;

    // Check we can store this result
    if(i < dest_words-1)
    {
      dest->words[i] = result;
    }
    else if(i >= dest_words || carry)
    {
      // overflow error
      return 0;
    }
    else
    {
      // Check last word (i == dest_words-1)
      unsigned int bits_on_last_word = boffset(dest->num_of_bits);

      if(bits_on_last_word > 0 && BIT_MASK(bits_on_last_word) < result)
      {
        // overflow error
        return 0;
      }
    }
  }

  if(carry)
  {
    dest->words[max_words] = 1;
  }

  // Zero the rest of dest
  for(i = max_words+1; i < dest_words; i++)
  {
    dest->words[i] = 0;
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
        bitarr->words[i--] = ULONG_MAX;
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

long bit_array_get_long(BIT_ARRAY* bitarr, bit_index_t start)
{
  // Bounds checking
  if(start >= bitarr->num_of_bits)
  {
    fprintf(stderr, "bit_array.c: bit_array_get_long() - out of bounds error "
                    "(index: %lu, length: %lu)\n", start, bitarr->num_of_bits);
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  word_addr_t word_index = bindex(start);
  unsigned int start_offset = boffset(start);

  long result = get_word(bitarr, word_index++, num_of_words) >> start_offset;

  unsigned int offset = WORD_SIZE - start_offset;

  // 64 bits in a long
  while(offset < 64)
  {
    result |= get_word(bitarr, word_index++, num_of_words) << offset;
    offset += WORD_SIZE;
  }

  return result;
}

int bit_array_get_int(BIT_ARRAY* bitarr, bit_index_t start)
{
  // Bounds checking
  if(start >= bitarr->num_of_bits)
  {
    fprintf(stderr, "bit_array.c: bit_array_get_long() - out of bounds error "
                    "(index: %lu, length: %lu)\n", start, bitarr->num_of_bits);
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  word_addr_t word_index = bindex(start);
  unsigned int start_offset = boffset(start);

  int result
    = ((get_word(bitarr, word_index++, num_of_words) >> start_offset) & UINT_MAX);

  unsigned int offset = WORD_SIZE - start_offset;

  // 32 bits in an int
  while(offset < 32)
  {
    result |= (get_word(bitarr, word_index++, num_of_words) << offset) & UINT_MAX;
    offset += WORD_SIZE;
  }

  return result;
}

char bit_array_get_char(BIT_ARRAY* bitarr, bit_index_t start)
{
  // Bounds checking
  if(start >= bitarr->num_of_bits)
  {
    fprintf(stderr, "bit_array.c: bit_array_get_long() - out of bounds error "
                    "(index: %lu, length: %lu)\n", start, bitarr->num_of_bits);
    exit(EXIT_FAILURE);
  }

  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  word_addr_t word_index = bindex(start);
  unsigned int start_offset = boffset(start);

  char result = get_word(bitarr, word_index, num_of_words) >> start_offset;
  result |= get_word(bitarr, word_index+1, num_of_words) << (WORD_SIZE - start_offset);

  return result;
}
