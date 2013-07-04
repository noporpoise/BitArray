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
#include <limits.h> // ULONG_MAX
#include <errno.h>
#include <signal.h> // needed for kill
#include <string.h> // memset
#include <assert.h>
#include <time.h> // needed for rand()
#include <unistd.h>  // need for getpid() for getting setting rand number
#include <ctype.h>  // need for tolower()

// Windows includes
#if defined(_WIN32)
#include <intrin.h>
#endif

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

const bit_index_t BIT_INDEX_MIN = 0;
const bit_index_t BIT_INDEX_MAX = ~(bit_index_t)0;

//
// Macros
//

// WORD_SIZE is the number of bits per word
// sizeof gives size in bytes (8 bits per byte)
#define WORD_SIZE 64
// #define WORD_SIZE (sizeof(word_t) * 8)

// TRAILING_ZEROS is number of least significant zeros
// LEADING_ZEROS is number of most significant zeros
// POPCOUNT is number of bits set

#if defined(_WIN32)
static word_t __inline windows_ctz(word_t x)
{
  word_offset_t r = 0;
  _BitScanReverse64(&r, x);
  return r;
}

static word_t __inline windows_clz(word_t x)
{
  word_offset_t r = 0;
  _BitScanForward64(&r, x);
  return r;
}

// See http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
static word_t __inline windows_popcount(word_t w)
{
  w = w - ((w >> 1) & (word_t)~(word_t)0/3);
  w = (w & (word_t)~(word_t)0/15*3) + ((w >> 2) & (word_t)~(word_t)0/15*3);
  w = (w + (w >> 4)) & (word_t)~(word_t)0/255*15;
  c = (word_t)(w * ((word_t)~(word_t)0/255)) >> (sizeof(word_t) - 1) * 8;
}

static word_t __inline windows_parity(word_t w)
{
  w ^= w >> 1;
  w ^= w >> 2;
  w = (w & 0x1111111111111111UL) * 0x1111111111111111UL;
  return (w >> 60) & 1;
}

#define TRAILING_ZEROS(x) windows_ctz(x)
#define LEADING_ZEROS(x) windows_clz(x)
#define POPCOUNT(x) windows_popcountl(x)
#define PARITY(x) windows_parity(x)
#else
#define TRAILING_ZEROS(x) ((x) == 0 ? WORD_SIZE : (unsigned)__builtin_ctzl(x))
#define LEADING_ZEROS(x) ((x) == 0 ? WORD_SIZE : (unsigned)__builtin_clzl(x))
#define POPCOUNT(x) (unsigned)__builtin_popcountl(x)
#define PARITY(x) (unsigned)__builtin_parityl(x)
#endif

#define MIN(a, b)  (((a) <= (b)) ? (a) : (b))
#define MAX(a, b)  (((a) >= (b)) ? (a) : (b))

// Make this a power of two
#define INIT_CAPACITY_WORDS 2

// Round a number up to the nearest number that is a power of two
#define ROUNDUP2POW(x) (0x1 << (64 - LEADING_ZEROS(x)))

// word of all 1s
#define WORD_MAX  (~(word_t)0)

// if w == 0 return WORD_SIZE, otherwise index of top bit set
#define TOP_BIT_SET(w) (w == 0 ? WORD_SIZE : WORD_SIZE - LEADING_ZEROS(w) - 1)

// WORD_MAX >> (WORD_SIZE-(length)) gives WORD_MAX instead of 0 if length is 0
// need to check for length == 0
#define BIT_MASK(length) (length == 0 ? (word_t)0 : WORD_MAX >> (WORD_SIZE-(length)))

// Index of word
#define bindex(b) ((b) / WORD_SIZE)

// Offset within a word (values up to 64 most likely)
#define boffset(b) ((b) % WORD_SIZE)

#define SET_REGION(arr,start,len)    _set_region((arr),(start),(len),FILL_REGION)
#define CLEAR_REGION(arr,start,len)  _set_region((arr),(start),(len),ZERO_REGION)
#define TOGGLE_REGION(arr,start,len) _set_region((arr),(start),(len),SWAP_REGION)

// A possibly faster way to combine two words with a mask
//#define MASK_MERGE(a,b,abits) ((a & abits) | (b & ~abits))
#define MASK_MERGE(a,b,abits) (b ^ ((a ^ b) & abits))

// Have we initialised with srand() ?
static char rand_initiated = 0;

static void _seed_rand()
{
  if(!rand_initiated)
  {
    // Initialise random number generator
    srand((unsigned int)time(NULL) + getpid());
    rand_initiated = 1;
  }
}

//
// Common internal functions
//

// Number of words required to store a given number of bits
// 0 -> 0
// 1..WORD_SIZE -> 1
// WORD_SIZE+1..2*WORD_SIZE -> 2 etc.
word_addr_t nwords(bit_index_t b)
{
  return (b + WORD_SIZE - 1) / WORD_SIZE;
}

// Number of bytes required to store a given number of bits
word_addr_t nbytes(bit_index_t b)
{
  return (b + 7) / 8;
}

static inline word_offset_t _bits_in_top_word(bit_index_t b)
{
  return (b == 0 ? 0 : boffset(b - 1) + 1);
}

// Mostly used for debugging
static inline void _print_word(word_t word, FILE* out)
{
  word_offset_t i;
  for(i = 0; i < WORD_SIZE; i++)
  {
    fprintf(out, "%c", ((word >> i) & (word_t)0x1) == 0 ? '0' : '1');
  }
}

// prints right to left
static inline char* _word_to_str(word_t word, char str[WORD_SIZE+1])
{
  word_offset_t i;
  for(i = 0; i < WORD_SIZE; i++)
  {
    str[WORD_SIZE-i-1] = ((word >> i) & (word_t)0x1) == 0 ? '0' : '1';
  }
  str[WORD_SIZE] = '\0';
  return str;
}

// Used in debugging
#define VALIDATE_BIT_ARRAY(a) validate_bitarr((a), __FILE__, __LINE__)

void validate_bitarr(BIT_ARRAY *arr, char *file, int lineno)
{
  // Check top word is masked
  word_addr_t tw = arr->num_of_words == 0 ? 0 : arr->num_of_words - 1;
  bit_index_t top_bits = _bits_in_top_word(arr->num_of_bits);

  if(arr->words[tw] > BIT_MASK(top_bits))
  {
    fprintf(stderr, "%s:%i:VALIDATE_BIT_ARRAY(): Fail -- "
                    "expected %i bits in top word[%i]:\n",
            file, lineno, (int)top_bits, (int)tw);

    _print_word(arr->words[tw], stderr);
    fprintf(stderr, "\n");

    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }

  // Check num of words is correct
  word_addr_t num_words = nwords(arr->num_of_bits);
  if(num_words != arr->num_of_words)
  {
    fprintf(stderr, "%s:%i:VALIDATE_BIT_ARRAY(): Fail -- num of words wrong "
                    "[bits: %i, word: %i, actual words: %i]\n",
            file, lineno,
            (int)arr->num_of_bits, (int)num_words, (int)arr->num_of_words);

    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }
}

// Reverse a word
static inline word_t _reverse_word(word_t word)
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

static inline void _mask_top_word(BIT_ARRAY* bitarr)
{
  // Mask top word
  word_addr_t num_of_words = MAX(1, bitarr->num_of_words);
  word_offset_t bits_active = _bits_in_top_word(bitarr->num_of_bits);
  bitarr->words[num_of_words-1] &= BIT_MASK(bits_active);
}

// Bounds check


static inline void _bounds_check_start(const BIT_ARRAY* bitarr,
                                       bit_index_t start,
                                       const char* file, int line,
                                       const char* func)
{
  if(start >= bitarr->num_of_bits)
  {
    fprintf(stderr, "%s:%i:%s() - out of bounds error "
                    "(index: %lu, num_of_bits: %lu)\n",
            file, line, func,
            (unsigned long)start, (unsigned long)bitarr->num_of_bits);
    kill(getpid(), SIGABRT);
    // errno = EDOM;
    // exit(EXIT_FAILURE);
  }
}

static inline void _bounds_check_length(const BIT_ARRAY* bitarr,
                                        bit_index_t length,
                                        const char* file, int line,
                                        const char* func)
{
  if(length > bitarr->num_of_bits)
  {
    fprintf(stderr, "%s:%i:%s() - out of bounds error "
                    "(length: %lu, num_of_bits: %lu)\n",
            file, line, func,
            (unsigned long)length, (unsigned long)bitarr->num_of_bits);
    kill(getpid(), SIGABRT);
    // errno = EDOM;
    // exit(EXIT_FAILURE);
  }
}

static inline void _bounds_check_offset(const BIT_ARRAY* bitarr,
                                        bit_index_t start, bit_index_t len,
                                        const char* file, int line,
                                        const char* func)
{
  if(start + len > bitarr->num_of_bits)
  {
    fprintf(stderr, "%s:%i:%s() - out of bounds error "
                    "(start: %lu; length: %lu; num_of_bits: %lu)\n",
            file, line, func,
            (unsigned long)start, (unsigned long)len,
            (unsigned long)bitarr->num_of_bits);
    kill(getpid(), SIGABRT);
    // errno = EDOM;
    // exit(EXIT_FAILURE);
  }
}

//
// Get and set words (internal use only -- no bounds checking)
//

static inline word_t _get_word(const BIT_ARRAY* bitarr, bit_index_t start)
{
  word_addr_t word_index = bindex(start);
  word_offset_t word_offset = boffset(start);

  word_t result = bitarr->words[word_index] >> word_offset;

  word_offset_t bits_taken = WORD_SIZE - word_offset;

  // word_offset is now the number of bits we need from the next word
  // Check the next word has at least some bits
  if(word_offset > 0 && start + bits_taken < bitarr->num_of_bits)
  {
    result |= bitarr->words[word_index+1] << (WORD_SIZE - word_offset);
  }

  return result;
}

