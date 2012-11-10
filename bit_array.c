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
#include <assert.h>
#include <time.h> // needed for rand()
#include <unistd.h>  // need for getpid() function

#include "bit_array.h"
#include "lookup3.h"

//
// Tables of constants
//

// byte reverse look up table
static const word_t reverse_table[256] = 
{
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
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
  0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};

// Morton table for interleaving bytes
static const word_t morton_table0[256] = 
{
  0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015,
  0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
  0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115,
  0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
  0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415,
  0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
  0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515,
  0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
  0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015,
  0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
  0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115,
  0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
  0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415,
  0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
  0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515,
  0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,
  0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015,
  0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
  0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115,
  0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
  0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415,
  0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
  0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515,
  0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
  0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015,
  0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
  0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115,
  0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
  0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415,
  0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
  0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515,
  0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555,
};

// Morton table for interleaving bytes, shifted left 1 bit
static const word_t morton_table1[256] = 
{
  0x0000, 0x0002, 0x0008, 0x000A, 0x0020, 0x0022, 0x0028, 0x002A,
  0x0080, 0x0082, 0x0088, 0x008A, 0x00A0, 0x00A2, 0x00A8, 0x00AA,
  0x0200, 0x0202, 0x0208, 0x020A, 0x0220, 0x0222, 0x0228, 0x022A,
  0x0280, 0x0282, 0x0288, 0x028A, 0x02A0, 0x02A2, 0x02A8, 0x02AA,
  0x0800, 0x0802, 0x0808, 0x080A, 0x0820, 0x0822, 0x0828, 0x082A,
  0x0880, 0x0882, 0x0888, 0x088A, 0x08A0, 0x08A2, 0x08A8, 0x08AA,
  0x0A00, 0x0A02, 0x0A08, 0x0A0A, 0x0A20, 0x0A22, 0x0A28, 0x0A2A,
  0x0A80, 0x0A82, 0x0A88, 0x0A8A, 0x0AA0, 0x0AA2, 0x0AA8, 0x0AAA,
  0x2000, 0x2002, 0x2008, 0x200A, 0x2020, 0x2022, 0x2028, 0x202A,
  0x2080, 0x2082, 0x2088, 0x208A, 0x20A0, 0x20A2, 0x20A8, 0x20AA,
  0x2200, 0x2202, 0x2208, 0x220A, 0x2220, 0x2222, 0x2228, 0x222A,
  0x2280, 0x2282, 0x2288, 0x228A, 0x22A0, 0x22A2, 0x22A8, 0x22AA,
  0x2800, 0x2802, 0x2808, 0x280A, 0x2820, 0x2822, 0x2828, 0x282A,
  0x2880, 0x2882, 0x2888, 0x288A, 0x28A0, 0x28A2, 0x28A8, 0x28AA,
  0x2A00, 0x2A02, 0x2A08, 0x2A0A, 0x2A20, 0x2A22, 0x2A28, 0x2A2A,
  0x2A80, 0x2A82, 0x2A88, 0x2A8A, 0x2AA0, 0x2AA2, 0x2AA8, 0x2AAA,
  0x8000, 0x8002, 0x8008, 0x800A, 0x8020, 0x8022, 0x8028, 0x802A,
  0x8080, 0x8082, 0x8088, 0x808A, 0x80A0, 0x80A2, 0x80A8, 0x80AA,
  0x8200, 0x8202, 0x8208, 0x820A, 0x8220, 0x8222, 0x8228, 0x822A,
  0x8280, 0x8282, 0x8288, 0x828A, 0x82A0, 0x82A2, 0x82A8, 0x82AA,
  0x8800, 0x8802, 0x8808, 0x880A, 0x8820, 0x8822, 0x8828, 0x882A,
  0x8880, 0x8882, 0x8888, 0x888A, 0x88A0, 0x88A2, 0x88A8, 0x88AA,
  0x8A00, 0x8A02, 0x8A08, 0x8A0A, 0x8A20, 0x8A22, 0x8A28, 0x8A2A,
  0x8A80, 0x8A82, 0x8A88, 0x8A8A, 0x8AA0, 0x8AA2, 0x8AA8, 0x8AAA,
  0xA000, 0xA002, 0xA008, 0xA00A, 0xA020, 0xA022, 0xA028, 0xA02A,
  0xA080, 0xA082, 0xA088, 0xA08A, 0xA0A0, 0xA0A2, 0xA0A8, 0xA0AA,
  0xA200, 0xA202, 0xA208, 0xA20A, 0xA220, 0xA222, 0xA228, 0xA22A,
  0xA280, 0xA282, 0xA288, 0xA28A, 0xA2A0, 0xA2A2, 0xA2A8, 0xA2AA,
  0xA800, 0xA802, 0xA808, 0xA80A, 0xA820, 0xA822, 0xA828, 0xA82A,
  0xA880, 0xA882, 0xA888, 0xA88A, 0xA8A0, 0xA8A2, 0xA8A8, 0xA8AA,
  0xAA00, 0xAA02, 0xAA08, 0xAA0A, 0xAA20, 0xAA22, 0xAA28, 0xAA2A,
  0xAA80, 0xAA82, 0xAA88, 0xAA8A, 0xAAA0, 0xAAA2, 0xAAA8, 0xAAAA,
};


