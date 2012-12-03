/*
 bit_array_test.c
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 Adapted from: http://stackoverflow.com/a/2633584/431087
 author: Isaac Turner <turner.isaac@gmail.com>

 Copyright (c) 2011-2012, Isaac Turner
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
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "bit_array.h"

//
// Utility functions
//

void die(const char *fmt, ...)
__attribute__((format(printf, 1, 2)))
__attribute__((noreturn));

void warn(const char *fmt, ...)
__attribute__((format(printf, 1, 2)));

void die(const char *fmt, ...)
{
  fflush(stdout);

  // Print error
  fprintf(stderr, "Error: ");

  va_list argptr;
  va_start(argptr, fmt);
  vfprintf(stderr, fmt, argptr);
  va_end(argptr);

  // Check if we need to print a newline
  if(*(fmt + strlen(fmt) - 1) != '\n')
  {
    fprintf(stderr, "\n");
  }

  exit(EXIT_FAILURE);
}

void warn(const char *fmt, ...)
{
  fflush(stdout);

  // Print warning
  fprintf(stderr, "Warning: ");

  va_list argptr;
  va_start(argptr, fmt);
  vfprintf(stderr, fmt, argptr);
  va_end(argptr);

  // Check if we need to print a newline
  if(*(fmt + strlen(fmt) - 1) != '\n')
  {
    fprintf(stderr, "\n");
  }

  fflush(stderr);
}

//
// Testing per function
//

void test_arithmetic()
{
  printf("== testing arithmetic ==\n");

  char tmp[101];

  BIT_ARRAY* arr1 = bit_array_create(100);
  BIT_ARRAY* arr2 = bit_array_create(100);

  int i = 0;
  for(i = 0; i < 99; i+=3)
  {
    bit_array_set_bit(arr1, i);
    bit_array_set_bit(arr2, i);
    bit_array_set_bit(arr2, i+1);
  }

  printf("Init:\n");
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));

  printf("Increment: arr1++\n");
  bit_array_add(arr1, 1);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Decrement: arr1--\n");
  bit_array_subtract(arr1, 1);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Sum: arr1 = arr1 + arr2\n");
  bit_array_sum(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Difference: arr1 = arr1 - arr2\n");
  bit_array_difference(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Difference: arr1 = arr1 - arr1\n");
  bit_array_difference(arr1, arr1, arr1);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Sum: arr1 = arr1 + arr2\n");
  bit_array_sum(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Sum: arr1 = arr1 + arr2\n");
  bit_array_sum(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  bit_array_free(arr1);
  bit_array_free(arr2);

  printf("== End of testing arithmetic ==\n\n");
}

void _print_first_last_bit_set(BIT_ARRAY* arr)
{
  bit_index_t first_bit = 65, last_bit = 65;

  char bit_set = bit_array_find_first_set_bit(arr, &first_bit);
  bit_array_find_last_set_bit(arr, &last_bit);

  if(bit_set)
    printf("First bit set: %i, last bit set: %i\n", (int)first_bit, (int)last_bit);
  else
    printf("First bit set: -, last bit set: -\n");
}


void test_first_last_bit_set()
{
  printf("== testing first and last bit set ==\n");

  char tmp[101];
  BIT_ARRAY* arr = bit_array_create(100);

  printf("Initialise 100:\n");
  printf("arr: %s\n", bit_array_to_str(arr, tmp));

  printf("Set bits 0,5,24,64,80,99:\n");
  bit_array_set_bits(arr, 6, 0, 5, 24, 64, 80, 99);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));
  _print_first_last_bit_set(arr);

  printf("Clear bits 0,99:\n");
  bit_array_clear_bits(arr, 2, 0, 99);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));
  _print_first_last_bit_set(arr);

  printf("Clear bits 5,80:\n");
  bit_array_clear_bits(arr, 2, 5, 80);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));
  _print_first_last_bit_set(arr);

  printf("Clear bits 24,64:\n");
  bit_array_clear_bits(arr, 2, 24, 64);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));
  _print_first_last_bit_set(arr);

  printf("Set only bit 0:\n");
  bit_array_set_bit(arr, 0);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));
  _print_first_last_bit_set(arr);

  const int len = 9;
  bit_index_t set[len] = {0, 1, 31, 62, 63, 64, 65, 98, 99};
  int i;

  for(i = 0; i < len; i++)
  {
    bit_index_t pos = set[i];
    printf("Set only bit %i:\n", (int)pos);
    bit_array_clear_all(arr);
    bit_array_set_bit(arr, pos);
    printf("arr: %s\n", bit_array_to_str(arr, tmp));
    _print_first_last_bit_set(arr);
  }

  bit_array_free(arr);

  printf("== End of testing first and last bit set ==\n\n");
}

void test_parity()
{
  printf("== testing parity ==\n");

  char tmp[101];
  BIT_ARRAY* arr = bit_array_create(100);

  printf("Initialise 100:\n");
  printf("arr: %s\n", bit_array_to_str(arr, tmp));
  printf("  parity: %i\n", (int)bit_array_parity(arr));

  printf("Set bits 0,5,24,64,80,99:\n");
  bit_array_set_bits(arr, 6, 0, 5, 24, 64, 80, 99);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));
  printf("  parity: %i\n", (int)bit_array_parity(arr));

  printf("Clear bits 24:\n");
  bit_array_clear_bit(arr, 24);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));
  printf("  parity: %i\n", (int)bit_array_parity(arr));

  printf("Clear bits 99:\n");
  bit_array_clear_bit(arr, 99);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));
  printf("  parity: %i\n", (int)bit_array_parity(arr));

  printf("Clear bits 0:\n");
  bit_array_clear_bit(arr, 0);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));
  printf("  parity: %i\n", (int)bit_array_parity(arr));

  bit_array_free(arr);

  printf("== End of testing parity ==\n\n");
}

void test_interleave()
{
  printf("== testing interleave ==\n");

  char tmp[201];
  BIT_ARRAY* arr1 = bit_array_create(10);
  BIT_ARRAY* arr2 = bit_array_create(10);
  BIT_ARRAY* result = bit_array_create(0);

  printf("--\n");
  bit_array_set_all(arr1);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("Interleave:\n");
  bit_array_interleave(result, arr1, arr2);
  printf("result: %s\n", bit_array_to_str(result, tmp));

  printf("--\n");
  bit_array_clear_all(arr1);
  bit_array_set_all(arr2);
  bit_array_resize(result, 25);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("tmp: %s\n", bit_array_to_str(result, tmp));
  printf("Interleave:\n");
  bit_array_interleave(result, arr1, arr2);
  printf("result: %s\n", bit_array_to_str(result, tmp));

  printf("--\n");
  bit_array_set_all(arr1);
  bit_array_set_all(arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("Interleave:\n");
  bit_array_interleave(result, arr1, arr2);
  printf("result: %s\n", bit_array_to_str(result, tmp));

  printf("--\n");
  bit_array_clear_all(arr1);
  bit_array_clear_all(arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("Interleave:\n");
  bit_array_interleave(result, arr1, arr2);
  printf("result: %s\n", bit_array_to_str(result, tmp));

  printf("--\n");
  bit_array_resize(arr1, 100);
  bit_array_resize(arr2, 100);
  bit_array_clear_all(arr1);
  bit_array_set_all(arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("Interleave:\n");
  bit_array_interleave(result, arr1, arr2);
  printf("result: %s\n", bit_array_to_str(result, tmp));

  bit_array_free(arr1);
  bit_array_free(arr2);
  bit_array_free(result);

  printf("== End of testing interleave ==\n\n");
}

void test_compare()
{
  printf("== testing compare ==\n");

  char tmp[201];
  BIT_ARRAY* arr1 = bit_array_create(10);
  BIT_ARRAY* arr2 = bit_array_create(10);

  printf("--\n");
  bit_array_from_str(arr1, "011010100");
  bit_array_from_str(arr2, "001101010");
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("compare: %i\n", bit_array_cmp(arr1, arr2));

  printf("--\n");
  bit_array_from_str(arr1, "0");
  bit_array_from_str(arr2, "00");
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("compare: %i\n", bit_array_cmp(arr1, arr2));

  printf("--\n");
  bit_array_from_str(arr1, "");
  bit_array_from_str(arr2, "");
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("compare: %i\n", bit_array_cmp(arr1, arr2));

  bit_array_free(arr1);
  bit_array_free(arr2);

  printf("== End of testing compare ==\n\n");
}

void test_compare2()
{
  printf("== testing other endian compare ==\n");

  char tmp[201];
  BIT_ARRAY* arr1 = bit_array_create(10);
  BIT_ARRAY* arr2 = bit_array_create(10);

  printf("--\n");
  bit_array_from_str(arr1, "011010100");
  bit_array_from_str(arr2, "001101010");
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("compare: %i\n", bit_array_other_endian_cmp(arr1, arr2));

  printf("--\n");
  bit_array_from_str(arr1, "0");
  bit_array_from_str(arr2, "00");
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("compare: %i\n", bit_array_other_endian_cmp(arr1, arr2));

  printf("--\n");
  bit_array_from_str(arr1, "");
  bit_array_from_str(arr2, "");
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));
  printf("compare: %i\n", bit_array_other_endian_cmp(arr1, arr2));

  bit_array_free(arr1);
  bit_array_free(arr2);

  printf("== End of testing compare ==\n\n");
}

void test_hash()
{

  printf("== Testing hash ==\n");

  BIT_ARRAY* arr = bit_array_create(0);
  char str[200];

  printf("arr: %s\n", bit_array_to_str(arr, str));
  printf("hash: %lu\n", (unsigned long)bit_array_hash(arr, 0));
  printf("--\n");

  bit_array_resize(arr, 10);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  printf("hash: %lu\n", (unsigned long)bit_array_hash(arr, 0));
  printf("--\n");

  bit_array_set_bits(arr, 3, 5,7,9);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  printf("hash: %lu\n", (unsigned long)bit_array_hash(arr, 0));
  printf("--\n");

  bit_array_resize(arr, 80);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  printf("hash: %lu\n", (unsigned long)bit_array_hash(arr, 0));
  printf("--\n");

  bit_array_set_bits(arr, 3, 50,57,59);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  printf("hash: %lu\n", (unsigned long)bit_array_hash(arr, 0));
  printf("--\n");

  bit_array_resize(arr, 1000);
  printf("len: %lu\n", (unsigned long)bit_array_length(arr));
  printf("hash: %lu\n", (unsigned long)bit_array_hash(arr, 0));
  printf("--\n");

  bit_array_free(arr);

  printf("== End of testing hash ==\n\n");
}

void test_reverse()
{
  printf("== Testing reverse ==\n");

  BIT_ARRAY* arr = bit_array_create(0);
  char str[200];

  printf("Initialise length 0; reverse\n");
  bit_array_reverse(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("resize length 10; reverse\n");
  bit_array_resize(arr, 10);
  bit_array_reverse(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Set bits 2,3,9\n");
  bit_array_set_bits(arr, 3, 2,3,9);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  printf("Reverse\n");
  bit_array_reverse(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("resize length 80; reverse\n");
  bit_array_resize(arr, 80);
  bit_array_reverse(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  bit_array_free(arr);

  printf("== End of testing reverse ==\n\n");
}

void test_toggle()
{
  printf("== Testing toggle ==\n");

  BIT_ARRAY* arr = bit_array_create(0);
  char str[200];

  printf("Initialise length 0; toggle all\n");
  bit_array_toggle_all(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Initialise length 0; toggle region 0-0\n");
  bit_array_toggle_region(arr, 0, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("resize length 10; set bits 2,3,6,9\n");
  bit_array_resize(arr, 10);
  bit_array_set_bits(arr, 4, 2,3,6,9);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Toggle region 3-8 [start: 3, len:6]\n");
  bit_array_toggle_region(arr, 3, 6);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Toggle all\n");
  bit_array_toggle_all(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("resize length 80; set bits 0,20,50,62,64,70,75,79\n");
  bit_array_resize(arr, 80);
  bit_array_set_bits(arr, 8, 0,20,50,62,64,70,75,79);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Toggle region 20-74 [start: 25, len:50]\n");
  bit_array_toggle_region(arr, 25, 50);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Resize 100; reset\n");
  bit_array_resize(arr, 100);
  bit_array_clear_all(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Toggle bits 0,2,3\n");
  bit_array_toggle_bits(arr, 3, 0,2,3);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Toggle all\n");
  bit_array_toggle_all(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("toggle 0,100:\n");
  bit_array_toggle_region(arr,0,100);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("toggle 0,0:\n");
  bit_array_toggle_region(arr,0,0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("toggle 1,1:\n");
  bit_array_toggle_region(arr,1,1);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("toggle 3,1:\n");
  bit_array_toggle_region(arr,3,1);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("toggle 20,80:\n");
  bit_array_toggle_region(arr,20,80);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("toggle 0,64:\n");
  bit_array_toggle_region(arr,0,64);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("toggle 64,26:\n");
  bit_array_toggle_region(arr,64,36);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  bit_array_free(arr);

  printf("== End of testing toggle ==\n\n");
}


void test_shuffle()
{
  printf("== Testing shuffle ==\n");

  BIT_ARRAY* arr = bit_array_create(0);
  char str[200];

  printf("Initialise length 0; shuffle\n");
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                          (int)bit_array_num_bits_set(arr));

  printf("resize length 10; set bits 2,3,6,9\n");
  bit_array_resize(arr, 10);
  bit_array_set_bits(arr, 4, 2,3,6,9);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));

  printf("Shuffle\n");
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));

  printf("resize length 80; clear all; set bits 0-15 [start:0, len: 16]\n");
  bit_array_resize(arr, 80);
  bit_array_clear_all(arr);
  bit_array_set_region(arr, 0, 16);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));

  printf("Shuffle\n");
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));

  printf("Clear all; set bits 0-69 [start:0, len: 70]\n");
  bit_array_clear_all(arr);
  bit_array_set_region(arr, 0, 70);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));

  printf("Shuffle\n");
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));
  bit_array_shuffle(arr);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));

  bit_array_free(arr);

  printf("== End of testing shuffle ==\n\n");
}

// used in test_random
void _print_random_arr(BIT_ARRAY* arr, float *rates, int num_rates, char *tmp)
{
  int i, j;

  for(i = 0; i < num_rates; i++)
  {
    float rate = rates[i];

    printf("Random %f\n", rate);

    // 4 repetitions
    for(j = 0; j < 4; j++)
    {
      bit_array_random(arr, rate);
      printf("arr: %s [%i]\n", bit_array_to_str(arr, tmp),
                               (int)bit_array_num_bits_set(arr));
    }
  }
}

void test_random()
{
  printf("== Testing random ==\n");

  BIT_ARRAY* arr = bit_array_create(0);
  char str[200];

  const int num_rates = 4;
  float rates[] = {0.0, 0.1, 0.5, 1.0};

  printf("Initialise length 0; random\n");
  bit_array_random(arr, 0.1);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));

  printf("resize length 10; set bits 2,3,6,9\n");
  bit_array_resize(arr, 10);
  bit_array_set_bits(arr, 4, 2,3,6,9);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));

  _print_random_arr(arr, rates, num_rates, str);

  printf("resize length 80; clear all\n");
  bit_array_resize(arr, 80);
  bit_array_clear_all(arr);

  _print_random_arr(arr, rates, num_rates, str);

  bit_array_free(arr);

  printf("== End of testing random ==\n\n");
}

// Test cyclic shift / circular shift
void test_cycle()
{
  printf("== Testing circular shift ==\n");

  BIT_ARRAY* arr = bit_array_create(0);
  char str[200];

  printf("Initialise length 0; cycle left 3 cycle right 0\n");
  bit_array_cycle_left(arr, 3);
  bit_array_cycle_right(arr, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("resize length 10; set bits 2,3,6,9\n");
  bit_array_resize(arr, 10);
  bit_array_set_bits(arr, 4, 2,3,6,9);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));

  printf("cycle left 3\n");
  bit_array_cycle_left(arr, 3);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("cycle left 0\n");
  bit_array_cycle_left(arr, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("cycle right 3\n");
  bit_array_cycle_right(arr, 3);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("cycle right 0\n");
  bit_array_cycle_right(arr, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("cycle left 25\n");
  bit_array_cycle_left(arr, 25);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("cycle right 25\n");
  bit_array_cycle_right(arr, 25);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("resize length 80; set bits 10, 12, 28, 32, 39, 63, 64, 79\n");
  bit_array_resize(arr, 80);
  bit_array_set_bits(arr, 8, 10, 12, 28, 32, 39, 63, 64, 79);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("cycle right 65; then back left 65\n");
  bit_array_cycle_right(arr, 65);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  bit_array_cycle_left(arr, 65);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  int i;
  printf("Clear all; set even bits\n");
  bit_array_clear_all(arr);
  for(i = 0; i < 80; i += 2) bit_array_set_bit(arr, i);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  
  printf("cycle left 1; then right 1\n");
  bit_array_cycle_left(arr, 1);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  bit_array_cycle_right(arr, 1);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  bit_array_free(arr);

  printf("== End of testing circular shift ==\n\n");
}

// Test shift
void test_shift()
{
  printf("== Testing shift ==\n");

  BIT_ARRAY* arr = bit_array_create(0);
  char str[200];

  printf("Initialise length 0; shift left 3 shift fill 1\n");
  bit_array_shift_left(arr, 3, 1);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Shift right 0 shift fill 1\n");
  bit_array_shift_right(arr, 0, 1);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("resize length 10; set bits 2,3,6,9\n");
  bit_array_resize(arr, 10);
  bit_array_set_bits(arr, 4, 2,3,6,9);
  printf("arr: %s [%i]\n", bit_array_to_str(arr, str),
                           (int)bit_array_num_bits_set(arr));

  printf("shift left 3, fill 0\n");
  bit_array_shift_left(arr, 3, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("shift left 0, fill 0\n");
  bit_array_shift_left(arr, 0, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("shift right 3, fill 1\n");
  bit_array_shift_right(arr, 3, 1);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("shift right 0, fill 1\n");
  bit_array_shift_right(arr, 0, 1);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("shift left 25, fill 0\n");
  bit_array_shift_left(arr, 25, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("shift right 25, fill 0\n");
  bit_array_shift_right(arr, 25, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("resize length 80; set bits 10, 12, 28, 32, 39, 63, 64, 79\n");
  bit_array_resize(arr, 80);
  bit_array_set_bits(arr, 8, 10, 12, 28, 32, 39, 63, 64, 79);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("shift right 65; then back left 65 [fill 0]\n");
  bit_array_shift_right(arr, 65, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  bit_array_shift_left(arr, 65, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  int i;
  printf("Clear all; set even bits\n");
  bit_array_clear_all(arr);
  for(i = 0; i < 80; i += 2) bit_array_set_bit(arr, i);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  
  printf("shift left 1; then right 1 [fill 1]\n");
  bit_array_shift_left(arr, 1, 1);
  printf("arr: %s\n", bit_array_to_str(arr, str));
  bit_array_shift_right(arr, 1, 1);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  bit_array_free(arr);

  printf("== End of testing shift ==\n\n");
}

void test_next_permutation()
{
  printf("== Testing next permutation ==\n");

  BIT_ARRAY* arr = bit_array_create(0);
  char str[1000];
  int i;

  printf("Initialise length 0\n");
  printf("arr: %s\n", bit_array_to_str(arr, str));
  printf("Permute\n");
  bit_array_next_permutation(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Resize length 10\n");
  bit_array_resize(arr, 10);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Permute x 3\n");
  for(i = 0; i < 3; i++)
  {
    bit_array_next_permutation(arr);
    printf("%3i) arr: %s\n", i, bit_array_to_str(arr, str));
  }

  printf("Set 1 bit\n");
  bit_array_set_bit(arr, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Permute x 12\n");
  for(i = 0; i < 12; i++)
  {
    bit_array_next_permutation(arr);
    printf("%3i) arr: %s\n", i, bit_array_to_str(arr, str));
  }

  printf("Clear all; set bits 0,1\n");
  bit_array_clear_all(arr);
  bit_array_set_bit(arr, 0);
  bit_array_set_bit(arr, 1);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Permute x 50\n");
  for(i = 0; i < 50; i++)
  {
    bit_array_next_permutation(arr);
    printf("%3i) arr: %s\n", i, bit_array_to_str(arr, str));
  }

  printf("Resize length 80; set bit 0\n");
  bit_array_resize(arr, 80);
  bit_array_clear_all(arr);
  bit_array_set_bit(arr, 0);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Permute x 100\n");
  for(i = 0; i < 100; i++)
  {
    bit_array_next_permutation(arr);
    printf("%3i) arr: %s\n", i, bit_array_to_str(arr, str));
  }

  printf("Resize to 180; set bits 10-149\n");
  bit_array_resize(arr, 180);
  bit_array_clear_all(arr);
  bit_array_set_region(arr, 10, 150);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Permute\n");
  bit_array_next_permutation(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Clear all; set bits 10-179\n");
  bit_array_clear_all(arr);
  bit_array_set_region(arr, 10, 170);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Permute\n");
  bit_array_next_permutation(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Clear all; set bit 63\n");
  bit_array_clear_all(arr);
  bit_array_set_bit(arr, 63);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Permute\n");
  bit_array_next_permutation(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Clear all; set bits 63, 64\n");
  bit_array_clear_all(arr);
  bit_array_set_bit(arr, 63);
  bit_array_set_bit(arr, 64);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  printf("Permute\n");
  bit_array_next_permutation(arr);
  printf("arr: %s\n", bit_array_to_str(arr, str));

  bit_array_free(arr);

  printf("== End of testing next permutation ==\n\n");
}

void _print_hamming_values(BIT_ARRAY* arr1, BIT_ARRAY* arr2)
{
  char str[200];

  printf("arr1: %s [%i]\n",
         bit_array_to_str(arr1, str), (int)bit_array_num_bits_set(arr1));
  printf("arr2: %s [%i]\n",
         bit_array_to_str(arr2, str), (int)bit_array_num_bits_set(arr2));
  printf(" hamming distance: %i\n", (int)bit_array_hamming_distance(arr1, arr2));
}

void test_hamming_weight()
{
  printf("== Testing hamming weight ==\n");

  BIT_ARRAY* arr1 = bit_array_create(0);
  BIT_ARRAY* arr2 = bit_array_create(0);

  _print_hamming_values(arr1, arr2);
  bit_array_resize(arr1, 10);
  _print_hamming_values(arr1, arr2);
  bit_array_set_bits(arr1, 3, 0, 2, 7);
  _print_hamming_values(arr1, arr2);
  bit_array_resize(arr2, 10);
  _print_hamming_values(arr1, arr2);
  bit_array_set_bits(arr2, 3, 0, 2, 7);
  _print_hamming_values(arr1, arr2);
  bit_array_resize(arr1, 80);
  bit_array_clear_bit(arr1, 2);
  bit_array_set_region(arr1, 50, 20);
  _print_hamming_values(arr1, arr2);
  _print_hamming_values(arr2, arr1);

  bit_array_free(arr1);
  bit_array_free(arr2);

  printf("== End of testing hamming weight ==\n\n");
}

void _test_save_load(BIT_ARRAY *arr1, BIT_ARRAY *arr2)
{
  FILE *f = fopen("test.bitarr.bin", "w");

  if(f == NULL)
  {
    die("Couldn't open file to write: 'test.bitarr.bin'");
  }

  char *tmp = (char*)malloc(sizeof(char) * (bit_array_length(arr1)+1));
  printf("%s\n", bit_array_to_str(arr1, tmp));

  bit_array_save(arr1, f);
  fclose(f);

  f = fopen("test.bitarr.bin", "r");

  if(f == NULL)
  {
    die("Couldn't open file to read: 'test.bitarr.bin'");
  }

  if(!bit_array_load(arr2, f))
  {
    warn("Load returned warning");
  }

  fclose(f);

  if(bit_array_cmp(arr1, arr2) != 0)
  {
    printf("1>%s\n", bit_array_to_str(arr1, tmp));
    printf("2>%s\n", bit_array_to_str(arr2, tmp));
    die("Failed save/load");
  }

  free(tmp);
}

void test_save_load()
{
  printf("== Testing save / load ==\n");

  BIT_ARRAY* arr1 = bit_array_create(0);
  BIT_ARRAY* arr2 = bit_array_create(0);

  printf("1) Empty:\n");
  _test_save_load(arr1, arr2);

  printf("2) ten 0s:\n");
  bit_array_resize(arr1, 10);
  _test_save_load(arr1, arr2);

  printf("3) thousand 1s:\n");
  bit_array_resize(arr1, 1000);
  bit_array_set_all(arr1);
  _test_save_load(arr1, arr2);

  printf("4) 1100111010:\n");
  bit_array_resize(arr1, 10);
  bit_array_clear_bits(arr1, 4, 2,3,7,9);
  _test_save_load(arr1, arr2);

  bit_array_free(arr1);
  bit_array_free(arr2);

  printf("== End of testing save / load ==\n\n");
}

//
// Aggregate testing
//

void test_zero_length_arrays()
{
  printf("== Testing zero length arrays ==\n");

  BIT_ARRAY* arr1 = bit_array_create(0);
  BIT_ARRAY* arr2 = bit_array_create(10);

  char tmp[101];

  printf("Initial arr1[length:0] arr2[length:10]\n");
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));

  printf("--\nResize arr2 to 0\n");
  bit_array_resize(arr2, 0);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));

  printf("--\nAnd (arr1, arr2)\n");
  bit_array_and(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));

  printf("--\nNot (arr1)\n");
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("--\nAnd (arr1, arr2)\n");
  bit_array_and(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));
  printf("arr2: %s\n", bit_array_to_str(arr2, tmp));

  bit_array_free(arr1);
  bit_array_free(arr2);

  printf("== End of testing zero length arrays ==\n\n");
}

void test_multiple_actions()
{
  printf("== testing all functions ==\n");

  printf("Create bit array 100 bits long\n");

  BIT_ARRAY* bitarr = bit_array_create(100);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Set bit 2\n");
  bit_array_set_bit(bitarr, 2);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Set bit 5\n");
  bit_array_set_bit(bitarr, 5);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Set bit 99\n");
  bit_array_set_bit(bitarr, 99);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Set bit 0\n");
  bit_array_set_bit(bitarr, 0);
  bit_array_print(bitarr, stdout);
  printf("\n");

  /* Test clone */
  printf("Clone\n");
  BIT_ARRAY* clone = bit_array_clone(bitarr);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("  Clear bit 0\n");
  bit_array_clear_bit(clone, 0);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("  Set bit 21\n");
  bit_array_set_bit(clone, 21);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("  Set bit 63\n");
  bit_array_set_bit(clone, 63);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("  End of clone\n");
  bit_array_free(clone);

  /* End of clone */
  
  printf("Fill with zeros\n");
  bit_array_clear_all(bitarr);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Fill with ones\n");
  bit_array_set_all(bitarr);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Clear bit 1\n");
  bit_array_clear_bit(bitarr, 1);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Clear bits 0-39\n");
  bit_array_clear_region(bitarr, 0, 40);
  bit_array_print(bitarr, stdout);
  printf("\n");

  bit_index_t first_set_bit;
  if(bit_array_find_first_set_bit(bitarr, &first_set_bit))
  {
    printf("first set bit: %i\n", (int)first_set_bit);
  }
  
  printf("Set bits 1-1\n");
  bit_array_set_region(bitarr, 1, 1);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Set bits (3,0)\n");
  bit_array_set_region(bitarr, 3, 0);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Clear bits 50-59\n");
  bit_array_clear_region(bitarr, 50, 10);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Negate\n");
  bit_array_not(bitarr, bitarr);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Sort bits\n");
  bit_array_sort_bits(bitarr);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Clear bits 1-51\n");
  bit_array_clear_region(bitarr, 1, 51);
  bit_array_print(bitarr, stdout);
  printf("\n");

  if(bit_array_find_first_set_bit(bitarr, &first_set_bit))
  {
    printf("first set bit: %i\n", (int)first_set_bit);
  }
  
  printf("Set bits 1-51\n");
  bit_array_set_region(bitarr, 1, 51);
  bit_array_print(bitarr, stdout);
  printf("\n");

  if(bit_array_find_first_set_bit(bitarr, &first_set_bit))
  {
    printf("first set bit: %i\n", (int)first_set_bit);
  }
  
  printf("Clear bit 98\n");
  bit_array_clear_bit(bitarr, 98);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Clear bit 99\n");
  bit_array_clear_bit(bitarr, 99);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Resize to 40 bits\n");
  bit_array_resize(bitarr, 40);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Resize to 100 bits\n");
  bit_array_resize(bitarr, 100);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Fill with ones\n");
  bit_array_set_all(bitarr);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Resize to 64 bits\n");
  bit_array_resize(bitarr, 64);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Resize to 100 bits\n");
  bit_array_resize(bitarr, 100);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Set clear 0,10,55:\n");
  bit_array_clear_bits(bitarr, 3, 0, 10, 55);
  printf("Set bits 67,69,70:\n");
  bit_array_set_bits(bitarr, 3, 67, 69, 70);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Bits set: %i\n", (int)bit_array_num_bits_set(bitarr));

  printf("Shift left 10 bits\n");
  bit_array_shift_left(bitarr, 10, 0);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Shift right 10 bits [fill with 1s]\n");
  bit_array_shift_right(bitarr, 10, 1);
  bit_array_print(bitarr, stdout);
  printf("\n");
  
  printf("Shift left 10 bits [fill with 1s]\n");
  bit_array_shift_left(bitarr, 10, 1);
  bit_array_print(bitarr, stdout);
  printf("\n");

  // Test cycle bits

  printf("Cycle right 10 bits\n");
  bit_array_cycle_right(bitarr, 10);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Cycle right 80 bits\n");
  bit_array_cycle_right(bitarr, 80);
  bit_array_print(bitarr, stdout);
  printf("\n");

  bit_array_free(bitarr);

  printf("\nNew bit array:\n");
  bitarr = bit_array_create(210); // = ~3.28 words
  bit_array_set_region(bitarr, 0, 100);
  bit_array_set_bits(bitarr, 5, 200, 202, 204, 206, 208);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Cycle right 80 bits\n");
  bit_array_cycle_right(bitarr, 80);
  bit_array_print(bitarr, stdout);
  printf("\n");

  // Test reverse

  printf("Reverse region [10,90]:\n");
  bit_array_reverse_region(bitarr, 10, 90);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Reverse region [10,90]:\n");
  bit_array_reverse_region(bitarr, 10, 90);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Reverse region [90,10]:\n");
  bit_array_reverse_region(bitarr, 90, 10);
  bit_array_print(bitarr, stdout);
  printf("\n");

  printf("Reverse region [90,10]:\n");
  bit_array_reverse_region(bitarr, 90, 10);
  bit_array_print(bitarr, stdout);
  printf("\n");

  // Test write/read file
  char filename[] = "/tmp/bitarrtest.bits";
  FILE* f;

  printf("Saving bitarray in %s\n", filename);
  f = fopen(filename, "w");
  bit_array_save(bitarr, f);
  fclose(f);

  bit_array_print(bitarr, stdout);
  printf("\n");

  // Deconstruct bit array
  bit_array_free(bitarr);

  printf("Loading bitarray in %s\n", filename);
  f = fopen(filename, "r");
  bit_array_load(bitarr, f);
  fclose(f);

  bit_array_print(bitarr, stdout);
  printf("\n");

  // Deconstruct bit array
  bit_array_free(bitarr);

  printf("== End of testing all functions ==\n\n");
}

