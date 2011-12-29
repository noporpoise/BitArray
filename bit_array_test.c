/*
 bit_array_test.c
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
  
  /* Test Copy */
  BIT_ARRAY* copy = bit_array_copy(bitarr);
  str = bit_array_to_string(copy);
  printf("%i.1) copy\n", step);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_clear_bit(copy, 0);
  str = bit_array_to_string(copy);
  printf("%i.2) clear bit 0\n", step);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_set_bit(copy, 21);
  str = bit_array_to_string(copy);
  printf("%i.3) set bit 21\n", step++);
  printf("'%s'\n", str);
  free(str);
  
  bit_array_free(copy);
  /* End of copy */
  
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
