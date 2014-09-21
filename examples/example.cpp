/*
 examples/example.cpp
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 maintainer: Isaac Turner <turner.isaac@gmail.com>
 license: Public Domain, no warranty
 date: Sep 2014
*/

#include <iostream>
#include "bit_array.h"

int main(int argc, char* argv[])
{
  using namespace std;

  if(argc > 1)
  {
    cout << "  Unused args '" << argv[1] << "..'\n";
    cout << "Usage: " << argv[0] << "\n";
    return -1;
  }

  BIT_ARRAY *bitarr = bit_array_create(10);
  bit_array_print(bitarr, stdout);
  cout << "\n";

  bit_array_set_bits(bitarr, 3, 1,2,5);
  bit_array_print(bitarr, stdout);
  cout << "\n";

  return 0;
}