#ifdef DEBUG
#include <execinfo.h>
void print_trace()
{
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace(array, 10);
  strings = backtrace_symbols (array, (int)size);

  fprintf(stderr, "Obtained %zd stack frames.\n", size);

  for(i = 0; i < size; i++)
    fprintf(stderr, "%s\n", strings[i]);

  free(strings);
}
#endif

const bit_index_t BIT_INDEX_MIN = 0;
const bit_index_t BIT_INDEX_MAX = ~(bit_index_t)0;

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
// WORD_MAX >> (WORD_SIZE-(length)) gives WORD_MAX if length is 0 -- have to check
#define BIT_MASK(length) (word_t)(length == 0 ? 0 : WORD_MAX >> (WORD_SIZE-(length)))

#if defined(_WIN32)
#define TRAILING_ZEROS(x) _BitScanForward(x)
#define LEADING_ZEROS(x) _BitScanReverse(x)
#else
#define TRAILING_ZEROS(x) __builtin_ctzl(x)
#define LEADING_ZEROS(x) __builtin_clzl(x)
#endif

struct BIT_ARRAY {
  word_t* words;
  bit_index_t num_of_bits;
  // For more efficient allocation we use realloc only to double size --
  // not for adding every word.  Initial size is INIT_CAPACITY_WORDS.
  word_addr_t capacity_in_words;
};

