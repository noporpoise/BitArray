/*
 examples/example.c
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 maintainer: Isaac Turner <turner.isaac@gmail.com>
 license: Public Domain, no warranty
 date: Sep 2014
*/

#include <stdlib.h>
#include <stdio.h>
#include "bit_array.h"

int main(int argc, char* argv[])
{
  if(argc > 1)
  {
    printf("Unused args '%s..'\n", argv[1]);
    printf("Usage: ./%s\n", argv[0]);
    return EXIT_FAILURE;
  }

  BIT_ARRAY *bitarr = bit_array_create(10);
  bit_array_print(bitarr, stdout);
  fputc('\n', stdout);

  bit_array_set_bits(bitarr, 3, 1,2,5);
  bit_array_print(bitarr, stdout);
  fputc('\n', stdout);

  return EXIT_SUCCESS;
}