// bit_array_from_str(), bit_array_from_substr()
// bit_array_to_str(), bit_array_to_substr()
// bit_array_print(), bit_array_print_substr()
void test_string_functions()
{
  printf("== Testing string functions ==\n\n");

  char data[]
    = "10110010100100011101000110011110101010111001010100110111000110000110110";
  char tmp[1000];

  printf("  %s\n", data);

  BIT_ARRAY *arr = bit_array_create(0);

  printf("  bit_array_to_substr()\n");
  bit_array_from_str(arr, data);
  printf("  %s\n", bit_array_to_str(arr, tmp));
  bit_array_to_substr(arr, 4, strlen(data)-4, tmp, '1', '0', 1);
  printf("      %s\n", tmp);
  bit_array_to_substr(arr, 0, strlen(data)-4, tmp, '1', '0', 1);
  printf("  %s\n", tmp);

  printf("  bit_array_from_substr()\n");
  bit_array_resize(arr, 0);
  bit_array_from_substr(arr, 10, data+10, strlen(data)-10, "1", "0", 1);
  printf("  %s\n", bit_array_to_str(arr, tmp));
  bit_array_resize(arr, 0);
  bit_array_from_substr(arr, 0, data, strlen(data)-10, "1", "0", 1);
  printf("  %s\n", bit_array_to_str(arr, tmp));

  printf("  bit_array_print_substr()\n");
  bit_array_from_str(arr, data);
  printf("            ");
  bit_array_print_substr(arr, 10, strlen(data)-10, stdout, '1', '0', 1);
  printf("\n  ");
  bit_array_print_substr(arr, 0, strlen(data)-10, stdout, '1', '0', 1);
  printf("\n\n");

  printf("== End of testing string functions ==\n\n");
}

