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

int main(int argc, char* argv[])
{
  printf("Oh hi!\n");
  printf("  Example operations on bit_array C library:\n");

  BIT_ARRAY* bitarr;
  char* str;
  
  // construct bit array
  bitarr = bit_array_create(100);
  
  int step = 0;

  str = bit_array_to_string(bitarr);
  printf("%i) Create bit array 100 bits long\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_set_bit(bitarr, 2);
  str = bit_array_to_string(bitarr);
  printf("%i) set bit 2\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_set_bit(bitarr, 5);
  str = bit_array_to_string(bitarr);
  printf("%i) set bit 5\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_set_bit(bitarr, 99);
  str = bit_array_to_string(bitarr);
  printf("%i) set bit 99\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_set_bit(bitarr, 0);
  str = bit_array_to_string(bitarr);
  printf("%i) set bit 0\n", step);
  printf("'%s'\n", str);
  free(str);
  
  /* Test clone */
  BIT_ARRAY* clone = bit_array_clone(bitarr);
  str = bit_array_to_string(clone);
  printf("%i.1) clone\n", step);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_clear_bit(clone, 0);
  str = bit_array_to_string(clone);
  printf("%i.2) clear bit 0\n", step);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_set_bit(clone, 21);
  str = bit_array_to_string(clone);
  printf("%i.3) set bit 21\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_free(clone);
  /* End of clone */
  
  bit_array_fill_zeros(bitarr);
  str = bit_array_to_string(bitarr);
  printf("%i) fill with zeros\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_fill_ones(bitarr);
  str = bit_array_to_string(bitarr);
  printf("%i) fill with ones\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_clear_bit(bitarr, 1);
  str = bit_array_to_string(bitarr);
  printf("%i) clear bit 1\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_clear_bit(bitarr, 98);
  str = bit_array_to_string(bitarr);
  printf("%i) clear bit 98\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_clear_bit(bitarr, 99);
  str = bit_array_to_string(bitarr);
  printf("%i) clear bit 99\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_resize(bitarr, 40);
  str = bit_array_to_string(bitarr);
  printf("%i) resize to 40 bits\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_resize(bitarr, 100);
  str = bit_array_to_string(bitarr);
  printf("%i) resize to 100 bits\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_fill_ones(bitarr);
  str = bit_array_to_string(bitarr);
  printf("%i) fill with ones\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_resize(bitarr, 64);
  str = bit_array_to_string(bitarr);
  printf("%i) resize to 64 bits\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_resize(bitarr, 128);
  str = bit_array_to_string(bitarr);
  printf("%i) resize to 128 bits\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  // Deconstruct bit array
  bit_array_free(bitarr);
  
  printf(" THE END.\n");
  
  return EXIT_SUCCESS;
}
