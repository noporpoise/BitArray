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

// Constants
const char test_filename[] = "bitarr_example.dump";

#define MAX(x,y) ((x) >= (y) ? (x) : (y))

//
// Tests
//
char *suite_name;
char suite_pass;
int suites_run = 0, suites_failed = 0;

#define QUOTE(str) #str
#define ASSERT(x) if(!(x)) \
  {warn("failed assert [%s:%i] %s", __FILE__, __LINE__, QUOTE(x)); \
   suite_pass = 0;}

#define SUITE_START(x) {suite_pass = 1; suite_name = x; suites_run++;}

#define SUITE_END() printf("Testing %s: %s\n", \
                           suite_name, suite_pass ? "Pass" : "Fail"); \
                    if(!suite_pass) { suites_failed++;}

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

void reverse_str(char *str)
{
  size_t s, len = strlen(str);
  for(s = 0; s < len / 2; s++)
  {
    char tmp = str[s];
    str[s] = str[len-s-1];
    str[len-s-1] = tmp;
  }
}

word_t word_from_str(char *str)
{
  word_t w = 0;
  int i;
  for(i = 0; i < 64 && str[i] != '\0'; i++)
  {
    w <<= 1;
    if(str[i] == '1')
    {
      w |= 1;
    }
  }
  return w;
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
  bit_array_minus(arr1, 1);
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

void _check_first_last_bit_set(BIT_ARRAY* arr, char not_zero,
                               bit_index_t first, bit_index_t last)
{
  bit_index_t first_bit = 0, last_bit = 0;

  char bit_set = bit_array_find_first_set_bit(arr, &first_bit);
  bit_array_find_last_set_bit(arr, &last_bit);

  ASSERT(not_zero == bit_set);

  if(not_zero)
  {
    ASSERT(first_bit == first);
    ASSERT(last_bit == last);
  }
  else
  {
    ASSERT(first_bit == 0);
    ASSERT(last_bit == 0);
  }
}


void test_first_last_bit_set()
{
  SUITE_START("first/last bit set");

  BIT_ARRAY* arr = bit_array_create(100);
  _check_first_last_bit_set(arr, 0, 0, 0);

  bit_array_set_bits(arr, 6, 0, 5, 24, 64, 80, 99);
  _check_first_last_bit_set(arr, 1, 0, 99);

  bit_array_clear_bits(arr, 2, 0, 99);
  _check_first_last_bit_set(arr, 1, 5, 80);

  bit_array_clear_bits(arr, 2, 5, 80);
  _check_first_last_bit_set(arr, 1, 24, 64);

  bit_array_clear_bits(arr, 2, 24, 64);
  _check_first_last_bit_set(arr, 0, 0, 0);

  bit_array_set_bit(arr, 0);
  _check_first_last_bit_set(arr, 1, 0, 0);

  const int len = 9;
  bit_index_t set[] = {0, 1, 31, 62, 63, 64, 65, 98, 99};
  int i;

  for(i = 0; i < len; i++)
  {
    bit_index_t pos = set[i];
    bit_array_clear_all(arr);
    bit_array_set_bit(arr, pos);
    _check_first_last_bit_set(arr, 1, pos, pos);
  }

  bit_array_free(arr);

  SUITE_END();
}

void test_parity()
{
  SUITE_START("parity");

  BIT_ARRAY* arr = bit_array_create(100);

  ASSERT(bit_array_parity(arr) == 0);

  bit_array_set_bits(arr, 6, 0, 5, 24, 64, 80, 99);
  ASSERT(bit_array_parity(arr) == 0);

  bit_array_clear_bit(arr, 24);
  ASSERT(bit_array_parity(arr) == 1);

  bit_array_clear_bit(arr, 99);
  ASSERT(bit_array_parity(arr) == 0);

  bit_array_clear_bit(arr, 0);
  ASSERT(bit_array_parity(arr) == 1);

  bit_array_free(arr);

  SUITE_END();
}

void _test_interleave(BIT_ARRAY* result, BIT_ARRAY *arr1, BIT_ARRAY *arr2)
{
  bit_array_interleave(result, arr1, arr2);

  size_t result_len = bit_array_length(result);
  char *result_str = malloc(result_len+1);
  bit_array_to_str(result, result_str);

  size_t len1 = bit_array_length(arr1);
  size_t len2 = bit_array_length(arr2);

  char *correct = malloc(result_len+1);
  memset(correct, '0', sizeof(char) * result_len);
  correct[result_len] = '\0';

  size_t i, j = 0;
  size_t len = MAX(len1,len2);
  for(i = 0; i < len; i++)
  {
    if(i < len1)
      correct[j++] = bit_array_get_bit(arr1, i) ? '1' : '0';
 
    if(i < len2)
      correct[j++] = bit_array_get_bit(arr2, i) ? '1' : '0';
  }

  ASSERT(strcmp(correct, result_str) == 0);

  if(strcmp(correct, result_str) != 0)
  {
    // debug output
    printf("corr: %s\n", correct);
    printf("got : %s\n", result_str);
  }

  free(correct);
  free(result_str);
}

void test_interleave()
{
  SUITE_START("interleave");

  BIT_ARRAY* arr1 = bit_array_create(10);
  BIT_ARRAY* arr2 = bit_array_create(10);
  BIT_ARRAY* result = bit_array_create(0);

  bit_array_set_all(arr1);
  _test_interleave(result, arr1, arr2);

  bit_array_clear_all(arr1);
  bit_array_set_all(arr2);
  bit_array_resize(result, 25);
  _test_interleave(result, arr1, arr2);

  bit_array_set_all(arr1);
  bit_array_set_all(arr2);
  _test_interleave(result, arr1, arr2);

  bit_array_clear_all(arr1);
  bit_array_clear_all(arr2);
  _test_interleave(result, arr1, arr2);

  bit_array_resize(arr1, 100);
  bit_array_resize(arr2, 100);
  bit_array_clear_all(arr1);
  bit_array_set_all(arr2);
  _test_interleave(result, arr1, arr2);

  bit_array_free(arr1);
  bit_array_free(arr2);
  bit_array_free(result);

  SUITE_END();
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

void _test_reverse(int len)
{
  char str[1000], str2[1000];

  BIT_ARRAY* arr = bit_array_create(len);

  bit_array_random(arr, 0.5);

  bit_array_to_str(arr, str);
  reverse_str(str);

  bit_array_reverse(arr);
  bit_array_to_str(arr, str2);

  ASSERT(strcmp(str, str2) == 0);

  if(strcmp(str, str2) != 0)
  {
    // debug output
    printf("strrev: %s\n", str);
    printf("arrrev: %s\n", str2);
  }

  bit_array_free(arr);
}

void test_reverse()
{
  SUITE_START("reverse");

  _test_reverse(0);
  _test_reverse(10);
  _test_reverse(63);
  _test_reverse(64);
  _test_reverse(65);
  _test_reverse(100);
  _test_reverse(128);
  _test_reverse(506);

  SUITE_END();
}

void _test_toggle_region(BIT_ARRAY *arr, size_t start, size_t region_len)
{
  bit_index_t len = bit_array_length(arr);
  char *str1 = (char*)malloc(len+1);
  bit_array_to_str(arr, str1);

  // Toggle string
  size_t i;
  for(i = start; i < start+region_len; i++)
  {
    str1[i] = (str1[i] == '1' ? '0' : '1');
  }

  // Toggle array
  char *str2 = (char*)malloc(len+1);
  bit_array_toggle_region(arr, start, region_len);
  bit_array_to_str(arr, str2);

  ASSERT(strcmp(str1, str2) == 0);

  free(str1);
  free(str2);
}

void test_toggle()
{
  SUITE_START("toggle");

  BIT_ARRAY* arr = bit_array_create(0);
  _test_toggle_region(arr, 0, 0);

  bit_array_resize(arr, 10);
  bit_array_set_bits(arr, 4, 2,3,6,9);
  _test_toggle_region(arr, 3, 6);
  _test_toggle_region(arr, 0, 10);

  bit_array_resize(arr, 80);
  bit_array_set_bits(arr, 8, 0,20,50,62,64,70,75,79);
  _test_toggle_region(arr, 25, 50);

  bit_array_resize(arr, 100);
  bit_array_clear_all(arr);
  _test_toggle_region(arr, 0, 100);
  _test_toggle_region(arr, 0, 0);
  _test_toggle_region(arr, 1, 1);
  _test_toggle_region(arr, 3, 1);
  _test_toggle_region(arr, 20, 80);
  _test_toggle_region(arr, 0, 64);
  _test_toggle_region(arr, 64, 36);

  int i;
  for(i = 0; i < 1000; i += 50)
  {
    bit_array_resize(arr, i);
    bit_array_random(arr, 0.5);
    _test_toggle_region(arr, i / 2, i - i / 2);
  }

  bit_array_free(arr);

  SUITE_END();
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

// Saves arr1 to file, then reloads it into arr2 and compares them
void _test_save_load(BIT_ARRAY *arr1, BIT_ARRAY *arr2)
{
  FILE *f = fopen(test_filename, "w");

  if(f == NULL)
  {
    die("Couldn't open file to write: '%s'", test_filename);
  }

  // char *tmp = (char*)malloc(sizeof(char) * (bit_array_length(arr1)+1));
  // printf("%s\n", bit_array_to_str(arr1, tmp));
  bit_array_save(arr1, f);
  fclose(f);

  f = fopen(test_filename, "r");

  if(f == NULL)
  {
    die("Couldn't open file to read: '%s'", test_filename);
  }

  if(!bit_array_load(arr2, f))
  {
    die("Load returned warning [file: %s]", test_filename);
  }

  fclose(f);

  ASSERT(bit_array_cmp(arr1, arr2) == 0);

  if(bit_array_cmp(arr1, arr2) != 0 && bit_array_length(arr1) < 500)
  {
    // debug output
    bit_index_t len = MAX(bit_array_length(arr1), bit_array_length(arr2));
    char *tmp = (char*)malloc(sizeof(char) * (len+1));

    printf("out: %s\n", bit_array_to_str(arr1, tmp));
    printf("in : %s\n", bit_array_to_str(arr2, tmp));

    free(tmp);
  }
}

void test_save_load()
{
  // printf("== Testing save / load ==\n");
  SUITE_START("load and save");

  BIT_ARRAY* arr1 = bit_array_create(0);
  BIT_ARRAY* arr2 = bit_array_create(0);

  // Empty array
  _test_save_load(arr1, arr2);

  // Ten 0s
  bit_array_resize(arr1, 10);
  _test_save_load(arr1, arr2);

  // Thousand 1s:
  bit_array_resize(arr1, 1000);
  bit_array_set_all(arr1);
  _test_save_load(arr1, arr2);

  // 1100111010
  bit_array_resize(arr1, 10);
  bit_array_clear_bits(arr1, 4, 2,3,7,9);
  _test_save_load(arr1, arr2);

  // 100 x random lengths and bits
  int i;
  for(i = 0; i < 100; i++)
  {
    bit_array_resize(arr1, (double)rand()*5000 / RAND_MAX);
    bit_array_random(arr1, 0.5);
    _test_save_load(arr1, arr2);
  }

  bit_array_free(arr1);
  bit_array_free(arr2);

  SUITE_END();
  // printf("== End of testing save / load ==\n\n");
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
  FILE* f;

  printf("Saving bitarray in %s\n", test_filename);
  f = fopen(test_filename, "w");
  bit_array_save(bitarr, f);
  fclose(f);

  bit_array_print(bitarr, stdout);
  printf("\n");

  // Deconstruct bit array
  bit_array_free(bitarr);

  printf("Loading bitarray in %s\n", test_filename);
  f = fopen(test_filename, "r");
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

// Convert string of hex to bit array and back, then compare
void _test_hex_functions(BIT_ARRAY *arr, const char* hex, int offset, char upper,
                         const char* correct)
{
  char tmp_buf[1000];
  char *tmp = tmp_buf;

  if(hex[0] == '0' && hex[1] == 'x')
  {
    tmp_buf[0] = '0';
    tmp_buf[1] = 'x';
    tmp = tmp_buf + 2;
  }

  // Reset BIT_ARRAY by resizing to zero
  bit_array_resize(arr, 0);
  // printf(" Hex: %s; offset: %i\n", hex, offset);
  bit_array_from_hex(arr, offset, hex, strlen(hex));
  // printf(" bit_array_to_hex: %s\n", bit_array_to_str(arr, tmp));

  bit_array_to_hex(arr, offset, bit_array_length(arr)-offset, tmp, upper);
  // printf(" bit_array_to_hex: 0x%s\n", tmp);

  // printf(" bit_array_print_hex: 0x");
  // bit_array_print_hex(arr, offset, bit_array_length(arr)-offset, stdout, upper);
  // printf("\n");

  ASSERT(strcmp(tmp_buf, correct) == 0);

  if(strcmp(tmp_buf, correct) != 0)
  {
    printf("in : %s\n", hex);
    printf("out: %s\n", tmp_buf);
    printf("cor: %s\n\n", correct);
  }
}

// bit_array_from_hex(), bit_array_to_hex(), bit_array_print_hex()
void test_hex_functions()
{
//  printf("== Testing hex functions ==\n");
  SUITE_START("hex functions");

  BIT_ARRAY *arr = bit_array_create(0);

  bit_array_resize(arr, 0);
  // lowercase then uppercase
  _test_hex_functions(arr, "123456789ABcDeF0", 0, 0, "123456789abcdef0");
  _test_hex_functions(arr, "123456789ABcDeF0", 0, 1, "123456789ABCDEF0");

  _test_hex_functions(arr, "0x123456789ABcDeF0", 0, 0, "0x123456789abcdef0");
  _test_hex_functions(arr, "0x123456789ABcDeF0", 0, 1, "0x123456789ABCDEF0");

  _test_hex_functions(arr, "0x123456789Agadsfasdf", 0, 0, "0x123456789a");

  _test_hex_functions(arr, "0x123456789ABcDeF0", 1, 0, "0x123456789abcdef0");
  _test_hex_functions(arr, "0x123456789ABcDeF0", 40, 0, "0x123456789abcdef0");

  SUITE_END();
//  printf("== End of testing hex functions ==\n\n");
}

void _test_nums(BIT_ARRAY *arr, char *tmp, unsigned long true_value)
{
  bit_array_add(arr, true_value);
  printf(" bit_array_add(0+%lu): %s\n", true_value, bit_array_to_str(arr, tmp));

  uint64_t num = 0;
  if(!bit_array_as_num(arr, &num))
  {
    printf("Error:%s:%i: failed as_num\n", __FILE__, __LINE__);
  }

  printf(" bit_array_as_num(%lu): %lu\n", true_value, (unsigned long)num);

  if(num != true_value)
  {
    printf("Error:%s%i: num mismatch\n", __FILE__, __LINE__);
  }

  printf("compare to %lu: %i\n", true_value-1,
         bit_array_compare_num(arr, true_value-1));
  printf("compare to %lu: %i\n", true_value,
         bit_array_compare_num(arr, true_value));
  printf("compare to %lu: %i\n", true_value+1,
         bit_array_compare_num(arr, true_value+1));
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
  //bit_array_resize(arr, 0);
  //_test_nums(arr, tmp, 100000000000);
  bit_array_resize(arr, 0);
  _test_nums(arr, tmp, ULONG_MAX);

  bit_array_free(arr);

  printf("== End of testing num functions ==\n\n");
}

void _test_add_word_small(unsigned long init, unsigned long add, int offset)
{
  BIT_ARRAY *arr1 = bit_array_create(0);
  BIT_ARRAY *arr2 = bit_array_create(0);
//  char tmp[100];

  uint64_t a, b, result;

  // printf(" Add %lu, %lu [offset: %i]\n", init, add, offset);
  bit_array_add(arr1, init);
  // printf("  init: %s\n", bit_array_to_str(arr1, tmp));
  bit_array_as_num(arr1, &a);

  bit_array_add(arr2, add);
  if(offset > 0)
  {
    bit_array_resize(arr2, bit_array_length(arr2)+offset);
    bit_array_shift_left(arr2, offset, 0);
  }
  // printf("  add : %s\n", bit_array_to_str(arr2, tmp));
  bit_array_as_num(arr2, &b);

  if(b != add << offset)
  {
    printf("Warning: b != add << offset\n");
  }

  bit_array_add_word(arr1, offset, add);
  // printf("  sum : %s\n", bit_array_to_str(arr1, tmp));
  bit_array_as_num(arr1, &result);

  ASSERT(a + b == result);

  // printf("  %lu + %lu = %lu [%s]\n",
  //        (unsigned long)a, (unsigned long)b, (unsigned long)result,
  //        (a + b == result ? "Pass" : "Fail"));

  bit_array_free(arr1);
  bit_array_free(arr2);
}

void test_add_word()
{
  SUITE_START("add word");

  _test_add_word_small(0, 3, 0);
  _test_add_word_small(0, 3, 1);
  _test_add_word_small(3, 3, 0);
  _test_add_word_small(3, 3, 1);
  _test_add_word_small(0, 0, 3);

  //   0111010 [58]
  // + 1010000 [5 << 4]
  _test_add_word_small(58, 5, 4);

  _test_add_word_small(ULONG_MAX-(1<<4), 1, 4);

  SUITE_END();
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

  BIT_ARRAY *arr1 = bit_array_create(0);
  BIT_ARRAY *arr2 = bit_array_create(0);

  bit_array_resize(arr1, 200);
  bit_array_set_region(arr1, 0, 150);
  bit_array_add(arr2, 100012);

  printf("a: ");
  bit_array_print_substr(arr1, 0, bit_array_length(arr1), stdout, '1', '0', 0);
  printf("\n");
  printf("b: ");
  bit_array_print_substr(arr2, 0, bit_array_length(arr2), stdout, '1', '0', 0);
  printf("\n");


  int offset = 10;
  bit_array_add_words(arr2, offset, arr1);
  printf("b + (a << %i):\n", offset);
  printf(" = ");
  bit_array_print_substr(arr2, 0, bit_array_length(arr2), stdout, '1', '0', 0);
  printf("\n");

  printf("== End of testing add words ==\n\n");
}

void _test_multiply_small(uint64_t a, uint64_t b)
{
  BIT_ARRAY *arr = bit_array_create(0);
  uint64_t product;

  bit_array_add(arr, a);
  bit_array_multiply(arr, b);
  bit_array_as_num(arr, &product);

  ASSERT(a * b == product);

  if(a * b != product)
  {
    // debug output
    printf("%lu * %lu = %lu [expected: %lu]\n", (unsigned long)a,
           (unsigned long)b, (unsigned long)product, (unsigned long)(a * b));
  }

  bit_array_free(arr);
}

void test_multiply()
{
  SUITE_START("small multiply");

  _test_multiply_small(2, 4);
  _test_multiply_small(4, 2);
  _test_multiply_small(3, 6);
  _test_multiply_small(9, 256);
  _test_multiply_small(10, 100);
  _test_multiply_small(0, 0);
  _test_multiply_small(10, 0);
  _test_multiply_small(0, 10);

  SUITE_END();
}

void _test_small_product(uint64_t a, uint64_t b, char expect_overflow)
{
  BIT_ARRAY *arr1 = bit_array_create(0);
  BIT_ARRAY *arr2 = bit_array_create(0);

  int i;
  for(i = 0; i < 2; i++)
  {
    bit_array_resize(arr1, 0);
    bit_array_resize(arr2, 0);
    bit_array_add(arr1, a);
    bit_array_add(arr2, b);

    BIT_ARRAY *result = (i == 0 ? arr1 : arr2);

    bit_array_product(result, arr1, arr2);

    // printf("  = %s\n", bit_array_to_str(result, tmp));

    uint64_t c;
    char overflowed = !bit_array_as_num(result, &c);

    ASSERT(expect_overflow == overflowed);

    if(!expect_overflow)
    {
      ASSERT(a*b == c);
    }

    if(expect_overflow != overflowed || (!expect_overflow && a*b != c))
    {
      // debug output
      printf("expect: %lu * %lu = %lu [overflow: %s]\n",
             (unsigned long)a, (unsigned long)b, (unsigned long)(a*b),
             expect_overflow ? "yes" : "no");
      printf("got: %lu [overflow: %s]\n", (unsigned long)c,
             overflowed ? "yes" : "no");
    }
  }

  bit_array_free(arr1);
  bit_array_free(arr2);
}

void test_small_products()
{
  SUITE_START("product with small numbers");

  _test_small_product(1, 1, 0);
  _test_small_product(0, 1, 0);
  _test_small_product(0, 0, 0);
  _test_small_product(2, 2, 0);
  _test_small_product(3, 9, 0);
  _test_small_product(3, 90000, 0);
  _test_small_product(300000, 90000, 0);
  _test_small_product(123456789123132, 9876543212351234, 1);

  SUITE_END();
}

void _test_div(uint64_t nom, uint64_t denom)
{
  BIT_ARRAY *nom_arr = bit_array_create(0);
  bit_array_add(nom_arr, nom);

  uint64_t quotient, rem;

  bit_array_div(nom_arr, denom, &rem);
  bit_array_as_num(nom_arr, &quotient);

  ASSERT(quotient * denom + rem == nom);

  if(quotient * denom + rem != nom)
  {
    // Debug output
    printf("%lu = %lu * %lu + %lu [%s]\n",
           (unsigned long)nom, (unsigned long)quotient,
           (unsigned long)denom, (unsigned long)rem,
           (quotient * denom + rem == nom ? "Pass" : "Fail"));
  }

  bit_array_free(nom_arr);
}

void test_div()
{
  SUITE_START("bit_array_div");

  _test_div(10, 2);
  _test_div(100, 100);
  _test_div(0, 100);
  _test_div(12, 3);
  _test_div(13, 7);
  _test_div(12, 9);
  _test_div(64, 8);
  _test_div(9, 9);
  _test_div(10000000, 13);

  SUITE_END();
}

void _test_to_from_decimal(char *str)
{
  size_t len = strlen(str);
  char new_str[len+1];
  BIT_ARRAY *bitarr = bit_array_create(0);

  bit_array_from_decimal(bitarr, str);
  bit_array_to_decimal(bitarr, new_str, len+1);

  ASSERT(atoi(str) == atoi(new_str));

  if(atoi(str) != atoi(new_str))
  {
    // debug output
    printf("%s -> %s\n", str, new_str);
  }
}

void test_to_from_decimal()
{
  SUITE_START("to/from decimal");

  _test_to_from_decimal("1");
  _test_to_from_decimal("0");
  _test_to_from_decimal("1234");
  _test_to_from_decimal("5678");
  _test_to_from_decimal("012");

  SUITE_END();
}

void _test_product_divide()
{
  // Rand number between 0-255 inclusive
  int num_digits;

  do
  {
    num_digits = rand() & 255;
  } while(num_digits == 0);

  // Set
  BIT_ARRAY *arr = bit_array_create(num_digits);
  BIT_ARRAY *divisor = bit_array_create(num_digits);

  while(bit_array_num_bits_set(divisor) == 0)
  {
    do
    {
      bit_array_random(arr, (double)rand() / RAND_MAX);
    } while(bit_array_num_bits_set(arr) == 0);

    do
    {
      bit_array_random(divisor, (double)rand() / RAND_MAX);
    } while(bit_array_num_bits_set(divisor) == 0);

    while(bit_array_cmp(arr, divisor) < 0)
    {
      bit_array_shift_right(divisor, 1, 0);
    }

    if(bit_array_compare_num(divisor, 1) > 0 && rand() < RAND_MAX / 2)
    {
      bit_array_shift_right(divisor, 1, 0);
    }
  }

  // Copy
  BIT_ARRAY *tmp = bit_array_clone(arr);
  BIT_ARRAY *quotient = bit_array_create(0);

  // Divide
  bit_array_divide(tmp, quotient, divisor);

  // Multiply and add
  bit_array_product(quotient, quotient, divisor);
  bit_array_sum(tmp, tmp, quotient);

  // Compare
  ASSERT(bit_array_cmp_words(tmp, 0, arr) == 0);
  // int cmp = bit_array_cmp_words(tmp, 0, arr);
  // printf("product & divide: [%s]\n\n", cmp == 0 ? "Pass" : "Fail");

  bit_array_free(tmp);
  bit_array_free(quotient);

  bit_array_free(arr);
  bit_array_free(divisor);
}

void test_product_divide()
{
  SUITE_START("product and divide");

  int i;
  for(i = 0; i < 10; i++)
  {
    _test_product_divide();
  }

  SUITE_END();
}

void _test_add_minus_words()
{
  // Rand number between 0-511 inclusive
  int num_digits;

  do
  {
    num_digits = rand() & 0x1ff;
  } while(num_digits == 0);

  // Set
  BIT_ARRAY *big = bit_array_create(num_digits);
  BIT_ARRAY *small = bit_array_create(num_digits);

  int offset = ((double)rand() / RAND_MAX) * (num_digits-1);

  while(bit_array_num_bits_set(small) == 0)
  {
    do
    {
      bit_array_random(big, (double)rand() / RAND_MAX);
    } while(bit_array_num_bits_set(big) == 0);

    do
    {
      bit_array_random(small, (double)rand() / RAND_MAX);
    } while(bit_array_num_bits_set(small) == 0);

    while(bit_array_cmp_words(big, offset, small) < 0)
    {
      bit_array_shift_right(small, 1, 0);
    }
  }

  // Copy
  // tmp = big
  BIT_ARRAY *tmp = bit_array_clone(big);

  // tmp -= small
  bit_array_minus_words(tmp, offset, small);

  // tmp += small
  bit_array_add_words(tmp, offset, small);

  // Compare tmp and big
  ASSERT(bit_array_cmp_words(tmp, 0, big) == 0);
  // int cmp = bit_array_cmp_words(tmp, 0, big);
  // printf(" add/minus words: [%s]\n\n", cmp == 0 ? "Pass" : "Fail");

  bit_array_free(tmp);
  bit_array_free(big);
  bit_array_free(small);
}

void test_add_minus_words()
{
  SUITE_START("add/minus multiple words");

  int i;
  for(i = 0; i < 10; i++)
  {
    _test_add_minus_words();
  }

  SUITE_END();
}

void _test_add_minus_single_word()
{
  // Rand number between 0-511 inclusive
  int num_digits;

  do
  {
    num_digits = rand() & 0x1ff;
  } while(num_digits == 0);

  // Set
  BIT_ARRAY *arr = bit_array_create(num_digits);

  int offset = ((double)rand() / RAND_MAX) * (num_digits+2);

  do
  {
    bit_array_random(arr, (double)rand() / RAND_MAX);
  } while(bit_array_num_bits_set(arr) == 0);

  word_t word = rand();

  // Copy
  // tmp = arr
  BIT_ARRAY *tmp = bit_array_clone(arr);

  // tmp += word
  bit_array_add_word(tmp, offset, word);

  // tmp -= word
  bit_array_minus_word(tmp, offset, word);

  // Compare tmp and arr
  ASSERT(bit_array_cmp_words(tmp, 0, arr) == 0);

  bit_array_free(tmp);
  bit_array_free(arr);
}

/*
void _test_minus_word_example(char *arr_str, char *word_str, int offset)
{
  word_t wrd = word_from_str(word_str);

  char str[2000];

  BIT_ARRAY *arr = bit_array_create(0);
  bit_array_from_str(arr, arr_str);

  bit_array_reverse(arr);
  bit_array_to_str(arr, str);
  reverse_str(str);

  if(strcmp(str, arr_str) != 0)
  {
    printf("FAIL\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    printf("PASS\n");
  }

  printf("  arr: %s\n", bit_array_to_str_rev(arr, str));

  BIT_ARRAY *orig = bit_array_clone(arr);

  printf(" word: %s\n", bit_array_word2str_rev(&wrd, 64, str));

  printf(" +/- word with offset %i\n", offset);
  bit_array_add_word(arr, offset, wrd);
  printf("+word: %s\n", bit_array_to_str_rev(arr, str));
  bit_array_minus_word(arr, offset, wrd);
  printf("-word: %s\n", bit_array_to_str_rev(arr, str));

  int cmp = bit_array_cmp_words(orig, 0, arr);
  printf(" add/minus single word: [%s]\n\n", cmp == 0 ? "Pass" : "Fail");

  bit_array_free(orig);
  bit_array_free(arr);
}
*/

void test_add_minus_single_word()
{
  SUITE_START("add/minus single word");

  int i;
  for(i = 0; i < 10; i++)
  {
    _test_add_minus_single_word();
  }

  /*
  char *str = "10111101111111111111111111111111111111111111101111111111111111111111111011111011111111111110111111111111101111111111111111111111111111111111111111111111111101111011111111111111111111111011111110110111111101111111110011111011111111111111111111111111110111111111111111111111111111011111101111111111111111111110101111111111111111110110111111111111111111111111111111110111110111111111111111111111111111111111111111111111111111111111111111011101111111111111111111110111111111111011111101110";
  char *wrd = "0000000000000000000000000000000001110101010110010000011001111110";

  _test_minus_word_example(str, wrd, 475);
  */

  SUITE_END();
}

void test_minus_word()
{
  printf("== Testing minus word ==\n\n");

  BIT_ARRAY *arr = bit_array_create(100);
  char tmp[2000];

  bit_array_set_region(arr, 20, 50);
  printf("arr: %s\n", bit_array_to_str_rev(arr, tmp));

  printf("minus 10001111 << 30\n");
  bit_array_minus_word(arr, 30, 0x8f);
  printf("arr: %s\n", bit_array_to_str_rev(arr, tmp));

  printf("minus 101 << 0\n");
  bit_array_minus_word(arr, 0, 0x5);
  printf("arr: %s\n", bit_array_to_str_rev(arr, tmp));

  printf("minus 111 << 62\n");
  bit_array_minus_word(arr, 62, 0x7);
  printf("arr: %s\n", bit_array_to_str_rev(arr, tmp));

  printf("minus 110 << 62\n");
  bit_array_minus_word(arr, 62, 0x6);
  printf("arr: %s\n", bit_array_to_str_rev(arr, tmp));

  bit_array_free(arr);

  printf("\n== End of testing minus word ==\n\n");
}

void _test_copy(BIT_ARRAY *arr2, bit_index_t to,
                BIT_ARRAY *arr1, bit_index_t from,
                bit_index_t len)
{
  char *str1 = (char*)malloc(bit_array_length(arr1)+1);
  char *corr = (char*)malloc(bit_array_length(arr2)+to+len+1);

  bit_array_to_str(arr1, str1);
  bit_array_to_str(arr2, corr);
  size_t arr2_len = strlen(corr);

  memmove(corr+to, str1+from, len * sizeof(char));

  if(to+len > arr2_len)
  {
    corr[to+len] = '\0';
  }

  // do copy
  bit_array_copy(arr2, to, arr1, from, len);

  char *str2 = (char*)malloc(bit_array_length(arr2)+1);
  bit_array_to_str(arr2, str2);

  // compare
  ASSERT(strcmp(str2, corr) == 0);

  if(strcmp(str2, corr) != 0)
  {
    // debug output
    printf("str1: %s\n", str1);
    printf("str2: %s\n", str2);
    printf("corr: %s\n", corr);
  }

  free(str1);
  free(str2);
  free(corr);
}

void test_copy()
{
  SUITE_START("copy");

  BIT_ARRAY *arr = bit_array_create(200);
  bit_array_set_region(arr, 0, 20);

  _test_copy(arr, 30, arr, 0, 15);
  _test_copy(arr, 50, arr, 0, 50);
  _test_copy(arr, 100, arr, 0, 100);

  bit_index_t len = bit_array_length(arr);
  int shift = 3;

  bit_array_resize(arr, len + shift);
  _test_copy(arr, shift, arr, 0, len);
  _test_copy(arr, 0, arr, shift, len);

  bit_array_free(arr);

  SUITE_END();
}

void test_crc()
{
  printf("== Testing CRC ==\n\n");

  /*
  char str[500];
  BIT_ARRAY *arr = bit_array_create(100);
  bit_array_free(arr);
  */

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

  printf("  Test bit_array C library:\n\n");

  // Test functions
  test_copy();
  // test_arithmetic();
  test_first_last_bit_set();
  // test_zero_length_arrays();
  test_parity();
  test_interleave();
  /*test_compare();
  test_compare2();
  test_hash();*/
  test_reverse();
  test_toggle();
  /*test_next_permutation();
  test_shuffle();
  test_random();
  test_cycle();
  test_shift();
  test_hamming_weight();*/
  test_save_load();
  // test_nums();
  test_hex_functions();
  test_add_word();
  test_multiply();
  // test_string_functions();
  test_div();
  test_to_from_decimal();
  test_small_products();
  test_product_divide();
  test_add_minus_words();
  // test_minus_word();
  test_add_minus_single_word();

  // To do
  //test_crc();
  //test_multiple_actions();

  printf("\n");
  printf(" %i / %i suites failed\n", suites_failed, suites_run);

  printf("\n THE END.\n");
  
  return EXIT_SUCCESS;
}