void _test_hex_functions(BIT_ARRAY *arr, const char* hex, int offset, char upper)
{
  char tmp_buf[1000];
  char *tmp = tmp_buf;

  if(hex[0] == '0' && hex[1] == 'x')
  {
    tmp_buf[0] = '0';
    tmp_buf[1] = 'x';
    tmp = tmp_buf + 2;
  }

  bit_array_resize(arr, 0);
  printf(" Hex: %s; offset: %i\n", hex, offset);
  bit_array_from_hex(arr, offset, hex, strlen(hex));
  printf(" bit_array_to_hex: %s\n", bit_array_to_str(arr, tmp));

  bit_array_to_hex(arr, offset, bit_array_length(arr)-offset, tmp, upper);
  printf(" bit_array_to_hex: 0x%s\n", tmp);

  printf(" bit_array_print_hex: 0x");
  bit_array_print_hex(arr, offset, bit_array_length(arr)-offset, stdout, upper);
  printf("\n");

  if(strcasecmp(hex, tmp_buf) != 0)
  {
    printf("Warning: mismatching hex '%s' vs '%s'\n", tmp_buf, hex);
  }
}

// bit_array_from_hex(), bit_array_to_hex(), bit_array_print_hex()
void test_hex_functions()
{
  printf("== Testing hex functions ==\n");

  BIT_ARRAY *arr = bit_array_create(0);

  bit_array_resize(arr, 0);
  _test_hex_functions(arr, "123456789ABcDeF0", 0, 0);
  _test_hex_functions(arr, "123456789ABcDeF0", 0, 1); // uppercase
  _test_hex_functions(arr, "0x123456789ABcDeF0", 0, 0);
  _test_hex_functions(arr, "0x123456789ABcDeF0", 0, 1); // uppercase
  _test_hex_functions(arr, "0x123456789Agadsfasdf", 0, 0);
  _test_hex_functions(arr, "0x123456789ABcDeF0", 1, 0);
  _test_hex_functions(arr, "0x123456789ABcDeF0", 40, 0);

  printf("== End of testing hex functions ==\n\n");
}