// Have we initialised with srand() ?
char rand_initiated = 0;

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
  num_of_words = MAX(1, num_of_words);
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

  if(bits_set < WORD_SIZE && start > 0)
  {
    word >>= bits_set;

    // Prevent overwriting the bits we've just set
    // by setting 'start' as the upper bound for the number of bits to write
    word_offset_t bits_remaining = MIN(WORD_SIZE - bits_set, start);
    word_t mask = BIT_MASK(bits_remaining);

    bitarr->words[0] = (word & mask) | (bitarr->words[0] & ~mask);
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

  // DEV: this can be more efficient by setting whole words at a time with
  //      just bitarr->words[i] = fill
  // even better use mem_set
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

  while(bitarr->capacity_in_words * WORD_SIZE < nbits)
  {
    bitarr->capacity_in_words *= 2;
  }

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

// Change the size of a bit array. Enlarging an array will add zeros
// to the end of it. Returns 1 on success, 0 on failure (e.g. not enough memory)
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
    assert(bitarr->capacity_in_words > 0);

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
// Get, set, clear, assign and toggle individual bits
//

// Get the value of a bit (returns 0 or 1)
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

// set a bit (to 1) at position b
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

// clear a bit (to 0) at position b
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

// If bit is 0 -> 1, if bit is 1 -> 0.  AKA 'flip'
void bit_array_toggle_bit(BIT_ARRAY* bitarr, bit_index_t b)
{
  if(b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "%s:%i:bit_array_toggle_bit(): Out of bounds "
                    "(index: %lu; length: %lu)\n", __FILE__, __LINE__,
            (unsigned long)b, (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }

  bitarr->words[bindex(b)] ^= ~((word_t)0x1 << (boffset(b)));
}

// If char c != 0, set bit; otherwise clear bit
void bit_array_assign_bit(BIT_ARRAY* bitarr, bit_index_t b, char c)
{
  if(b >= bitarr->num_of_bits)
  {
    // out of bounds error
    fprintf(stderr, "%s:%i:bit_array_assign_bit(): Out of bounds "
                    "(index: %lu; length: %lu)\n", __FILE__, __LINE__,
            (unsigned long)b, (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }

  //c ? bit_array_set_bit(bitarr, b) : bit_array_clear_bit(bitarr, b);
  word_offset_t offset = boffset(b);
  word_t w = (word_t)c << offset;
  word_t m = (word_t)0x1 << offset;
  word_addr_t index = bindex(b);
  bitarr->words[index] = (bitarr->words[index] & ~m) | w;
}


//
// Set, clear and toggle several bits at once
//

// Set multiple bits at once. 
// e.g. set bits 1, 20 & 31: bit_array_set_bits(bitarr, 3, 1,20,31);
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
// e.g. clear bits 1, 20 & 31: bit_array_clear_bits(bitarr, 3, 1,20,31);
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

// Toggle multiple bits at once
// e.g. toggle bits 1, 20 & 31: bit_array_toggle_bits(bitarr, 3, 1,20,31);
void bit_array_toggle_bits(BIT_ARRAY* bitarr, size_t n, ...)
{
  va_list argptr;
  va_start(argptr, n);

  size_t i;
  for(i = 0; i < n; i++)
  {
    bit_index_t bit_index = va_arg(argptr, bit_index_t);
    bit_array_toggle_bit(bitarr, bit_index);
  }

  va_end(argptr);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}


//
// Set, clear and toggle all bits in a region
//

// Set all the bits in a region
void bit_array_set_region(BIT_ARRAY* bitarr,
                          bit_index_t start, bit_index_t length)
{
  _bit_array_fill_region(bitarr, start, length, WORD_MAX, WORD_SIZE);
}


// Clear all the bits in a region
void bit_array_clear_region(BIT_ARRAY* bitarr,
                            bit_index_t start, bit_index_t length)
{
  _bit_array_fill_region(bitarr, start, length, (word_t)0x0, WORD_SIZE);
}

// Toggle all the bits in a region
void bit_array_toggle_region(BIT_ARRAY* bitarr,
                             bit_index_t start, bit_index_t length)
{
  // Bounds checking
  if(start + length > bitarr->num_of_bits)
  {
    fprintf(stderr, "%s:%i:bit_array_toggle_region(): Out of bounds error "
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

  // DEV: this can be more efficient by setting whole words at a time with
  //      just bitarr->words[i] = ~bitarr->words[i]
  bit_index_t pos = start;

  for(; pos+WORD_SIZE <= start+length; pos += WORD_SIZE)
  {
    word_t w = _bit_array_get_word(bitarr, pos);
    _bit_array_set_word(bitarr, pos, ~w);
  }

  word_offset_t bits_remaining = start+length - pos;

  if(bits_remaining > 0)
  {
    word_t mask = BIT_MASK(bits_remaining);
    word_t dest_word = _bit_array_get_word(bitarr, pos);
    word_t fill_word = dest_word ^ mask;
    // same as:      = (~dest_word & mask) | (dest_word & ~mask);

    _bit_array_set_word(bitarr, pos, fill_word);
  }

  _mask_top_word(bitarr, nwords(bitarr->num_of_bits));

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}


//
// Set, clear and toggle all bits at once
//

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

// Set all 1 bits to 0, and all 0 bits to 1. AKA flip
void bit_array_toggle_all(BIT_ARRAY* bitarr)
{
  word_addr_t num_of_words = nwords(bitarr->num_of_bits);
  
  word_addr_t i;
  for(i = 0; i < num_of_words; i++)
  {
    bitarr->words[i] ^= WORD_MAX;
  }

  _mask_top_word(bitarr, num_of_words);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

//
// Get / set a word at a time
//

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

void bit_array_set_word64(BIT_ARRAY* bitarr, bit_index_t start, uint64_t word)
{
  _bit_array_set_word(bitarr, start, (word_t)word);
}

//
// Number/position of bits set
//

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
char bit_array_parity(const BIT_ARRAY* bitarr)
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


//
// Strings and printing
//

// Construct a BIT_ARRAY from a substring with given on and off characters.
void bit_array_from_substr(BIT_ARRAY* bitarr, const char* str, size_t len,
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
      exit(EXIT_FAILURE);
    }
  }

  if(bitarr->num_of_bits != len)
  {
    if(!bit_array_resize(bitarr, len))
    {
      fprintf(stderr, "%s:%i:bit_array_from_substr(): Ran out of memory\n",
              __FILE__, __LINE__);
      errno = ENOMEM;
      exit(EXIT_FAILURE);
    }
  }

  bit_array_clear_all(bitarr);

  // BitArray is now all 0s -- just set the 1s
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
}

// From string method
void bit_array_from_str(BIT_ARRAY* bitarr, const char* str)
{
  bit_array_from_substr(bitarr, str, strlen(str), '1', '0', 0);
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
    if(!bit_array_resize(dst, max_bits))
    {
      fprintf(stderr, "%s:%i:bit_array_and(): Ran out of memory\n",
              __FILE__, __LINE__);
      errno = ENOMEM;
      exit(EXIT_FAILURE);
    }
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
    if(!bit_array_resize(dst, max_bits))
    {
      fprintf(stderr, "%s:%i:bit_array_or_xor(): Ran out of memory\n",
              __FILE__, __LINE__);
      errno = ENOMEM;
      exit(EXIT_FAILURE);
    }
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

// If dst is longer than src, top bits are set to 1
void bit_array_not(BIT_ARRAY* dst, const BIT_ARRAY* src)
{
  if(dst->num_of_bits < src->num_of_bits)
  {
    if(!bit_array_resize(dst, src->num_of_bits))
    {
      fprintf(stderr, "%s:%i:bit_array_not(): Ran out of memory\n",
              __FILE__, __LINE__);
      errno = ENOMEM;
      exit(EXIT_FAILURE);
    }
  }

  word_addr_t src_words = nwords(src->num_of_bits);
  word_addr_t dst_words = nwords(dst->num_of_bits);

  word_addr_t i;

  for(i = 0; i < src_words; i++)
  {
    dst->words[i] = ~(src->words[i]);
  }

  // Set remaining words to 1s
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

// Compare two bit arrays by value stored, with index 0 being the Least
// Significant Bit (LSB). Arrays do not have to be the same length.
// Example: ..0101 (5) > ...0011 (3) [index 0 is LSB at right hand side]
// Sorts on length if all zeros: (0,0) < (0,0,0)
// returns:
//  >0 iff bitarr1 > bitarr2
//   0 iff bitarr1 == bitarr2
//  <0 iff bitarr1 < bitarr2
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
      break;
    }
  }

  if(bitarr1->num_of_bits > bitarr2->num_of_bits)
  {
    return 1;
  }
  else if(bitarr1->num_of_bits < bitarr2->num_of_bits)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}

// Compare two bit arrays by value stored, with index 0 being the Most
// Significant Bit (MSB). Arrays do not have to be the same length.
// Example: 10.. > 01.. [index 0 is MSB at left hand side]
// Sorts on length if all zeros: (0,0) < (0,0,0)
// returns:
//  >0 iff bitarr1 > bitarr2
//   0 iff bitarr1 == bitarr2
//  <0 iff bitarr1 < bitarr2
int bit_array_other_endian_cmp(const BIT_ARRAY* bitarr1, const BIT_ARRAY* bitarr2)
{
  word_addr_t nwords1 = nwords(bitarr1->num_of_bits);
  word_addr_t nwords2 = nwords(bitarr2->num_of_bits);

  word_addr_t max_words = MAX(nwords1, nwords2);

  word_addr_t i;
  word_t word1, word2;

  for(i = 0; i < max_words; i++)
  {
    word1 = (i < nwords1 ? bitarr1->words[i] : 0);
    word2 = (i < nwords2 ? bitarr2->words[i] : 0);

    word1 = _bit_array_reverse_word(word1);
    word2 = _bit_array_reverse_word(word2);

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
      break;
    }
  }

  if(bitarr1->num_of_bits > bitarr2->num_of_bits)
  {
    return 1;
  }
  else if(bitarr1->num_of_bits < bitarr2->num_of_bits)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}


//
// Shift left / right
//

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
// Interleave
//

// dst cannot point to the same bit array as src1 or src2
// src1, src2 may point to the same bit array
// abcd 1234 -> a1b2c3d4
// 0011 0000 -> 00001010
// 1111 0000 -> 10101010
// 0101 1010 -> 01100110
void bit_array_interleave(BIT_ARRAY* dst, const BIT_ARRAY* src1,
                                          const BIT_ARRAY* src2)
{
  if(dst == src1 || dst == src2)
  {
    fprintf(stderr, "%s:%i:bit_array_interleave(): dst cannot point to "
                    "src1 or src2\n", __FILE__, __LINE__);
    exit(EXIT_FAILURE);
  }
  else if(src1->num_of_bits != src2->num_of_bits)
  {
    fprintf(stderr, "%s:%i:bit_array_interleave(): Behaviour undefined when"
                    "src1 length (%lu) != src2 length (%lu)",
            __FILE__, __LINE__,
            (unsigned long)src1->num_of_bits, (unsigned long)src2->num_of_bits);
    exit(EXIT_FAILURE);
  }

  if(dst->num_of_bits < 2 * src1->num_of_bits)
  {
    if(!bit_array_resize(dst, 2 * src1->num_of_bits))
    {
      fprintf(stderr, "%s:%i:bit_array_interleave(): Ran out of memory\n",
              __FILE__, __LINE__);
      errno = ENOMEM;
      exit(EXIT_FAILURE);
    }
  }

  word_addr_t i, j;
  word_addr_t num_words = nwords(src1->num_of_bits);

  for(i = 0, j = 0; i < num_words; i++)
  {
    word_t a = src1->words[i];
    word_t b = src2->words[i];

    dst->words[j++] =  morton_table0[(a      ) & 0xff] |
                       morton_table1[(b      ) & 0xff] |
                      (morton_table0[(a >>  8) & 0xff] << 16) |
                      (morton_table1[(b >>  8) & 0xff] << 16) |
                      (morton_table0[(a >> 16) & 0xff] << 32) |
                      (morton_table1[(b >> 16) & 0xff] << 32) |
                      (morton_table0[(a >> 24) & 0xff] << 48) |
                      (morton_table1[(b >> 24) & 0xff] << 48);

    dst->words[j++] =  morton_table0[(a >> 32) & 0xff] |
                       morton_table1[(b >> 32) & 0xff] |
                      (morton_table0[(a >> 40) & 0xff] << 16) |
                      (morton_table1[(b >> 40) & 0xff] << 16) |
                      (morton_table0[(a >> 48) & 0xff] << 32) |
                      (morton_table1[(b >> 48) & 0xff] << 32) |
                      (morton_table0[(a >> 56)       ] << 48) |
                      (morton_table1[(b >> 56)       ] << 48);
  }

  #ifdef DEBUG
  _bit_array_check_top_word(dst);
  #endif
}

//
// Reverse -- coords may wrap around
//

void bit_array_reverse_region(BIT_ARRAY* bitarr,
                              bit_index_t start, bit_index_t length)
{
  // Check bounds
  if(start >= bitarr->num_of_bits)
  {
    fprintf(stderr, "%s:%i:bit_array_reverse_region(): "
                    "start index is out of bounds (start: %lu; bits: %lu)\n",
            __FILE__, __LINE__,
            (unsigned long)start, (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }
  else if(length > bitarr->num_of_bits)
  {
    fprintf(stderr, "%s:%i:bit_array_reverse_region(): "
                    "region length is too big (length: %lu; bits: %lu)\n",
            __FILE__, __LINE__,
            (unsigned long)length, (unsigned long)bitarr->num_of_bits);
    errno = EDOM;
    exit(EXIT_FAILURE);
  }

  if(length == 0)
  {
    return;
  }

  bit_index_t left = start;
  bit_index_t right = (start + length - WORD_SIZE) % bitarr->num_of_bits; 

  while(length >= WORD_SIZE)
  {
    // Swap entire words
    word_t left_word = _bit_array_get_word_cyclic(bitarr, left);
    word_t right_word = _bit_array_get_word_cyclic(bitarr, right);
  
    // reverse words individually
    left_word = _bit_array_reverse_word(left_word);
    right_word = _bit_array_reverse_word(right_word);

    // Swap
    _bit_array_set_word_cyclic(bitarr, left, right_word);
    _bit_array_set_word_cyclic(bitarr, right, left_word);
  
    // Update
    left = (left + WORD_SIZE) % bitarr->num_of_bits;
    right = (right < WORD_SIZE ? right + bitarr->num_of_bits : right) - WORD_SIZE;
    length = (length < 2 * WORD_SIZE ? 0 : length - 2 * WORD_SIZE);
  }

  if(length == 0)
  {
    return;
  }

  // printf("left: %i, right: %i, len: %i\n", (int)left, (int)right, (int)length);

  // Remaining bits
  word_t word = _bit_array_get_word_cyclic(bitarr, left);
  word_t rev = _bit_array_reverse_word(word);
  rev >>= WORD_SIZE - length;
  word_t mask = BIT_MASK(length);

  word = (rev & mask) | (word & ~mask);
  _bit_array_set_word_cyclic(bitarr, left, word);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

void bit_array_reverse(BIT_ARRAY* bitarr)
{
  if(bitarr->num_of_bits > 0)
  {
    bit_array_reverse_region(bitarr, 0, bitarr->num_of_bits);
  }
}

//
// Random
//

// Set bits randomly with probability prob : 0 <= prob <= 1
void bit_array_random(BIT_ARRAY* bitarr, float prob)
{
  if(prob == 1)
  {
    bit_array_set_all(bitarr);
    return;
  }

  // rand() generates number between 0 and RAND_MAX inclusive
  // therefore we want to check if rand() <= p
  long p = RAND_MAX * prob;

  if(!rand_initiated)
  {
    // Initialise random number generator
    srand(time(NULL) + getpid());
    rand_initiated = 1;
  }

  word_addr_t w;
  word_offset_t o;

  word_addr_t num_of_words = nwords(bitarr->num_of_bits);

  // Initialise to zero
  memset(bitarr->words, 0x0, num_of_words * sizeof(word_t));

  for(w = 0; w < num_of_words - 1; w++)
  {
    for(o = 0; o < WORD_SIZE; o++)
    {
      if(rand() <= p)
      {
        bitarr->words[w] |= ((word_t)0x1 << o);
      }
    }
  }

  // Top word
  word_offset_t bits_in_last_word = _bits_in_top_word(bitarr->num_of_bits);
  w = num_of_words - 1;

  for(o = 0; o < bits_in_last_word; o++)
  {
    if(rand() <= p)
    {
      bitarr->words[w] |= ((word_t)0x1 << o);
    }
  }

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}

// Shuffle the bits in an array randomly
void bit_array_shuffle(BIT_ARRAY* bitarr)
{
  if(bitarr->num_of_bits == 0)
    return;

  if(!rand_initiated)
  {
    // Initialise random number generator
    srand(time(NULL) + getpid());
    rand_initiated = 1;
  }

  bit_index_t i, j;

  for(i = bitarr->num_of_bits - 1; i > 0; i--)
  {
    j = (bit_index_t)rand() % i;
  
    // Swap i and j
    char x = (bitarr->words[bindex(i)] >> boffset(i)) & 0x1;
    char y = (bitarr->words[bindex(j)] >> boffset(j)) & 0x1;

    if(!y)
      bitarr->words[bindex(i)] &= ~((word_t)0x1 << boffset(i));
    else
      bitarr->words[bindex(i)] |= (word_t)0x1 << boffset(i);
  
    if(!x)
      bitarr->words[bindex(j)] &= ~((word_t)0x1 << boffset(j));
    else
      bitarr->words[bindex(j)] |= (word_t)0x1 << boffset(j);
  }

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
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
          fprintf(stderr, "%s:%i:_bit_array_arithmetic(): Ran out of memory\n",
                  __FILE__, __LINE__);
          errno = ENOMEM;
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
      fprintf(stderr, "%s:%i:bit_array_add(): Ran out of memory\n",
              __FILE__, __LINE__);
      errno = ENOMEM;
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
      fprintf(stderr, "%s:%i:bit_array_subtract(): Ran out of memory\n",
              __FILE__, __LINE__);
      errno = ENOMEM;
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
      if(!bit_array_resize(bitarr, bitarr->num_of_bits + 1))
      {
        fprintf(stderr, "%s:%i:bit_array_increment(): Ran out of memory\n",
                __FILE__, __LINE__);
        errno = ENOMEM;
        exit(EXIT_FAILURE);
      }

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
// Hash function
//

// Pass seed as 0 on first call, pass previous hash value if rehashing due
// to a collision
// Using bob jenkins hash lookup3
uint64_t bit_array_hash(const BIT_ARRAY* bitarr, uint64_t seed)
{
  uint32_t *pc = (uint32_t*)&seed;
  uint32_t *pb = pc+1;

  // Round up length to number 32bit words
  hashword2((uint32_t*)bitarr->words, (bitarr->num_of_bits + 31) / 32, pc, pb);

  // XOR with array length. This ensures arrays with different length but same
  // contents have different hash values
  seed ^= bitarr->num_of_bits;

  return seed;
}


//
// Experimental - development - full of bugs
//

void bit_array_cycle_right(BIT_ARRAY* bitarr, bit_index_t cycle_dist)
{
  if(bitarr->num_of_bits == 0)
  {
    return;
  }

  cycle_dist = cycle_dist % bitarr->num_of_bits;

  if(cycle_dist == 0)
  {
    return;
  }

  bit_index_t mid = bitarr->num_of_bits - cycle_dist;
  bit_array_reverse_region(bitarr, 0, mid);
  bit_array_reverse_region(bitarr, mid, cycle_dist);
  bit_array_reverse(bitarr);
}

void bit_array_cycle_left(BIT_ARRAY* bitarr, bit_index_t cycle_dist)
{
  if(bitarr->num_of_bits == 0)
  {
    return;
  }

  cycle_dist = cycle_dist % bitarr->num_of_bits;

  if(cycle_dist == 0)
  {
    return;
  }

  bit_index_t len = bitarr->num_of_bits - cycle_dist;
  bit_array_reverse_region(bitarr, 0, cycle_dist);
  bit_array_reverse_region(bitarr, cycle_dist, len);
  bit_array_reverse(bitarr);
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
 /*
Approaches:
1) Reverse each sections (0..p,p+1..n), then reverse the whole array
2) Rotate words using GCD, so only a right shift needed, loop through 0..n
   doing right shift
 */
void dev_bit_array_cycle_right(BIT_ARRAY* bitarr, bit_index_t cycle_dist)
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

void dev_bit_array_cycle_left(BIT_ARRAY* bitarr, bit_index_t cycle_dist)
{
  bit_array_cycle_right(bitarr, bitarr->num_of_bits - cycle_dist);

  #ifdef DEBUG
  _bit_array_check_top_word(bitarr);
  #endif
}
