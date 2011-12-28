/*
 bit_array.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h> // memset

#include "bit_array.h"

//
// For internal use
//

// sizeof gives size in bytes (8 bits per byte)
int WORD_SIZE = sizeof(word_t) * 8;

// Index of word
inline word_addr_t bindex(bit_index_t b) { return b / WORD_SIZE; }

// Offset within a word (values up to 64 most likely)
inline unsigned int boffset(bit_index_t b) { return b % WORD_SIZE; }

// Number of words required to store so many bits
inline word_addr_t nwords(bit_index_t b) { return (b + WORD_SIZE - 1) / WORD_SIZE; }


//
// Constructor
//
BIT_ARRAY* bit_array_create(bit_index_t nbits)
{
  BIT_ARRAY* bitarr = (BIT_ARRAY*) malloc(sizeof(BIT_ARRAY));
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
    fprintf(stderr, "bit_array.c: bit_array_create() - "
                    "cannot allocate enough memory (requested %lu bytes)\n",
            num_of_words);

    exit(EXIT_FAILURE);
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
    // bounds error
    fprintf(stderr, "bit_array.c: bit_array_set_bit() - "
            "out of bound error (index: %lu; length: %lu)\n",
            b, bitarr->num_of_bits);

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
                    "out of bound error (index: %lu; length: %lu)\n",
            b, bitarr->num_of_bits);

    exit(EXIT_FAILURE);
  }

  bitarr->words[bindex(b)] &= ~((word_t)1 << (boffset(b)));
}

char bit_array_get_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
  return (bitarr->words[bindex(b)] >> (boffset(b))) & 0x1;
}

/* set all elements of data to zero */
void bit_array_fill_zeros(BIT_ARRAY* bitarr)
{
  size_t num_of_bytes = (bitarr->num_of_bits / 8) + 1;
  memset(bitarr->words, 0, num_of_bytes);
}

/* set all elements of data to one */
void bit_array_fill_ones(BIT_ARRAY* bitarr)
{
  size_t num_of_bytes = (bitarr->num_of_bits / 8) + 1;
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

BIT_ARRAY* bit_array_copy(BIT_ARRAY* bitarr)
{
  BIT_ARRAY* cpy = (BIT_ARRAY*) malloc(sizeof(BIT_ARRAY));

  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  cpy->num_of_bits = bitarr->num_of_bits;
  cpy->words = (word_t*) malloc(sizeof(word_t) * num_of_words);

  // Copy across bits
  memcpy(cpy->words, bitarr->words, num_of_words);

  return cpy;
}

void bit_array_resize(BIT_ARRAY* bitarr, bit_index_t new_num_of_bits)
{
  bit_index_t old_num_of_bits = bitarr->num_of_bits;
  bitarr->num_of_bits = new_num_of_bits;

  word_addr_t old_num_of_words = nwords(old_num_of_bits);
  word_addr_t new_num_of_words = nwords(new_num_of_bits);

  printf("resize %lu -> %lu (words: %lu -> %lu)\n",
         old_num_of_bits, new_num_of_bits, old_num_of_words, new_num_of_words);

  if(new_num_of_words != old_num_of_words)
  {
    // Need to change the amount of memory used
    bitarr->words = realloc(bitarr->words, new_num_of_words * sizeof(word_t));
    
    if(bitarr->words == NULL)
    {
      // error - could not allocate enough memory
      fprintf(stderr, "bit_array.c: bit_array_resize() - "
                      "cannot allocate enough memory (requested %lu bytes)\n",
              new_num_of_words * sizeof(word_t));

      exit(EXIT_FAILURE);
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
    bit_index_t bits_on_last_word = boffset(old_num_of_bits);

    if(bits_on_last_word > 0)
    {
      bitarr->words[old_num_of_words-1] &= (1l << bits_on_last_word)-1;
    }
  }
}