void _test_nums(BIT_ARRAY *arr, char *tmp, unsigned long true_value)
{
  bit_array_add(arr, true_value);
  printf(" bit_array_add(0+%lu): %s\n", true_value, bit_array_to_str(arr, tmp));

  unsigned long num;
  if(!bit_array_as_num(arr, &num))
  {
    printf("Error:%s%i: failed as_num\n", __FILE__, __LINE__);
  }

  printf(" bit_array_as_num(%lu): %lu\n", true_value, num);

  if(num != true_value)
  {
    printf("Error:%s%i: num mismatch\n", __FILE__, __LINE__);
  }

  printf("compare to %lu: %i\n", true_value-1, bit_array_compare_num(arr, true_value-1));
  printf("compare to %lu: %i\n", true_value, bit_array_compare_num(arr, true_value));
  printf("compare to %lu: %i\n", true_value+1, bit_array_compare_num(arr, true_value+1));
}

// Test bit_array_as_num(), bit_array_compare_num()
void test_nums()
{
  printf("== Testing num functions ==\n\n");

  BIT_ARRAY *arr = bit_array_create(0);
  char tmp[100];

  bit_array_resize(arr, 0);
  _test_nums(arr, tmp, 0);
  bit_array_resize(arr, 0);
  _test_nums(arr, tmp, 10);
  bit_array_resize(arr, 0);
  _test_nums(arr, tmp, 100000000000);
  bit_array_resize(arr, 0);
  _test_nums(arr, tmp, ULONG_MAX);
  _test_nums(arr, tmp, ULONG_MAX);

  bit_array_free(arr);

  printf("== End of testing num functions ==\n\n");
}

