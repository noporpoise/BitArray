/*
 dev/bit_array_generate.c
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 author: Isaac Turner <turner.isaac@gmail.com>
 license: Public Domain, no warranty
 date: Dec 2013
*/


// 64 bit words
// Array length can be zero
// Unused top bits must be zero

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h> // memset
#include <stdint.h>

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

    printf(" 0x%02X%c", reverse(i), i == 255 ? '\n' : ',');
  }
  printf("};\n\n");
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

    if(a)
      m <<= 1;

    printf(" 0x%04X%c", m, i == 255 ? '\n' : ',');
  }
  printf("};\n\n");
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

    printf(i < 8 ? "},\n" : "}\n};\n\n");
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
  return 1;
}