// Set 64 bits from a particular start position
// Doesn't extend bit array
static inline void _set_word(BIT_ARRAY* bitarr, bit_index_t start, word_t word)
{
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
  
    if(word_index+1 < bitarr->num_of_words)
    {
      bitarr->words[word_index+1]
        = (word >> (WORD_SIZE - word_offset)) |
          (bitarr->words[word_index+1] & (WORD_MAX << word_offset));
    }
  }

  // Mask top word
  _mask_top_word(bitarr);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

static inline void _set_byte(BIT_ARRAY *bitarr, bit_index_t start, uint8_t byte)
{
  word_t w = _get_word(bitarr, start);
  _set_word(bitarr, start, (w & ~(word_t)0xff) | byte);
}

// 4 bits
static inline void _set_nibble(BIT_ARRAY *bitarr, bit_index_t start,
                               uint8_t nibble)
{
  word_t w = _get_word(bitarr, start);
  _set_word(bitarr, start, (w & ~(word_t)0xf) | nibble);
}

// Wrap around
static inline word_t _get_word_cyclic(const BIT_ARRAY* bitarr, bit_index_t start)
{
  word_t word = _get_word(bitarr, start);

  bit_index_t bits_taken = bitarr->num_of_bits - start;

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
static inline void _set_word_cyclic(BIT_ARRAY* bitarr,
                                    bit_index_t start, word_t word)
{
  _set_word(bitarr, start, word);

  bit_index_t bits_set = bitarr->num_of_bits - start;

  if(bits_set < WORD_SIZE && start > 0)
  {
    word >>= bits_set;

    // Prevent overwriting the bits we've just set
    // by setting 'start' as the upper bound for the number of bits to write
    word_offset_t bits_remaining = MIN(WORD_SIZE - bits_set, start);
    word_t mask = BIT_MASK(bits_remaining);

    bitarr->words[0] = MASK_MERGE(word, bitarr->words[0], mask);
  }
}

//
// Fill a region (internal use only)
//

// FillAction is fill with 0 or 1 or toggle
typedef enum {ZERO_REGION, FILL_REGION, SWAP_REGION} FillAction;

static inline void _set_region(BIT_ARRAY* bitarr, bit_index_t start,
                               bit_index_t length, FillAction action)
{
  if(length == 0) return;

  word_addr_t first_word = bindex(start);
  word_addr_t last_word = bindex(start+length-1);
  word_offset_t foffset = boffset(start);
  word_offset_t loffset = boffset(start+length-1);

  if(first_word == last_word)
  {
    word_t mask = BIT_MASK(length) << foffset;

    switch(action)
    {
      case ZERO_REGION: bitarr->words[first_word] &= ~mask; break;
      case FILL_REGION: bitarr->words[first_word] |=  mask; break;
      case SWAP_REGION: bitarr->words[first_word] ^=  mask; break;
    }
  }
  else
  {
    // Set first word
    switch(action)
    {
      case ZERO_REGION: bitarr->words[first_word] &=  BIT_MASK(foffset); break;
      case FILL_REGION: bitarr->words[first_word] |= ~BIT_MASK(foffset); break;
      case SWAP_REGION: bitarr->words[first_word] ^= ~BIT_MASK(foffset); break;
    }

    word_addr_t i;

    // Set whole words
    switch(action)
    {
      case ZERO_REGION:
        for(i = first_word + 1; i < last_word; i++)
          bitarr->words[i] = (word_t)0;
        break;
      case FILL_REGION:
        for(i = first_word + 1; i < last_word; i++)
          bitarr->words[i] = WORD_MAX;
        break;
      case SWAP_REGION:
        for(i = first_word + 1; i < last_word; i++)
          bitarr->words[i] ^= WORD_MAX;
        break;
    }

    // Set last word
    switch(action)
    {
      case ZERO_REGION: bitarr->words[last_word] &= ~BIT_MASK(loffset+1); break;
      case FILL_REGION: bitarr->words[last_word] |=  BIT_MASK(loffset+1); break;
      case SWAP_REGION: bitarr->words[last_word] ^=  BIT_MASK(loffset+1); break;
    }
  }
}



//
// Constructor
//

// If cannot allocate memory, set errno to ENOMEM, return NULL
BIT_ARRAY* bit_array_alloc(BIT_ARRAY* bitarr, bit_index_t nbits)
{
  bitarr->num_of_bits = nbits;
  bitarr->num_of_words = nwords(nbits);
  bitarr->capacity_in_words = ROUNDUP2POW(bitarr->num_of_words);
  bitarr->words = (word_t*)calloc(bitarr->capacity_in_words, sizeof(word_t));
  if(bitarr->words == NULL) {
    errno = ENOMEM;
    return NULL;
  }
  return bitarr;
}

void bit_array_dealloc(BIT_ARRAY* bitarr)
{
  free(bitarr->words);
}

// If cannot allocate memory, set errno to ENOMEM, return NULL
BIT_ARRAY* bit_array_create(bit_index_t nbits)
{
  BIT_ARRAY* bitarr = (BIT_ARRAY*) malloc(sizeof(BIT_ARRAY));

  // error if could not allocate enough memory
  if(bitarr == NULL || bit_array_alloc(bitarr, nbits) == NULL)
  {
    if(bitarr != NULL) free(bitarr);
    errno = ENOMEM;
    return NULL;
  }

  #ifdef DEBUG
  printf("Creating BIT_ARRAY (bits: %lu; allocated words: %lu; "
         "using words: %lu; WORD_SIZE: %i)\n",
         (unsigned long)nbits, (unsigned long)bitarr->capacity_in_words,
         (unsigned long)nwords(nbits), (int)WORD_SIZE);

  VALIDATE_BIT_ARRAY(bitarr);
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
  word_addr_t old_num_of_words = bitarr->num_of_words;

  word_addr_t new_num_of_words = nwords(new_num_of_bits);

  bitarr->num_of_bits = new_num_of_bits;
  bitarr->num_of_words = new_num_of_words;

  #ifdef DEBUG
  printf("Resize: old_num_of_words: %i; new_num_of_words: %i\n",
         (int)old_num_of_words, (int)new_num_of_words);
  #endif

  if(new_num_of_words > bitarr->capacity_in_words)
  {
    assert(bitarr->capacity_in_words > 0);

    // Need to change the amount of memory used
    word_addr_t old_capacity_in_words = bitarr->capacity_in_words;
    size_t old_capacity_in_bytes = old_capacity_in_words * sizeof(word_t);

    bitarr->capacity_in_words = ROUNDUP2POW(new_num_of_words);

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
    memset(bitarr->words + old_capacity_in_words, 0x0, num_bytes_to_zero);

    #ifdef DEBUG
    printf("zeroing from word %i for %i bytes\n", (int)old_capacity_in_words,
           (int)num_bytes_to_zero);
    #endif
  }
  else if(new_num_of_words < old_num_of_words)
  {
    // Shrunk -- need to zero old memory
    size_t num_bytes_to_zero = (old_num_of_words - new_num_of_words)*sizeof(word_t);
    memset(bitarr->words + new_num_of_words, 0x0, num_bytes_to_zero);
  }

  // Mask top word
  _mask_top_word(bitarr);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif

  return 1;
}

void bit_array_resize_critical(BIT_ARRAY* bitarr, bit_index_t num_of_bits,
                               const char *file, int lineno, const char *func)
{
  bit_index_t old_num_of_bits = bitarr->num_of_bits;

  if(!bit_array_resize(bitarr, num_of_bits))
  {
    fprintf(stderr, "%s:%i:%s(): Ran out of memory resizing [%lu -> %lu]\n",
            file, lineno, func,
            (unsigned long)old_num_of_bits, (unsigned long)num_of_bits);
    kill(getpid(), SIGABRT);
    // errno = ENOMEM;
    // exit(EXIT_FAILURE);
  }
}

// If bitarr length < num_bits, resizes to num_bits
char bit_array_ensure_size(BIT_ARRAY* bitarr, bit_index_t ensure_num_of_bits)
{
  if(bitarr->num_of_bits < ensure_num_of_bits)
  {
    return bit_array_resize(bitarr, ensure_num_of_bits);
  }

  return 1;
}


//
// Get, set, clear, assign and toggle individual bits
//

// Get the value of a bit (returns 0 or 1)
char _bit_array_get_bit(const char *file, int lineno,
                        const BIT_ARRAY* bitarr, bit_index_t b)
{
  _bounds_check_start(bitarr, b, file, lineno, "bit_array_get_bit");

  return bit_array_get(bitarr, b);
}

// set a bit (to 1) at position b
void _bit_array_set_bit(const char *file, int lineno,
                        BIT_ARRAY* bitarr, bit_index_t b)
{
  _bounds_check_start(bitarr, b, file, lineno, "bit_array_set_bit");

  bit_array_set(bitarr,b);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// clear a bit (to 0) at position b
void _bit_array_clear_bit(const char *file, int lineno,
                          BIT_ARRAY* bitarr, bit_index_t b)
{
  _bounds_check_start(bitarr, b, file, lineno, "bit_array_clear_bit");

  bit_array_clear(bitarr, b);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// If bit is 0 -> 1, if bit is 1 -> 0.  AKA 'flip'
void _bit_array_toggle_bit(const char *file, int lineno,
                           BIT_ARRAY* bitarr, bit_index_t b)
{
  _bounds_check_start(bitarr, b, file, lineno, "bit_array_toggle_bit");

  bit_array_toggle(bitarr, b);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// If char c != 0, set bit; otherwise clear bit
void _bit_array_assign_bit(const char *file, int lineno,
                           BIT_ARRAY* bitarr, bit_index_t b, char c)
{
  _bounds_check_start(bitarr, b, file, lineno, "bit_array_assign_bit");

  c ? bit_array_set(bitarr, b) : bit_array_clear(bitarr, b);
  /*
  // Without branching
  word_offset_t offset = boffset(b);
  word_t w = (word_t)c << offset;
  word_t m = (word_t)0x1 << offset;
  word_addr_t index = bindex(b);
  bitarr->words[index] = (bitarr->words[index] & ~m) | w;
  */
}


//
// Set, clear and toggle several bits at once
//

// Set multiple bits at once. 
// e.g. set bits 1, 20 & 31: bit_array_set_bits(bitarr, 3, 1,20,31);
void _bit_array_set_bits(const char* file, int line,
                         BIT_ARRAY* bitarr, size_t n, ...)
// void bit_array_set_bits(BIT_ARRAY* bitarr, size_t n, ...)
{
  va_list argptr;
  va_start(argptr, n);

  size_t i;
  for(i = 0; i < n; i++)
  {
    unsigned int bit_index = va_arg(argptr, unsigned int);
    _bit_array_set_bit(file, line, bitarr, bit_index);
  }

  va_end(argptr);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// Clear multiple bits at once.
// e.g. clear bits 1, 20 & 31: bit_array_clear_bits(bitarr, 3, 1,20,31);
void _bit_array_clear_bits(const char *file, int line,
                           BIT_ARRAY* bitarr, size_t n, ...)
{
  va_list argptr;
  va_start(argptr, n);

  size_t i;
  for(i = 0; i < n; i++)
  {
    unsigned int bit_index = va_arg(argptr, unsigned int);
    _bit_array_clear_bit(file, line, bitarr, bit_index);
  }

  va_end(argptr);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// Toggle multiple bits at once
// e.g. toggle bits 1, 20 & 31: bit_array_toggle_bits(bitarr, 3, 1,20,31);
void _bit_array_toggle_bits(const char *file, int line,
                            BIT_ARRAY* bitarr, size_t n, ...)
{
  va_list argptr;
  va_start(argptr, n);

  size_t i;
  for(i = 0; i < n; i++)
  {
    unsigned int bit_index = va_arg(argptr, unsigned int);
    _bit_array_toggle_bit(file, line, bitarr, bit_index);
  }

  va_end(argptr);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}


//
// Set, clear and toggle all bits in a region
//

// Set all the bits in a region
void _bit_array_set_region(const char *file, int line,
                           BIT_ARRAY* bitarr,
                           bit_index_t start,
                           bit_index_t length)
{
  // Bounds checking
  _bounds_check_offset(bitarr, start, length, file, line,
                      "bit_array_set_region");

  SET_REGION(bitarr, start, length);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}


// Clear all the bits in a region
void _bit_array_clear_region(const char *file, int line,
                             BIT_ARRAY* bitarr,
                             bit_index_t start,
                             bit_index_t length)
{
  // Bounds checking
  _bounds_check_offset(bitarr, start, length, file, line,
                      "bit_array_clear_region");

  CLEAR_REGION(bitarr, start, length);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// Toggle all the bits in a region
void _bit_array_toggle_region(const char *file, int line,
                              BIT_ARRAY* bitarr,
                              bit_index_t start,
                              bit_index_t length)
{
  // Bounds checking
  _bounds_check_offset(bitarr, start, length, file, line,
                      "bit_array_toggle_region");

  TOGGLE_REGION(bitarr, start, length);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}


//
// Set, clear and toggle all bits at once
//

// set all elements of data to one
void bit_array_set_all(BIT_ARRAY* bitarr)
{
  bit_index_t num_of_bytes = bitarr->num_of_words * sizeof(word_t);
  memset(bitarr->words, 0xFF, num_of_bytes);

  _mask_top_word(bitarr);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// set all elements of data to zero
void bit_array_clear_all(BIT_ARRAY* bitarr)
{
  memset(bitarr->words, 0, bitarr->num_of_words * sizeof(word_t));

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// Set all 1 bits to 0, and all 0 bits to 1. AKA flip
void bit_array_toggle_all(BIT_ARRAY* bitarr)
{ 
  word_addr_t i;
  for(i = 0; i < bitarr->num_of_words; i++)
  {
    bitarr->words[i] ^= WORD_MAX;
  }

  _mask_top_word(bitarr);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

//
// Get a word at a time
//

uint64_t _bit_array_get_word64(const char *file, int line,
                               const BIT_ARRAY* bitarr,
                               bit_index_t start)
{
  // Bounds checking
  _bounds_check_start(bitarr, start, file, line, "bit_array_word64");

  return (uint64_t)_get_word(bitarr, start);
}

uint32_t _bit_array_get_word32(const char *file, int line,
                               const BIT_ARRAY* bitarr,
                               bit_index_t start)
{
  // Bounds checking
  _bounds_check_start(bitarr, start, file, line, "bit_array_word32");

  return (uint32_t)_get_word(bitarr, start);
}

uint16_t _bit_array_get_word16(const char *file, int line,
                               const BIT_ARRAY* bitarr,
                               bit_index_t start)
{
  // Bounds checking
  _bounds_check_start(bitarr, start, file, line, "bit_array_word16");

  return (uint16_t)_get_word(bitarr, start);
}

uint8_t _bit_array_get_word8(const char *file, int line,
                             const BIT_ARRAY* bitarr,
                             bit_index_t start)
{
  // Bounds checking
  _bounds_check_start(bitarr, start, file, line, "bit_array_word8");

  return (uint8_t)_get_word(bitarr, start);
}

//
// Set a word at a time
//

void _bit_array_set_word64(const char *file, int line,
                           BIT_ARRAY* bitarr,
                           bit_index_t start, uint64_t word)
{
  // Bounds checking
  _bounds_check_start(bitarr, start, file, line, "bit_array_set_word64");

  _set_word(bitarr, start, (word_t)word);
}

void _bit_array_set_word32(const char *file, int line,
                           BIT_ARRAY* bitarr,
                           bit_index_t start, uint32_t word)
{
  // Bounds checking
  _bounds_check_start(bitarr, start, file, line, "bit_array_set_word32");

  word_t w = _get_word(bitarr, start);
  _set_word(bitarr, start, (w & ~(word_t)0xffffffff) | word);
}

void _bit_array_set_word16(const char *file, int line,
                           BIT_ARRAY* bitarr,
                           bit_index_t start, uint16_t word)
{
  // Bounds checking
  _bounds_check_start(bitarr, start, file, line, "bit_array_set_word16");

  word_t w = _get_word(bitarr, start);
  _set_word(bitarr, start, (w & ~(word_t)0xffff) | word);
}

void _bit_array_set_word8(const char *file, int line,
                          BIT_ARRAY* bitarr,
                          bit_index_t start, uint8_t byte)
{
  // Bounds checking
  _bounds_check_start(bitarr, start, file, line, "bit_array_set_word8");

  _set_byte(bitarr, start, byte);
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
  word_addr_t i;
  
  for(i = 0; i < bitarr->num_of_words; i++)
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
  if(bitarr->num_of_bits == 0)
  {
    return 0;
  }

  // Find last word that is greater than zero
  word_addr_t i = bitarr->num_of_words - 1;

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
  unsigned int parity = 0;

  for(w = 0; w < bitarr->num_of_words; w++)
  {
    parity ^= PARITY(bitarr->words[w]);
  }

  return (char)parity;
}

// Get the number of bits set (hamming weight)
bit_index_t bit_array_num_bits_set(const BIT_ARRAY* bitarr)
{
  word_addr_t i;
  
  bit_index_t num_of_bits_set = 0;

  for(i = 0; i < bitarr->num_of_words; i++)
  {
    if(bitarr->words[i] > 0)
    {
      num_of_bits_set += POPCOUNT(bitarr->words[i]);
    }
  }

  return num_of_bits_set;
}


// Get the number of bits set in on array and not the other.  This is equivalent
// to hamming weight of the XOR when the two arrays are the same length.
// e.g. 10101 vs 00111 => hamming distance 2 (XOR is 10010)
bit_index_t bit_array_hamming_distance(const BIT_ARRAY* arr1,
                                       const BIT_ARRAY* arr2)
{
  word_addr_t min_words = MIN(arr1->num_of_words, arr2->num_of_words);
  word_addr_t max_words = MAX(arr1->num_of_words, arr2->num_of_words);

  bit_index_t hamming_distance = 0;
  word_addr_t i;

  for(i = 0; i < min_words; i++)
  {
    hamming_distance += POPCOUNT(arr1->words[i] ^ arr2->words[i]);
  }

  if(min_words != max_words)
  {
    const BIT_ARRAY* long_arr
      = (arr1->num_of_words > arr2->num_of_words ? arr1 : arr2);

    for(i = min_words; i < max_words; i++)
    {
      hamming_distance += POPCOUNT(long_arr->words[i]);
    }
  }

  return hamming_distance;
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
  CLEAR_REGION(bitarr, 0, num_of_bits_cleared);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// Put all the 1s before all the 0s
void bit_array_sort_bits_rev(BIT_ARRAY* bitarr)
{
  bit_index_t num_of_bits_set = bit_array_num_bits_set(bitarr);

  #ifdef DEBUG
  printf("rev_sort_bits (bits set: %lu)\n", (unsigned long)num_of_bits_set);
  #endif

  bit_array_clear_all(bitarr);
  SET_REGION(bitarr, 0, num_of_bits_set);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}


//
// Strings and printing
//

// Construct a BIT_ARRAY from a substring with given on and off characters.
void _bit_array_from_substr(const char *file, int line,
                            BIT_ARRAY* bitarr, bit_index_t offset,
                            const char* str, size_t len,
                            const char *on, const char *off,
                            char left_to_right)
{
  bit_array_ensure_size(bitarr, offset + len);
  bit_array_clear_region(bitarr, offset, len);

  // BitArray region is now all 0s -- just set the 1s
  size_t i;
  bit_index_t j;

  for(i = 0; i < len; i++)
  {
    if(strchr(on, str[i]) != NULL)
    {
      j = offset + (left_to_right ? i : len - i - 1);
      bit_array_set(bitarr, j);
    }
    else if(strchr(off, str[i]) == NULL)
    {
      fprintf(stderr, "%s:%i:bit_array_from_substr(): Invalid char '%c' "
                      "(on: %s; off: %s)\n", file, line, str[i], on, off);
      kill(getpid(), SIGABRT);
      // errno = EDOM;
      // exit(EXIT_FAILURE);
    }
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// From string method
void bit_array_from_str(BIT_ARRAY* bitarr, const char* str)
{
  bit_array_from_substr(bitarr, 0, str, strlen(str), "1", "0", 1);
}

// Takes a char array to write to.  `str` must be bitarr->num_of_bits+1 in length
// Terminates string with '\0'
char* bit_array_to_str(const BIT_ARRAY* bitarr, char* str)
{
  bit_index_t i;
  
  for(i = 0; i < bitarr->num_of_bits; i++)
  {
    str[i] = bit_array_get(bitarr, i) ? '1' : '0';
  }

  str[bitarr->num_of_bits] = '\0';

  return str;
}

char* bit_array_to_str_rev(const BIT_ARRAY* bitarr, char* str)
{
  bit_index_t i;
  
  for(i = 0; i < bitarr->num_of_bits; i++)
  {
    str[i] = bit_array_get(bitarr, bitarr->num_of_bits-i-1) ? '1' : '0';
  }

  str[bitarr->num_of_bits] = '\0';

  return str;
}


// Get a string representations for a given region, using given on/off characters.
// Note: does not null-terminate
void _bit_array_to_substr(const char *file, int line,
                          const BIT_ARRAY* bitarr,
                          bit_index_t start, bit_index_t length,
                          char* str, char on, char off,
                          char left_to_right)
{
  // Bounds checking
  _bounds_check_offset(bitarr, start, length, file, line, "bit_array_to_substr");

  bit_index_t i, j;
  bit_index_t end = start + length - 1;

  for(i = 0; i < length; i++)
  {
    j = (left_to_right ? start + i : end - i);
    str[i] = bit_array_get(bitarr, j) ? on : off;
  }

//  str[length] = '\0';
}

// Print this array to a file stream.  Prints '0's and '1'.  Doesn't print newline.
void bit_array_print(const BIT_ARRAY* bitarr, FILE* fout)
{
  bit_index_t i;
  
  for(i = 0; i < bitarr->num_of_bits; i++)
  {
    fprintf(fout, "%c", bit_array_get(bitarr, i) ? '1' : '0');
  }
}

// Print a string representations for a given region, using given on/off characters.
void _bit_array_print_substr(const char *file, int line,
                             const BIT_ARRAY* bitarr,
                             bit_index_t start, bit_index_t length,
                             FILE* fout, char on, char off,
                             char left_to_right)
{
  // Bounds checking
  _bounds_check_offset(bitarr, start, length, file, line, "bit_array_print_substr");

  bit_index_t i, j;
  bit_index_t end = start + length - 1;
  
  for(i = 0; i < length; i++)
  {
    j = (left_to_right ? start + i : end - i);
    fprintf(fout, "%c", bit_array_get(bitarr, j) ? on : off);
  }
}

// Hexidecimal

char bit_array_hex_to_nibble(char c, uint8_t *b)
{
  c = tolower(c);

  if(c >= '0' && c <= '9')
  {
    *b = c - '0';
    return 1;
  }
  else if(c >= 'a' && c <= 'f')
  {
    *b = 0xa + (c - 'a');
    return 1;
  }
  else
  {
    return 0;
  }
}

char bit_array_nibble_to_hex(uint8_t b, char uppercase)
{
  if(b <= 9)
  {
    return '0' + b;
  }
  else
  {
    return (uppercase ? 'A' : 'a') + (b - 0xa);
  }
}

// Loads array from hex string
// Returns the number of bits loaded (will be chars rounded up to multiple of 4)
// (0 on failure)
bit_index_t bit_array_from_hex(BIT_ARRAY* bitarr, bit_index_t offset,
                               const char* str, size_t len)
{
  if(str[0] == '0' && tolower(str[1]) == 'x')
  {
    str += 2;
    len -= 2;
  }

  size_t i;
  for(i = 0; i < len; i++, offset += 4)
  {
    uint8_t b;
    if(bit_array_hex_to_nibble(str[i], &b))
    {
      bit_array_ensure_size(bitarr, offset + 4);
      _set_nibble(bitarr, offset, b);
    }
    else
    {
      break;
    }
  }

  return 4 * i;
}

// Returns number of characters written
size_t _bit_array_to_hex(const char *file, int line, const BIT_ARRAY* bitarr,
                         bit_index_t start, bit_index_t length,
                         char* str, char uppercase)
{
  _bounds_check_start(bitarr, start, file, line, "bit_array_to_hex");
  _bounds_check_offset(bitarr, start, length, file, line, "bit_array_to_hex");

  size_t k = 0;
  bit_index_t offset, end = start + length;

  for(offset = start; offset + WORD_SIZE <= end; offset += WORD_SIZE)
  {
    word_t w = _get_word(bitarr, offset);

    word_offset_t j;
    for(j = 0; j < 64; j += 4)
    {
      str[k++] = bit_array_nibble_to_hex((w>>j) & 0xf, uppercase);
    }
  }

  if(offset < end)
  {
    // Remaining full nibbles (4 bits)
    word_t w = _get_word(bitarr, offset);

    for(; offset + 4 <= end; offset += 4)
    {
      str[k++] = bit_array_nibble_to_hex(w & 0xf, uppercase);
      w >>= 4;
    }

    if(offset < end)
    {
      // Remaining bits
      str[k++] = bit_array_nibble_to_hex(w & BIT_MASK(end - offset), uppercase);
    }
  }

  str[k] = '\0';

  // Return number of characters written
  return k;
}

// Print bit array as hex
size_t _bit_array_print_hex(const char *file, int line,
                            const BIT_ARRAY* bitarr,
                            bit_index_t start, bit_index_t length,
                            FILE* fout, char uppercase)
{
  _bounds_check_start(bitarr, start, file, line, "bit_array_to_hex");
  _bounds_check_offset(bitarr, start, length, file, line, "bit_array_to_hex");

  size_t k = 0;
  bit_index_t offset, end = start + length;

  for(offset = start; offset + WORD_SIZE <= end; offset += WORD_SIZE)
  {
    word_t w = _get_word(bitarr, offset);

    word_offset_t j;
    for(j = 0; j < 64; j += 4)
    {
      fprintf(fout, "%c", bit_array_nibble_to_hex((w>>j) & 0xf, uppercase));
      k++;
    }
  }

  if(offset < end)
  {
    // Remaining full nibbles (4 bits)
    word_t w = _get_word(bitarr, offset);

    for(; offset + 4 <= end; offset += 4)
    {
      fprintf(fout, "%c", bit_array_nibble_to_hex(w & 0xf, uppercase));
      w >>= 4;
      k++;
    }

    if(offset < end)
    {
      // Remaining bits
      char hex = bit_array_nibble_to_hex(w & BIT_MASK(end - offset), uppercase);
      fprintf(fout, "%c", hex);
      k++;
    }
  }

  return k;
}

//
// Clone and copy
//

// Returns NULL if cannot malloc
BIT_ARRAY* bit_array_clone(const BIT_ARRAY* bitarr)
{
  BIT_ARRAY* cpy = bit_array_create(bitarr->num_of_bits);

  if(cpy == NULL)
  {
    return NULL;
  }

  // Copy across bits
  memcpy(cpy->words, bitarr->words, bitarr->num_of_words * sizeof(word_t));

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(cpy);
  #endif

  return cpy;
}

// destination and source may be the same bit_array
// and src/dst regions may overlap
static void _array_copy(BIT_ARRAY* dst, bit_index_t dstindx,
                        const BIT_ARRAY* src, bit_index_t srcindx,
                        bit_index_t length)
{
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
    printf("work left to right\n");
    #endif

    for(i = 0; i < num_of_full_words; i++)
    {
      word_t word = _get_word(src, srcindx+i*WORD_SIZE);
      _set_word(dst, dstindx+i*WORD_SIZE, word);
    }

    if(bits_in_last_word > 0)
    {
      word_t src_word = _get_word(src, srcindx+i*WORD_SIZE);
      word_t dst_word = _get_word(dst, dstindx+i*WORD_SIZE);

      word_t mask = BIT_MASK(bits_in_last_word);
      word_t word = MASK_MERGE(src_word, dst_word, mask);

      _set_word(dst, dstindx+num_of_full_words*WORD_SIZE, word);
    }
  }
  else
  {
    // Work right to left
    #ifdef DEBUG
    printf("work right to left\n");
    #endif

    for(i = 0; i < num_of_full_words; i++)
    {
      word_t word = _get_word(src, srcindx+length-(i+1)*WORD_SIZE);
      _set_word(dst, dstindx+length-(i+1)*WORD_SIZE, word);
    }

    #ifdef DEBUG
    printf("Copy %i,%i to %i\n", (int)srcindx, (int)bits_in_last_word,
                                 (int)dstindx);
    #endif

    if(bits_in_last_word > 0)
    {
      word_t src_word = _get_word(src, srcindx);
      word_t dst_word = _get_word(dst, dstindx);

      word_t mask = BIT_MASK(bits_in_last_word);
      word_t word = MASK_MERGE(src_word, dst_word, mask);
      _set_word(dst, dstindx, word);
    }
  }

  _mask_top_word(dst);
}

// destination and source may be the same bit_array
// and src/dst regions may overlap
void _bit_array_copy(const char *file, int line,
                     BIT_ARRAY* dst, bit_index_t dstindx,
                     const BIT_ARRAY* src, bit_index_t srcindx,
                     bit_index_t length)
{
  // Bounds checking in both arrays
  _bounds_check_offset(src, srcindx, length, file, line, "bit_array_copy");
  _bounds_check_offset(dst, dstindx, length, file, line, "bit_array_copy");
  
  _array_copy(dst, dstindx, src, srcindx, length);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(dst);
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
    bit_array_resize_critical(dst, max_bits, __FILE__, __LINE__,
                              "bit_array_and");
  }

  word_addr_t min_words = MIN(src1->num_of_words, src2->num_of_words);

  word_addr_t i;

  for(i = 0; i < min_words; i++)
  {
    dst->words[i] = src1->words[i] & src2->words[i];
  }

  // Set remaining bits to zero
  for(i = min_words; i < dst->num_of_words; i++)
  {
    dst->words[i] = (word_t)0;
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(dst);
  #endif
}

// Destination can be the same as one or both of the sources
static void _logical_or_xor(BIT_ARRAY* dst,
                            const BIT_ARRAY* src1,
                            const BIT_ARRAY* src2,
                            char use_xor)
{
  // Ensure dst array is big enough
  word_addr_t max_bits = MAX(src1->num_of_bits, src2->num_of_bits);

  if(dst->num_of_bits < max_bits)
  {
    bit_array_resize_critical(dst, max_bits, __FILE__, __LINE__,
                              "bit_array_xor");
  }

  word_addr_t min_words = MIN(src1->num_of_words, src2->num_of_words);
  word_addr_t max_words = MAX(src1->num_of_words, src2->num_of_words);

  word_addr_t i;

  if(use_xor)
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
    const BIT_ARRAY* longer = src1->num_of_words > src2->num_of_words ? src1 : src2;

    for(i = min_words; i < max_words; i++)
    {
      dst->words[i] = longer->words[i];
    }
  }

  // Set remaining bits to zero
  size_t size = (dst->num_of_words - max_words) * sizeof(word_t);
  memset(dst->words + max_words, 0, size);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(dst);
  #endif
}

void bit_array_or(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
  _logical_or_xor(dst, src1, src2, 0);
}

// Destination can be the same as one or both of the sources
void bit_array_xor(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
  _logical_or_xor(dst, src1, src2, 1);
}

// If dst is longer than src, top bits are set to 1
void bit_array_not(BIT_ARRAY* dst, const BIT_ARRAY* src)
{
  if(dst->num_of_bits < src->num_of_bits)
  {
    bit_array_resize_critical(dst, src->num_of_bits, __FILE__, __LINE__,
                              "bit_array_not");
  }

  word_addr_t i;

  for(i = 0; i < src->num_of_words; i++)
  {
    dst->words[i] = ~(src->words[i]);
  }

  // Set remaining words to 1s
  for(i = src->num_of_words; i < dst->num_of_words; i++)
  {
    dst->words[i] = WORD_MAX;
  }

  _mask_top_word(dst);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(dst);
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
  word_addr_t max_words = MAX(bitarr1->num_of_words, bitarr2->num_of_words);

  if(max_words == 0)
  {
    return 0;
  }

  word_addr_t i;
  word_t word1, word2;

  // i is unsigned to break when i == 0
  for(i = max_words-1; ; i--)
  {
    word1 = (i < bitarr1->num_of_words ? bitarr1->words[i] : (word_t)0);
    word2 = (i < bitarr2->num_of_words ? bitarr2->words[i] : (word_t)0);

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
  word_addr_t max_words = MAX(bitarr1->num_of_words, bitarr2->num_of_words);

  word_addr_t i;
  word_t word1, word2;

  for(i = 0; i < max_words; i++)
  {
    word1 = (i < bitarr1->num_of_words ? bitarr1->words[i] : (word_t)0);
    word2 = (i < bitarr2->num_of_words ? bitarr2->words[i] : (word_t)0);

    word1 = _reverse_word(word1);
    word2 = _reverse_word(word2);

    if(word1 > word2)
    {
      return 1;
    }
    else if(word1 < word2)
    {
      return -1;
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

// compare bitarr with (bitarr2 << pos)
// bit_array_cmp(bitarr1, bitarr2<<pos)
// returns:
//  >0 iff bitarr1 > bitarr2
//   0 iff bitarr1 == bitarr2
//  <0 iff bitarr1 < bitarr2
int bit_array_cmp_words(const BIT_ARRAY *arr1,
                        bit_index_t pos, const BIT_ARRAY *arr2)
{
  if(arr1->num_of_bits == 0 && arr2->num_of_bits == 0)
  {
    return 0;
  }

  bit_index_t top_bit1 = 0, top_bit2 = 0;

  char arr1_zero = !bit_array_find_last_set_bit(arr1, &top_bit1);
  char arr2_zero = !bit_array_find_last_set_bit(arr2, &top_bit2);

  if(arr1_zero && arr2_zero) return 0;
  if(arr1_zero) return -1;
  if(arr2_zero) return 1;

  bit_index_t top_bit2_offset = top_bit2 + pos;

  if(top_bit1 != top_bit2_offset) {
    return top_bit1 > top_bit2_offset ? 1 : -1;
  }

  word_addr_t i;
  word_t word1, word2;

  for(i = top_bit2 / WORD_SIZE; i > 0; i--)
  {
    word1 = _get_word(arr1, pos + i * WORD_SIZE);
    word2 = arr2->words[i];

    if(word1 > word2) return 1;
    if(word1 < word2) return -1;
  }

  word1 = _get_word(arr1, pos);
  word2 = arr2->words[0];

  if(word1 > word2) return 1;
  if(word1 < word2) return -1;

  // return 1 if arr1[0..pos] != 0, 0 otherwise

  // Whole words
  word_addr_t num_words = pos / WORD_SIZE;

  for(i = 0; i < num_words; i++)
  {
    if(arr1->words[i] > 0)
    {
      return 1;
    }
  }

  word_offset_t bits_remaining = pos - num_words * WORD_SIZE;

  if(arr1->words[num_words] & BIT_MASK(bits_remaining))
  {
    return 1;
  }

  return 0;
}


//
// Reverse -- coords may wrap around
//

// No bounds checking
// length cannot be zero
static void _reverse_region(BIT_ARRAY* bitarr,
                            bit_index_t start,
                            bit_index_t length)
{
  bit_index_t left = start;
  bit_index_t right = (start + length - WORD_SIZE) % bitarr->num_of_bits; 

  while(length >= 2 * WORD_SIZE)
  {
    // Swap entire words
    word_t left_word = _get_word_cyclic(bitarr, left);
    word_t right_word = _get_word_cyclic(bitarr, right);

    // reverse words individually
    left_word = _reverse_word(left_word);
    right_word = _reverse_word(right_word);

    // Swap
    _set_word_cyclic(bitarr, left, right_word);
    _set_word_cyclic(bitarr, right, left_word);

    // Update
    left = (left + WORD_SIZE) % bitarr->num_of_bits;
    right = (right < WORD_SIZE ? right + bitarr->num_of_bits : right) - WORD_SIZE;
    length -= 2 * WORD_SIZE;
  }

  word_t word, rev;

  if(length == 0)
  {
    return;
  }
  else if(length > WORD_SIZE)
  {
    // Words overlap
    word_t left_word = _get_word_cyclic(bitarr, left);
    word_t right_word = _get_word_cyclic(bitarr, right);

    rev = _reverse_word(left_word);
    right_word = _reverse_word(right_word);

    // fill left 64 bits with right word rev
    _set_word_cyclic(bitarr, left, right_word);

    // Now do remaining bits (length is between 1 and 64 bits)
    left += WORD_SIZE;
    length -= WORD_SIZE;

    word = _get_word_cyclic(bitarr, left);
  }
  else
  {
    word = _get_word_cyclic(bitarr, left);
    rev = _reverse_word(word);
  }

  rev >>= WORD_SIZE - length;
  word_t mask = BIT_MASK(length);

  word = MASK_MERGE(rev, word, mask);

  _set_word_cyclic(bitarr, left, word);
}

void _bit_array_reverse_region(const char *file, int line,
                               BIT_ARRAY* bitarr,
                               bit_index_t start, bit_index_t length)
{
  // Bounds checking
  _bounds_check_start(bitarr, start, file, line, "bit_array_reverse_region");
  _bounds_check_length(bitarr, length, file, line, "bit_array_reverse_region");

  if(length > 0)
  {
    _reverse_region(bitarr, start, length);
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

void bit_array_reverse(BIT_ARRAY* bitarr)
{
  if(bitarr->num_of_bits > 0)
  {
    _reverse_region(bitarr, 0, bitarr->num_of_bits);
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

//
// Shift left / right
//

// Shift towards MSB / higher index
void bit_array_shift_left(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)
{
  if(shift_dist >= bitarr->num_of_bits)
  {
    fill ? bit_array_set_all(bitarr) : bit_array_clear_all(bitarr);
    return;
  }
  else if(shift_dist == 0)
  {
    return;
  }

  FillAction action = fill ? FILL_REGION : ZERO_REGION;

  bit_index_t cpy_length = bitarr->num_of_bits - shift_dist;
  _array_copy(bitarr, shift_dist, bitarr, 0, cpy_length);

  _set_region(bitarr, 0, shift_dist, action);
}

// Shift towards LSB / lower index
void bit_array_shift_right(BIT_ARRAY* bitarr, bit_index_t shift_dist, char fill)
{
  if(shift_dist >= bitarr->num_of_bits)
  {
    fill ? bit_array_set_all(bitarr) : bit_array_clear_all(bitarr);
    return;
  }
  else if(shift_dist == 0)
  {
    return;
  }

  FillAction action = fill ? FILL_REGION : ZERO_REGION;

  bit_index_t cpy_length = bitarr->num_of_bits - shift_dist;
  bit_array_copy(bitarr, 0, bitarr, shift_dist, cpy_length);

  _set_region(bitarr, cpy_length, shift_dist, action);
}

//
// Cycle
//

// Cycle towards index 0
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

  bit_index_t len1 = cycle_dist;
  bit_index_t len2 = bitarr->num_of_bits - cycle_dist;

  _reverse_region(bitarr, 0, len1);
  _reverse_region(bitarr, len1, len2);
  bit_array_reverse(bitarr);
}

// Cycle away from index 0
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

  bit_index_t len1 = bitarr->num_of_bits - cycle_dist;
  bit_index_t len2 = cycle_dist;

  _reverse_region(bitarr, 0, len1);
  _reverse_region(bitarr, len1, len2);
  bit_array_reverse(bitarr);
}

//
// Next permutation
//

static word_t _next_permutation(word_t v) 
{
  // From http://graphics.stanford.edu/~seander/bithacks.html#NextBitPermutation
  word_t t = v | (v - 1); // t gets v's least significant 0 bits set to 1
  // Next set to 1 the most significant bit to change, 
  // set to 0 the least significant ones, and add the necessary 1 bits.
  return (t+1) | (((~t & (t+1)) - 1) >> (TRAILING_ZEROS(v) + 1));
}

// Get the next permutation of an array with a fixed size and given number of
// bits set.  Also known as next lexicographic permutation.
// Given a bit array find the next lexicographic orginisation of the bits
// Number of possible combinations given by (size choose bits_set) i.e. nCk
// 00011 -> 00101 -> 00110 -> 01001 -> 01010 ->
// 01100 -> 10001 -> 10010 -> 10100 -> 11000 -> 00011 (back to start)
void bit_array_next_permutation(BIT_ARRAY* bitarr)
{
  if(bitarr->num_of_bits == 0)
  {
    return;
  }

  word_addr_t w;

  char carry = 0;
  word_offset_t top_bits = boffset(bitarr->num_of_bits);

  for(w = 0; w < bitarr->num_of_words; w++)
  {
    word_t mask
      = (w < bitarr->num_of_words - 1 || top_bits == 0) ? WORD_MAX
                                                        : BIT_MASK(top_bits);

    if(bitarr->words[w] > 0 &&
       (bitarr->words[w] | (bitarr->words[w]-1)) == mask)
    {
      // Bits in this word cannot be moved forward
      carry = 1;
    }
    else if(carry)
    {
      // 0111 -> 1000, 1000 -> 1001
      word_t tmp = bitarr->words[w] + 1;

      // Count bits previously set
      bit_index_t bits_previously_set = POPCOUNT(bitarr->words[w]);

      // set new word
      bitarr->words[w] = tmp;

      // note: w in unsigned
      // Zero words while counting bits set
      while(w > 0)
      {
        bits_previously_set += POPCOUNT(bitarr->words[w-1]);
        bitarr->words[w-1] = 0;
        w--;
      }

      // Set bits at the beginning
      SET_REGION(bitarr, 0, bits_previously_set - POPCOUNT(tmp));

      carry = 0;
      break;
    }
    else if(bitarr->words[w] > 0)
    {
      bitarr->words[w] = _next_permutation(bitarr->words[w]);
      break;
    }
  }

  if(carry)
  {
    // Loop around
    bit_index_t num_bits_set = bit_array_num_bits_set(bitarr);
    bit_array_clear_all(bitarr);
    SET_REGION(bitarr, 0, num_bits_set);
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
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
void _bit_array_interleave(const char *file, int line,
                           BIT_ARRAY* dst, const BIT_ARRAY* src1,
                           const BIT_ARRAY* src2)
{
  if(dst == src1 || dst == src2)
  {
    fprintf(stderr, "%s:%i:bit_array_interleave(): dst cannot point to "
                    "src1 or src2\n", file, line);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }
  else if(src1->num_of_bits != src2->num_of_bits)
  {
    fprintf(stderr, "%s:%i:bit_array_interleave(): Behaviour undefined when"
                    "src1 length (%lu) != src2 length (%lu)", file, line,
            (unsigned long)src1->num_of_bits, (unsigned long)src2->num_of_bits);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }

  if(dst->num_of_bits < 2 * src1->num_of_bits)
  {
    bit_array_resize_critical(dst, 2 * src1->num_of_bits,
                              file, line, "bit_array_and");
  }

  word_addr_t i, j;

  for(i = 0, j = 0; i < src1->num_of_words; i++)
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
  VALIDATE_BIT_ARRAY(dst);
  #endif
}

//
// Random
//

// Set bits randomly with probability prob : 0 <= prob <= 1
void _bit_array_random(const char *file, int line, BIT_ARRAY* bitarr, float prob)
{
  if(bitarr->num_of_bits == 0)
  {
    return;
  }
  else if(prob > 1)
  {
    fprintf(stderr, "%s:%i:bit_array_random(): Behaviour undefined when "
                    "prob > 1 (%f)", file, line, prob);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }
  else if(prob == 1)
  {
    bit_array_set_all(bitarr);
    return;
  }

  // rand() generates number between 0 and RAND_MAX inclusive
  // therefore we want to check if rand() <= p
  long p = RAND_MAX * prob;

  _seed_rand();

  word_addr_t w;
  word_offset_t o;

  // Initialise to zero
  memset(bitarr->words, 0x0, bitarr->num_of_words * sizeof(word_t));

  for(w = 0; w < bitarr->num_of_words - 1; w++)
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
  w = bitarr->num_of_words - 1;

  for(o = 0; o < bits_in_last_word; o++)
  {
    if(rand() <= p)
    {
      bitarr->words[w] |= ((word_t)0x1 << o);
    }
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// Shuffle the bits in an array randomly
void bit_array_shuffle(BIT_ARRAY* bitarr)
{
  if(bitarr->num_of_bits == 0)
    return;

  _seed_rand();

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
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

//
// Arithmetic
//

// Returns 1 on sucess, 0 if value in array is too big
char bit_array_as_num(const BIT_ARRAY* bitarr, uint64_t* result)
{
  if(bitarr->num_of_bits == 0)
  {
    *result = 0;
    return 1;
  }

  word_addr_t w;

  for(w = bitarr->num_of_words-1; w > 0; w--)
  {
    if(bitarr->words[w] > 0)
    {
      return 0;
    }
  }

  *result = bitarr->words[0];
  return 1;
}


// 1 iff bitarr > value
// 0 iff bitarr == value
// -1 iff bitarr < value
int bit_array_compare_num(const BIT_ARRAY* bitarr, uint64_t value)
{
  if(bitarr->words[0] > value)
  {
    return 1;
  }

  word_addr_t i;
  for(i = 1; i < bitarr->num_of_words; i++)
  {
    if(bitarr->words[i] > 0)
    {
      return 1;
    }
  }

  // All words above words[0] are == 0
  // words[0] is not > value
  return (bitarr->words[0] == value ? 0 : -1);
}

// If value is zero, no change is made
void bit_array_add(BIT_ARRAY* bitarr, uint64_t value)
{
  if(value == 0)
  {
    return;
  }
  else if(bitarr->num_of_bits == 0)
  {
    bit_array_resize_critical(bitarr, WORD_SIZE - LEADING_ZEROS(value),
                              __FILE__, __LINE__, "bit_array_and");

    bitarr->words[0] = (word_t)value;
    return;
  }

  char carry = 0;
  word_addr_t i;

  for(i = 0; i < bitarr->num_of_words; i++)
  {
    if(WORD_MAX - bitarr->words[i] < value)
    {
      carry = 1;
      bitarr->words[i] += value;
    }
    else
    {
      // Carry is absorbed
      bitarr->words[i] += value;
      carry = 0;
      break;
    }
  }

  if(carry)
  {
    // Bit array full, need another bit after all words filled
    bit_array_resize_critical(bitarr, bitarr->num_of_words * WORD_SIZE + 1,
                              __FILE__, __LINE__, "bit_array_add");

    // Set top word to 1
    bitarr->words[bitarr->num_of_words-1] = 1;
  }
  else
  {
    word_t final_word = bitarr->words[bitarr->num_of_words-1];
    word_offset_t expected_bits = _bits_in_top_word(bitarr->num_of_bits);
    word_offset_t actual_bits = WORD_SIZE - LEADING_ZEROS(final_word);

    if(actual_bits > expected_bits)
    {
      // num_of_bits has increased -- num_of_words has not
      bitarr->num_of_bits += (actual_bits - expected_bits);
    }
  }
}

// If value is greater than bitarr, bitarr is not changed and 0 is returned
// Returns 1 on success, 0 if value > bitarr
char bit_array_minus(BIT_ARRAY* bitarr, uint64_t value)
{
  if(value == 0)
  {
    return 1;
  }
  else if(bitarr->words[0] >= value)
  {
    bitarr->words[0] -= value;
    return 1;
  }

  value -= bitarr->words[0];

  word_addr_t i;

  for(i = 1; i < bitarr->num_of_words; i++)
  {
    if(bitarr->words[i] > 0)
    {
      // deduct one
      bitarr->words[i]--;

      for(; i > 0; i--)
      {
        bitarr->words[i] = WORD_MAX;
      }

      // -1 since we've already deducted 1
      bitarr->words[0] = WORD_MAX - value - 1;

      return 1;
    }
  }

  // subtract value is greater than array
  return 0;
}

//
// Arithmetic between bit arrays
//

// src1, src2 and dst can all be the same BIT_ARRAY
static void _arithmetic(BIT_ARRAY* dst,
                        const BIT_ARRAY* src1,
                        const BIT_ARRAY* src2,
                        char subtract)
{
  word_addr_t max_words = MAX(src1->num_of_words, src2->num_of_words);

  // Adding: dst_words >= max(src1 words, src2 words)
  // Subtracting: dst_words is >= src1->num_of_words

  char carry = subtract ? 1 : 0;

  word_addr_t i;
  word_t word1, word2;
  
  for(i = 0; i < max_words; i++)
  {
    word1 = (i < src1->num_of_words ? src1->words[i] : 0);
    word2 = (i < src2->num_of_words ? src2->words[i] : 0);

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
      if(dst->num_of_words == max_words)
      {
        // Need to resize for the carry bit
        bit_array_resize_critical(dst, dst->num_of_bits+1,
                                  __FILE__, __LINE__, "_arithmetic");
      }

      dst->words[max_words] = (word_t)1;
    }
  }

  // Zero the rest of dst array
  for(i = max_words+carry; i < dst->num_of_words; i++)
  {
    dst->words[i] = (word_t)0;
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(dst);
  #endif
}

// src1, src2 and dst can all be the same BIT_ARRAY
// If dst is shorter than either of src1, src2, it is enlarged
void bit_array_sum(BIT_ARRAY* dst, const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
  bit_index_t max_src_bits = MAX(src1->num_of_bits, src2->num_of_bits);

  if(dst->num_of_bits < max_src_bits)
  {
    bit_array_resize_critical(dst, max_src_bits,
                              __FILE__, __LINE__, "bit_array_sum");
  }

  _arithmetic(dst, src1, src2, 0);
}

// dst = src1 - src2
// src1, src2 and dst can all be the same BIT_ARRAY
// If dst is shorter than src1, it will be extended to be as long as src1
// src1 must be greater than or equal to src2 (src1 >= src2)
void _bit_array_difference(const char *file, int line, BIT_ARRAY* dst,
                           const BIT_ARRAY* src1, const BIT_ARRAY* src2)
{
  // subtraction by method of complements:
  // a - b = a + ~b + 1 = src1 + ~src2 +1

  // src1 must be >= src2
  if(bit_array_cmp(src1, src2) < 0)
  {
    // Error
    fprintf(stderr, "%s:%i:bit_array_difference(): bit_array_substract "
                    "requires src1 >= src2\n", file, line);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }

  if(dst->num_of_bits < src1->num_of_bits)
  {
    bit_array_resize_critical(dst, src1->num_of_bits,
                              file, line, "bit_array_difference");
  }

  _arithmetic(dst, src1, src2, 1);
}


// Add `add` to `bitarr` at `pos`
// Bounds checking not needed as out of bounds is valid
void bit_array_add_word(BIT_ARRAY *bitarr, bit_index_t pos, uint64_t add)
{
  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif

  if(add == 0)
  {
    return;
  }
  else if(pos >= bitarr->num_of_bits)
  {
    // Resize and add!
    bit_index_t num_bits_required = pos + (WORD_SIZE - LEADING_ZEROS(add));
    
    bit_array_resize_critical(bitarr, num_bits_required,
                              __FILE__, __LINE__, "bit_array_add_word");

    _set_word(bitarr, pos, (word_t)add);
    return;
  }

  /*
  char str[1000];
  printf(" add_word: %s\n", bit_array_to_str_rev(bitarr, str));
  printf("     word: %s [pos: %i]\n", _word_to_str(add, str), (int)pos);
  */

  word_t w = _get_word(bitarr, pos);
  word_t sum = w + add;
  char carry = WORD_MAX - w < add;

  // Ensure array is big enough
  bit_index_t num_bits_required = pos + (carry ? WORD_SIZE + 1
                                               : (WORD_SIZE - LEADING_ZEROS(sum)));

  bit_array_ensure_size(bitarr, num_bits_required);

  _set_word(bitarr, pos, sum);
  pos += WORD_SIZE;

  if(carry)
  {
    word_offset_t offset = pos % WORD_SIZE;
    word_addr_t addr = bindex(pos);

    add = (word_t)0x1 << offset;
    carry = (WORD_MAX - bitarr->words[addr] < add);
    sum = bitarr->words[addr] + add;

    num_bits_required = addr * WORD_SIZE +
                        (carry ? WORD_SIZE + 1 : (WORD_SIZE - LEADING_ZEROS(sum)));

    bit_array_ensure_size(bitarr, num_bits_required);

    bitarr->words[addr++] = sum;

    if(carry)
    {
      while(addr < bitarr->num_of_words && bitarr->words[addr] == WORD_MAX)
      {
        bitarr->words[addr++] = 0;
      }

      if(addr == bitarr->num_of_words)
      {
        bit_array_resize_critical(bitarr, addr * WORD_SIZE + 1,
                              __FILE__, __LINE__, "bit_array_add_word");
      }
      else if(addr == bitarr->num_of_words-1 &&
              bitarr->words[addr] == BIT_MASK(_bits_in_top_word(bitarr->num_of_bits)))
      {
        bit_array_resize_critical(bitarr, bitarr->num_of_bits + 1,
                                  __FILE__, __LINE__, "bit_array_add_word");
      }

      bitarr->words[addr]++;
    }
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

// Add `add` to `bitarr` at `pos`
// Bounds checking not needed as out of bounds is valid
void _bit_array_add_words(const char *file, int line, BIT_ARRAY *bitarr,
                          bit_index_t pos, const BIT_ARRAY *add)
{
  if(bitarr == add)
  {
    // Error
    fprintf(stderr, "%s:%i:bit_array_add_words() bitarr and add cannot "
                    "point to the same bit array\n", file, line);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }

  bit_index_t add_top_bit_set;

  if(!bit_array_find_last_set_bit(add, &add_top_bit_set))
  {
    // No bits set in add
    return;
  }
  else if(pos >= bitarr->num_of_bits)
  {
    // Just resize and copy!
    bit_index_t num_bits_required = pos + add_top_bit_set + 1;
    
    bit_array_resize_critical(bitarr, num_bits_required,
                              file, line, "bit_array_add_words");

    _array_copy(bitarr, pos, add, 0, add->num_of_bits);
    return;
  }
  else if(pos == 0)
  {
    bit_array_sum(bitarr, bitarr, add);
    return;
  }

  /*
  char str[1000];
  printf(" add_words1: %s\n", bit_array_to_str_rev(bitarr, str));
  printf(" add_words2: %s\n", bit_array_to_str_rev(add, str));
  printf(" [pos: %i]\n", (int)pos);
  */

  bit_index_t num_bits_required = pos + add_top_bit_set + 1;
  bit_array_ensure_size(bitarr, num_bits_required);

  word_addr_t first_word = bindex(pos);
  word_offset_t first_offset = boffset(pos);

  word_t w = add->words[0] << first_offset;
  unsigned char carry = (WORD_MAX - bitarr->words[first_word] < w);

  bitarr->words[first_word] += w;

  word_addr_t i = first_word + 1;
  bit_index_t offset = WORD_SIZE - first_offset;

  for(; carry || offset <= add_top_bit_set; i++, offset += WORD_SIZE)
  {
    w = offset < add->num_of_bits ? _get_word(add, offset) : (word_t)0;

    if(i >= bitarr->num_of_words)
    {
      // Extend by a word
      bit_array_resize_critical(bitarr, (bit_index_t)(i+1)*WORD_SIZE+1,
                                file, line, "bit_array_add_words");
    }

    word_t prev = bitarr->words[i];

    bitarr->words[i] += w + carry;

    carry = (WORD_MAX - prev < w || (carry && prev + w == WORD_MAX)) ? 1 : 0;
  }

  word_offset_t top_bits
    = WORD_SIZE - LEADING_ZEROS(bitarr->words[bitarr->num_of_words-1]);

  bit_index_t min_bits = (bitarr->num_of_words-1)*WORD_SIZE + top_bits;

  if(bitarr->num_of_bits < min_bits)
  {
    // Extend within the last word
    bitarr->num_of_bits = min_bits;
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

char bit_array_minus_word(BIT_ARRAY* bitarr, bit_index_t pos, word_t minus)
{
  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif

  if(minus == 0)
  {
    return 1;
  }

  word_t w = _get_word(bitarr, pos);

  if(w >= minus)
  {
    _set_word(bitarr, pos, w - minus);

    #ifdef DEBUG
    VALIDATE_BIT_ARRAY(bitarr);
    #endif

    return 1;
  }

  minus -= w;

  bit_index_t offset;
  for(offset = pos + WORD_SIZE; offset < bitarr->num_of_bits; offset += WORD_SIZE)
  {
    w = _get_word(bitarr, offset);

    if(w > 0)
    {
      // deduct one
      _set_word(bitarr, offset, w - 1);

      SET_REGION(bitarr, pos, offset-pos);

      // -1 since we've already deducted 1
      minus--;

      _set_word(bitarr, pos, WORD_MAX - minus);

      #ifdef DEBUG
      VALIDATE_BIT_ARRAY(bitarr);
      #endif

      return 1;
    }
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif

  return 0;
}

char _bit_array_minus_words(const char *file, int line,
                            BIT_ARRAY* bitarr, bit_index_t pos, BIT_ARRAY* minus)
{
  if(bitarr == minus)
  {
    // Error
    fprintf(stderr, "%s:%i:bit_array_minus_words() bitarr and minus cannot "
                    "point to the same bit array\n", file, line);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }

  int cmp = bit_array_cmp_words(bitarr, pos, minus);

  if(cmp == 0)
  {
    bit_array_clear_all(bitarr);
    return 1;
  }
  else if(cmp < 0)
  {
    return 0;
  }

  bit_index_t bitarr_length = bitarr->num_of_bits;

  bit_index_t bitarr_top_bit_set;
  bit_array_find_last_set_bit(bitarr, &bitarr_top_bit_set);

  // subtraction by method of complements:
  // a - b = a + ~b + 1 = src1 + ~src2 +1

  bit_array_not(minus, minus);

  bit_array_add_words(bitarr, pos, minus);
  bit_array_add_word(bitarr, pos, (word_t)1);

  bit_array_minus_word(bitarr, pos+minus->num_of_bits, 1);
  bit_array_resize(bitarr, bitarr_length);

  bit_array_not(minus, minus);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif

  return 1;
}

void bit_array_multiply(BIT_ARRAY *bitarr, uint64_t multiplier)
{
  if(bitarr->num_of_bits == 0 || multiplier == 1)
  {
    return;
  }
  else if(multiplier == 0)
  {
    bit_array_clear_all(bitarr);
    return;
  }

  bit_index_t i;

  for(i = bitarr->num_of_bits; i > 0; i--)
  {
    if(bit_array_get(bitarr, i-1))
    {
      bit_array_clear(bitarr, i-1);
      bit_array_add_word(bitarr, i-1, multiplier);
    }
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif
}

void _bit_array_product(const char *file, int line,
                        BIT_ARRAY *dst, BIT_ARRAY *src1, BIT_ARRAY *src2)
{
  if(src1->num_of_bits == 0 || src2->num_of_bits == 0)
  {
    bit_array_clear_all(dst);
    return;
  }
  else if(dst == src1 && src1 == src2)
  {
    fprintf(stderr, "%s:%i:bit_array_product(): Cannot pass the same array "
                    "as dst, src1 AND src2\n", file, line);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }
  // Dev: multiplier == 1?

  BIT_ARRAY *read_arr, *add_arr;

  if(src1 == dst)
  {
    read_arr = src1;
    add_arr = src2;
  }
  else
  {
    read_arr = src2;
    add_arr = src1;
  }

  if(dst != src1 && dst != src2)
  {
    bit_array_clear_all(dst);
  }

  bit_index_t i;

  for(i = read_arr->num_of_bits; i > 0; i--)
  {
    if(bit_array_get(read_arr, i-1))
    {
      bit_array_clear(dst, i-1);
      bit_array_add_words(dst, i-1, add_arr);
    }
  }

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(dst);
  #endif
}

// bitarr = round_down(bitarr / divisor)
// rem = bitarr % divisor
void _bit_array_div(const char *file, int line,
                    BIT_ARRAY *bitarr, uint64_t divisor, uint64_t *rem)
{
  if(divisor == 0)
  {
    fprintf(stderr, "%s:%i:bit_array_div(): Cannot divide by zero\n", file, line);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }

  bit_index_t div_top_bit = 63 - LEADING_ZEROS(divisor);
  bit_index_t bitarr_top_bit;

  if(!bit_array_find_last_set_bit(bitarr, &bitarr_top_bit))
  {
    *rem = 0;
    return;
  }

  if(bitarr_top_bit < div_top_bit)
  {
    *rem = bitarr->words[0];
    bit_array_clear_all(bitarr);
    return;
  }

  // When div is shifted by offset, their top set bits are aligned
  bit_index_t offset = bitarr_top_bit - div_top_bit;

  uint64_t tmp = _get_word(bitarr, offset);
  _set_word(bitarr, offset, (word_t)0);

  // Carry if 1 if the top bit was set before left shift
  char carry = 0;

  // offset unsigned so break when offset == 0
  while(1)
  {
    if(carry)
    {
      // (carry:tmp) - divisor = (WORD_MAX+1+tmp)-divisor
      tmp = WORD_MAX - divisor + tmp + 1;
      bit_array_set(bitarr, offset);
    }
    else if(tmp >= divisor)
    {
      tmp -= divisor;
      bit_array_set(bitarr, offset);
    }
    else
    {
      bit_array_clear(bitarr, offset);
    }

    if(offset == 0)
      break;

    offset--;

    // Is the top bit set (that we're about to shift off)?
    carry = tmp & 0x8000000000000000;

    tmp <<= 1;
    tmp |= bit_array_get(bitarr, offset);
  }

  *rem = tmp;
}

// Results in:
//   quotient = dividend / divisor
//   dividend = dividend % divisor
// (dividend is used to return the remainder)
void _bit_array_divide(const char *file, int line, BIT_ARRAY *dividend,
                       BIT_ARRAY *quotient, BIT_ARRAY *divisor)
{
  if(bit_array_compare_num(divisor, 0) == 0)
  {
    fprintf(stderr, "%s:%i:bit_array_divide(): Cannot divide by zero\n",
            file, line);
    kill(getpid(), SIGABRT);
    // exit(EXIT_FAILURE);
  }

  bit_array_clear_all(quotient);

  int cmp = bit_array_cmp(dividend, divisor);

  if(cmp == 0)
  {
    bit_array_ensure_size(quotient, 1);
    bit_array_set(quotient, 0);
    bit_array_clear_all(dividend);
    return;
  }
  else if(cmp < 0)
  {
    // dividend is < divisor, quotient is zero -- done
    return;
  }

  // now we know: dividend > divisor, quotient is zero'd,
  //              dividend != 0, divisor != 0
  bit_index_t dividend_top_bit = 0, div_top_bit = 0;

  bit_array_find_last_set_bit(dividend, &dividend_top_bit);
  bit_array_find_last_set_bit(divisor, &div_top_bit);

  // When divisor is shifted by offset, their top set bits are aligned
  bit_index_t offset = dividend_top_bit - div_top_bit;

  // offset unsigned so break when offset == 0
  for(; ; offset--)
  {
    if(bit_array_cmp_words(dividend, offset, divisor) >= 0)
    {
      bit_array_minus_words(dividend, offset, divisor);
      bit_array_ensure_size(quotient, offset+1);
      bit_array_set(quotient, offset);
    }

    if(offset == 0)
      break;
  }
}

// Get bit array as decimal str (e.g. 0b1101 -> "13")
// len is the length of str char array -- will write at most len-1 chars
// returns the number of characters needed
// return is the same as strlen(str)
size_t bit_array_to_decimal(const BIT_ARRAY *bitarr, char *str, size_t len)
{
  size_t i = 0;

  if(bit_array_compare_num(bitarr, 0) == 0)
  {
    if(len >= 2)
    {
      *str = '0';
      *(str+1) = '\0';
    }

    return 1;
  }

  BIT_ARRAY *tmp = bit_array_clone(bitarr);
  uint64_t rem;

  str[len-1] = '\0';

  while(bit_array_compare_num(tmp, 0) != 0)
  {
    bit_array_div(tmp, 10, &rem);

    if(i < len-1)
    {
      str[len-2-i] = '0' + rem;
    }

    i++;
  }

  if(i < len-1)
  {
    // Moves null-terminator as well
    memmove(str, str+len-i-1, i+1);
  }

  bit_array_free(tmp);

  return i;
}

// Get bit array from decimal str (e.g. "13" -> 0b1101)
// Returns number of characters used
size_t bit_array_from_decimal(BIT_ARRAY *bitarr, const char* decimal)
{
  bit_array_clear_all(bitarr);
  size_t i = 0;

  if(decimal[0] == '\0' || decimal[0] < '0' || decimal[0] > '9')
  {
    return 0;
  }

  bit_array_add(bitarr, decimal[i] - '0');
  i++;

  while(decimal[i] != '\0' && decimal[i] >= '0' && decimal[i] <= '9')
  {
    bit_array_multiply(bitarr, 10);
    bit_array_add(bitarr, decimal[i] - '0');
    i++;
  }

  return i;
}

uint64_t bit_array_crc(const BIT_ARRAY *bitarr, uint64_t crc)
{
  // Top bit mask
  word_t tbm = (word_t)0x1 << (WORD_SIZE - 1);

  if(crc == 0 || crc & tbm)
  {
    fprintf(stderr, "Error: n must be 0 < n < 64\n");
    errno = EDOM;
    return 0;
  }

  // crcn is the number of bits set in crc
  word_offset_t crcn = TOP_BIT_SET(crc);

  bit_index_t top_bit;

  if(!bit_array_find_last_set_bit(bitarr, &top_bit))
  {
    // All bits zero
    return 0;
  }

  bit_index_t offset = MAX(top_bit, WORD_SIZE) - WORD_SIZE;

  word_t w = _get_word(bitarr, offset);
  word_t m = crcn == 64 ? crc : crc << (WORD_SIZE - crcn);

  if(w & tbm)
  {
    w ^= m;
  }

  while(offset > 0)
  {
    offset--;
    w = (w << 1) | bit_array_get(bitarr, offset);
    
    if(w & tbm)
    {
      w ^= m;
    }
  }

  word_offset_t i;
  for(i = 0; i < crcn - 1; i++)
  {
    w <<= 1;

    if(w & tbm)
    {
      w ^= m;
    }
  }

  if(crcn < WORD_SIZE)
  {
    w >>= (WORD_SIZE - crcn);
  }

  return w;
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

// Reads bit array from a file. bitarr is resized and filled.
// Returns 1 on success, 0 on failure
char bit_array_load(BIT_ARRAY* bitarr, FILE* f)
{
  bit_index_t items_read;

  // Read in number of bits
  bit_index_t num_bits = 0;
  items_read = fread(&num_bits, 8, 1, f);

  if(items_read != 1)
  {
    return 0;
  }

  // Resize
  bit_array_resize_critical(bitarr, num_bits,
                            __FILE__, __LINE__, "bit_array_load");

  // Have to calculate how many bytes are needed for the file
  // (Note: this may be different from num_of_words * sizeof(word_t))
  bit_index_t num_of_bytes_in_file = nbytes(bitarr->num_of_bits);

  items_read = fread(bitarr->words, 1, num_of_bytes_in_file, f);

  if(items_read != num_of_bytes_in_file)
  {
    return 0;
  }

  // Mask top word
  _mask_top_word(bitarr);

  #ifdef DEBUG
  VALIDATE_BIT_ARRAY(bitarr);
  #endif

  return 1;
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
// Generally useful functions
//

// Generalised 'binary to string' function
// Adds bits to the string in order of lsb to msb
// e.g. 0b11010 (26 in decimal) would come out as "01011"
char* bit_array_word2str(const void *ptr, size_t num_of_bits, char *str)
{
  const uint8_t* d = (const uint8_t*)ptr;

  size_t i;
  for(i = 0; i < num_of_bits; i++)
  {
    uint8_t bit = (d[i/8] >> (i % 8)) & 0x1;
    str[i] = bit ? '1' : '0';
  }
  str[num_of_bits] = '\0';
  return str;
}

char* bit_array_word2str_rev(const void *ptr, size_t num_of_bits, char *str)
{
  const uint8_t* d = (const uint8_t*)ptr;

  size_t i;
  for(i = 0; i < num_of_bits; i++)
  {
    uint8_t bit = (d[i/8] >> (i % 8)) & 0x1;
    str[num_of_bits-1-i] = bit ? '1' : '0';
  }
  str[num_of_bits] = '\0';
  return str;
}