void _test_add_word_small(unsigned long init, unsigned long add, int offset)
{
  BIT_ARRAY *arr1 = bit_array_create(0);
  BIT_ARRAY *arr2 = bit_array_create(0);
  char tmp[100];

  unsigned long a, b, result;

  printf(" Add %lu, %lu [offset: %i]\n", init, add, offset);
  bit_array_add(arr1, init);
  printf("  init: %s\n", bit_array_to_str(arr1, tmp));
  bit_array_as_num(arr1, &a);

  bit_array_add(arr2, add);
  if(offset > 0)
  {
    bit_array_resize(arr2, bit_array_length(arr2)+offset);
    bit_array_shift_left(arr2, offset, 0);
  }
  printf("  add : %s\n", bit_array_to_str(arr2, tmp));
  bit_array_as_num(arr2, &b);

  if(b != add << offset)
  {
    printf("Warning: b != add << offset\n");
  }

  bit_array_add_word(arr1, offset, add);
  printf("  sum : %s\n", bit_array_to_str(arr1, tmp));
  bit_array_as_num(arr1, &result);

  printf("  %lu + %lu = %lu [%s]\n", a, b, result, (a+b == result ? "Pass" : "Fail"));

  bit_array_free(arr1);
  bit_array_free(arr2);
}

