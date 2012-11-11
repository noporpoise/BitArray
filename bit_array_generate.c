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

uint8_t reverse(uint8_t b)
{
  uint8_t r = 0;
  int i;

  for(i = 0; i < 8; i++)
  {
    r <<= 1;
    r |= b & 0x1;
    b >>= 1;
  }

  return r;
}

// Print table of bytes -> byte reverse
void generate_reverse()
{
  int i;
  printf("static const word_t reverse_table[256] = \n{\n ");
  for(i = 0; i < 256; i++)
  {
    if(i % 8 == 0 && i > 0)
      printf("\n ");

    printf(" 0x%02X,", reverse(i));
  }
  printf("\n};\n\n");
}

uint16_t morton(uint8_t b)
{
  uint16_t r = 0;
  int i;
  for(i = 0; i < 8; i++)
  {
    r |= (b & 0x1) << (2*i);
    b >>= 1;
  }
  return r;
}

// a is either 0 or 1, and is how much to shift by
void generate_morton(char a)
{
  int i;
  printf("static const word_t morton_table%c[256] = \n{\n ", a ? '1' : '0');
  for(i = 0; i < 256; i++)
  {
    if(i % 8 == 0 && i > 0)
      printf("\n ");

    uint16_t m = morton(i);
    printf(" 0x%04X,", a ? m << 1 : m);
  }
  printf("\n};\n\n");
}

unsigned int next_permutation(unsigned int v) 
{
  unsigned int t = v | (v - 1); // t gets v's least significant 0 bits set to 1
  // Next set to 1 the most significant bit to change, 
  // set to 0 the least significant ones, and add the necessary 1 bits.
  //return (t + 1) | (((~t & -~t) - 1) >> (__builtin_ctz(v) + 1));
  return (t+1) | (((~t & (t+1)) - 1) >> (__builtin_ctz(v) + 1));
}

long factorial(int k)
{
  long r = k;
  for(k--; k > 1; k--)
  {
    r *= k;
  }
  return r;
}

void generate_shuffle()
{
  int i;

  printf("static const uint8_t shuffle_permutations[9] = {1");

  for(i = 1; i < 8; i++)
  {
    // nCk = n! / ((n-k)!k!)
    long combinations = factorial(8) / (factorial(8-i)*factorial(i));
    printf(", %li", combinations);
  }

  printf(", 1};\n");

  printf("static const uint8_t shuffle[9][70] = {\n");

  for(i = 0; i <= 8; i++)
  {
    printf(" // %i bits set\n", i);
    unsigned char init = i == 0 ? 0 : (0x1 << i) - 1;

    printf(" {0x%02X", (int)init);

    unsigned int c = next_permutation(init);
    int num;

    for(num = 1; num < 70; c = next_permutation(c), num++)
    {
      printf(num % 10 == 0 ? ",\n  " : ", ");
      printf("0x%02X", c <= 255 ? (int)c : 0);
    }

    printf(i < 8 ? "},\n" : "}};\n\n");
  }
}

int main()
{
  printf("// byte reverse look up table\n");
  generate_reverse();
  printf("// Morton table for interleaving bytes\n");
  generate_morton(0);
  printf("// Morton table for interleaving bytes, shifted left 1 bit\n");
  generate_morton(1);
  printf("// Tables for shuffling\n");
  generate_shuffle();
}
