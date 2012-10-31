/*
 bit_array_test.c
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
#include "bit_array.h"

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
  bit_array_increment(arr1);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Decrement: arr1--\n");
  bit_array_decrement(arr1);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Add: arr1 = arr1 + arr2\n");
  bit_array_add(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Subtract: arr1 = arr1 - arr2\n");
  bit_array_subtract(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Subtract: arr1 = arr1 - arr1\n");
  bit_array_subtract(arr1, arr1, arr1);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Add: arr1 = arr1 + arr2\n");
  bit_array_add(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Add: arr1 = arr1 + arr2\n");
  bit_array_add(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  bit_array_free(arr1);
  bit_array_free(arr2);

  printf("== End of testing arithmetic ==\n\n");
}

void test_complement_region()
{
  printf("== testing complement_region ==\n");

  char tmp[101];
  BIT_ARRAY* arr = bit_array_create(100);

  printf("complement 0,100:\n");
  bit_array_complement_region(arr,0,100);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));

  printf("complement 0,0:\n");
  bit_array_complement_region(arr,0,0);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));

  printf("complement 1,1:\n");
  bit_array_complement_region(arr,1,1);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));

  printf("complement 3,1:\n");
  bit_array_complement_region(arr,3,1);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));

  printf("complement 20,80:\n");
  bit_array_complement_region(arr,20,80);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));

  printf("complement 0,64:\n");
  bit_array_complement_region(arr,0,64);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));

  printf("complement 64,26:\n");
  bit_array_complement_region(arr,64,36);
  printf("arr: %s\n", bit_array_to_str(arr, tmp));

  bit_array_free(arr);

  printf("== End of testing complement_region ==\n\n");
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

  /*
  // Test cycle bits when it's ready

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
  */

  /*
  // Test reverse when it's ready

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
  */

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
  bitarr = bit_array_load(f);
  fclose(f);

  bit_array_print(bitarr, stdout);
  printf("\n");

  // Deconstruct bit array
  bit_array_free(bitarr);

  printf("== End of testing all functions ==\n\n");
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

  test_complement_region();
  test_arithmetic();
  test_first_last_bit_set();
  test_zero_length_arrays();
  test_parity();
  //test_multiple_actions();

  printf(" THE END.\n");
  
  return EXIT_SUCCESS;
}