void test_add_word()
{
  printf("== Testing add word ==\n\n");

  _test_add_word_small(0, 3, 0);
  _test_add_word_small(0, 3, 1);
  _test_add_word_small(3, 3, 0);
  _test_add_word_small(3, 3, 1);
  _test_add_word_small(0, 0, 3);

  //   0111010 [58]
  // + 1010000 [5 << 4]
  _test_add_word_small(58, 5, 4);

  _test_add_word_small(ULONG_MAX, 1, 4);

  printf("== End of testing add word ==\n\n");
}

void _test_add_words(unsigned long a, unsigned long b, int offset)
{
  BIT_ARRAY *arr1 = bit_array_create(0);
  BIT_ARRAY *arr2 = bit_array_create(0);
  char tmp[500];

  bit_array_add(arr1, a);
  bit_array_add(arr2, b);

  printf(" Add %lu + %lu [offset: %i]\n", a, b, offset);
  printf(" a: %s\n", bit_array_to_str(arr1, tmp));
  printf(" b: %s >> %i\n", bit_array_to_str(arr2, tmp), offset);

  bit_array_add_words(arr1, offset, arr2);

  printf("  = %s\n", bit_array_to_str(arr1, tmp));

  bit_array_free(arr1);
  bit_array_free(arr2);
}

void test_add_words()
{
  printf("== Testing add words ==\n\n");

  _test_add_words(3, 12, 0);
  _test_add_words(12, 3, 0);
  _test_add_words(3, 12, 1);
  _test_add_words(12, 3, 1);
  _test_add_words(1, 1, 1);
  _test_add_words(1, 1, 0);
  _test_add_words(1, 1, 10);
  _test_add_words(1, 1, 100);

  printf("TEST 2:\n");

  char tmp[1000];

  BIT_ARRAY *arr1 = bit_array_create(0);
  BIT_ARRAY *arr2 = bit_array_create(0);

  bit_array_resize(arr1, 200);
  bit_array_set_region(arr1, 0, 150);
  bit_array_add(arr2, 12);

  printf("a: ");
  bit_array_print_substr(arr1, 0, bit_array_length(arr1), stdout, '1', '0', 0);
  printf("\n");
  printf("b: ");
  bit_array_print_substr(arr2, 0, bit_array_length(arr2), stdout, '1', '0', 0);
  printf("\n");

  int offset = 0;
  for(offset = 0; offset < 200; offset += 20)
  {
    bit_array_clear_all(arr1);
    bit_array_set_region(arr1, 0, 150);

    bit_array_add_words(arr1, offset, arr2);

    printf(" %3i:%s\n", offset, bit_array_to_str(arr1, tmp));
  }

  printf("== End of testing add words ==\n\n");
}

void _test_multiply_small(unsigned long a, unsigned long b)
{
  BIT_ARRAY *arr = bit_array_create(0);
  unsigned long product;

  bit_array_add(arr, a);
  bit_array_multiply(arr, b);
  bit_array_as_num(arr, &product);

  printf("%lu * %lu = %lu [%s]\n", a, b, product,
         (a*b == product ? "Pass" : "Fail"));

  bit_array_free(arr);
}

void test_multiply()
{
  printf("== Testing multiply ==\n");

  _test_multiply_small(2,4);
  _test_multiply_small(4,2);
  _test_multiply_small(3,6);
  _test_multiply_small(9,256);
  _test_multiply_small(10,100);
  _test_multiply_small(0,0);
  _test_multiply_small(10,0);
  _test_multiply_small(0,10);

  BIT_ARRAY *arr = bit_array_create(0);
  char tmp[1000];
  bit_array_add(arr, ULONG_MAX);
  bit_array_shift_left(arr, 2, 0);
  bit_array_to_substr(arr, 0, bit_array_length(arr), tmp, '1', '0', 0);
  printf("  data: 0b%s\n", tmp);
  bit_array_multiply(arr, 6);
  bit_array_to_substr(arr, 0, bit_array_length(arr), tmp, '1', '0', 0);
  printf("   x 6: 0b%s\n", tmp);
  bit_array_free(arr);


  printf("== End of testing multiply ==\n\n");
}

void _test_product(unsigned long a, unsigned long b)
{
  BIT_ARRAY *arr1 = bit_array_create(0);
  BIT_ARRAY *arr2 = bit_array_create(0);
  char tmp[1000];

  int i;
  for(i = 0; i < 2; i++)
  {
    bit_array_resize(arr1, 0);
    bit_array_resize(arr2, 0);
    bit_array_add(arr1, a);
    bit_array_add(arr2, b);

    printf(" a: %s\n", bit_array_to_str(arr1, tmp));
    printf(" b: %s\n", bit_array_to_str(arr2, tmp));

    BIT_ARRAY *result = i == 0 ? arr1 : arr2;

    bit_array_product(result, arr1, arr2);

    printf("  = %s\n", bit_array_to_str(result, tmp));

    unsigned long c;
    if(bit_array_as_num(result, &c))
    {
      printf("  %lu * %lu = %lu [%s]\n", a, b, c, (a*b == c ? "Pass" : "Fail"));
    }
    else
    {
      printf("  %lu * %lu overflowed\n", a, b);
    }
  }

  bit_array_free(arr1);
  bit_array_free(arr2);
}

void test_product()
{
  printf("== Testing product ==\n\n");

  _test_product(1, 1);
  _test_product(0, 1);
  _test_product(0, 0);
  _test_product(2, 2);
  _test_product(3, 9);

  BIT_ARRAY *arr1 = bit_array_create(0);
  BIT_ARRAY *arr2 = bit_array_create(0);

  bit_array_resize(arr1, 200);
  bit_array_set_region(arr1, 0, 150);

  bit_array_add(arr2, 12);

  printf("a: ");
  bit_array_print_substr(arr1, 0, bit_array_length(arr1), stdout, '1', '0', 0);
  printf("\n");
  printf("b: ");
  bit_array_print_substr(arr2, 0, bit_array_length(arr2), stdout, '1', '0', 0);
  printf("\n");

  bit_array_product(arr1, arr1, arr2);
  printf(" = ");
  bit_array_print_substr(arr1, 0, bit_array_length(arr1), stdout, '1', '0', 0);
  printf("\n");

  bit_array_free(arr1);
  bit_array_free(arr2);

  printf("== End of testing product ==\n\n");
}

void test_crc()
{
  printf("== Testing CRC ==\n\n");

  printf("== End of testing CRC ==\n\n");
}

int main(int argc, char* argv[])
{
  if(argc != 1)
  {
    printf("  Unused args '%s..'\n", argv[1]);
    printf("Usage: ./bit_array_test\n");
    exit(EXIT_FAILURE);
  }

  printf("  Example operations on bit_array C library:\n\n");

  // Test functions
  test_arithmetic();
  test_first_last_bit_set();
  test_zero_length_arrays();
  test_parity();
  test_interleave();
  test_compare();
  test_compare2();
  test_hash();
  test_reverse();
  test_toggle();
  test_next_permutation();
  test_shuffle();
  test_random();
  test_cycle();
  test_shift();
  test_hamming_weight();
  test_save_load();
  test_nums();
  test_hex_functions();
  test_add_word();
  test_multiply();
  test_string_functions();
  test_add_words();
  test_product();

  // To do
  //test_crc();

  //test_multiple_actions();

  printf(" THE END.\n");
  
  return EXIT_SUCCESS;
}
