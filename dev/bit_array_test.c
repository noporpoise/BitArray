/*
 dev/bit_array_test.c
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 maintainer: Isaac Turner <turner.isaac@gmail.com>
 license: Public Domain, no warranty
 date: Sep 2014
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <time.h> // needed for rand()
#include <unistd.h>  // need for getpid() for getting setting rand number
#include "bit_array.h"

// Constants
const char test_filename[] = "bitarr_example.dump";

#define MAX(x,y) ((x) >= (y) ? (x) : (y))
#define MIN(x,y) ((x) <= (y) ? (x) : (y))

#define RAND(x) (((x) * (long)rand())/RAND_MAX)

//
// Tests
//
const char *suite_name;
char suite_pass;
int suites_run = 0, suites_failed = 0, suites_empty = 0;
int tests_in_suite = 0, tests_run = 0, tests_failed = 0;

#define QUOTE(str) #str
#define ASSERT(x) {tests_run++; tests_in_suite++; if(!(x)) \
  { fprintf(stderr, "failed assert [%s:%i] %s\n", __FILE__, __LINE__, QUOTE(x)); \
    suite_pass = 0; tests_failed++; }}

void SUITE_START(const char *name)
{
  suite_pass = 1;
  suite_name = name;
  suites_run++;
  tests_in_suite = 0;
}

void SUITE_END()
{
  printf("Testing %s ", suite_name);
  size_t suite_i;
  for(suite_i = strlen(suite_name); suite_i < 80-8-5; suite_i++) printf(".");
  printf("%s\n", suite_pass ? " pass" : " fail");
  if(!suite_pass) suites_failed++;
  if(!tests_in_suite) suites_empty++;
}

//
// Utility functions
//

#define die(fmt,...) do { \
  fprintf(stderr, "[%s:%i] Error: %s() "fmt"\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
  exit(EXIT_FAILURE); \
} while(0);

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

void _test_copy(BIT_ARRAY *arr2, bit_index_t to,
                BIT_ARRAY *arr1, bit_index_t from,
                bit_index_t len)
{
  char *str1 = (char*)malloc((bit_array_length(arr1)+1) * sizeof(char));
  char *corr = (char*)malloc((bit_array_length(arr2)+to+len+1) * sizeof(char));

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

  char *str2 = (char*)malloc((bit_array_length(arr2)+1) * sizeof(char));
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

// Fetch bits start..end from arr into setbits, then check that the return
// result is correct. `setbits` must be at least (end-start) elements long
void _get_bits(const BIT_ARRAY *arr, bit_index_t start, bit_index_t end,
               bit_index_t *setbits)
{
  bit_index_t i, j, n = bit_array_get_bits(arr, start, end, setbits);
  for(i = start, j = 0; i < end; i++) {
    if(bit_array_get(arr,i)) {
      ASSERT(setbits[j] == i);
      j++;
    }
  }
  ASSERT(j == n);
}

// Test function bit_array_get_bits(arr,start,end,indices)
void test_get_bits()
{
  bit_index_t *setbits = (bit_index_t *) calloc(200, sizeof(*setbits));
  BIT_ARRAY *arr = bit_array_create(200);
  bit_array_random(arr, 0.5f);
  _get_bits(arr, 0, 0, setbits);
  _get_bits(arr, 0, 200, setbits);
  _get_bits(arr, 200, 200, setbits);
  _get_bits(arr, 50, 150, setbits);
  free(setbits);
  bit_array_free(arr);
  SUITE_END();
}

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
  bit_array_add_uint64(arr1, 1);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Decrement: arr1--\n");
  bit_array_sub_uint64(arr1, 1);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Sum: arr1 = arr1 + arr2\n");
  bit_array_add(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Difference: arr1 = arr1 - arr2\n");
  bit_array_subtract(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Difference: arr1 = arr1 - arr1\n");
  bit_array_subtract(arr1, arr1, arr1);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Sum: arr1 = arr1 + arr2\n");
  bit_array_add(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  printf("Sum: arr1 = arr1 + arr2\n");
  bit_array_add(arr1, arr1, arr2);
  printf("arr1: %s\n", bit_array_to_str(arr1, tmp));

  bit_array_free(arr1);
  bit_array_free(arr2);

  printf("== End of testing arithmetic ==\n\n");
}

void _check_first_last_bit_set(BIT_ARRAY* arr, char not_zero,
                               bit_index_t first, bit_index_t last)
{
  bit_index_t first_bit = 0, last_bit = 0;

  char bit_set0 = bit_array_find_first_set_bit(arr, &first_bit);
  char bit_set1 = bit_array_find_last_set_bit(arr,  &last_bit);

  ASSERT(not_zero == bit_set0);
  ASSERT(not_zero == bit_set1);

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

  BIT_ARRAY *arr = bit_array_create(100);
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

// Check finding set/clear bits return the same results
bit_index_t _test_find_next_bit(BIT_ARRAY *arr, bit_index_t offset,
                                bit_index_t *result)
{
  bit_index_t pos1 = 0, pos2 = 0;
  char found1, found2;
  found1 = bit_array_find_next_set_bit(arr, offset, &pos1);
  bit_array_toggle_all(arr);
  found2 = bit_array_find_next_clear_bit(arr, offset, &pos2);
  bit_array_toggle_all(arr);

  // printf("next (%i,%zu) vs (%i,%zu)\n", (int)found1, (size_t)pos1,
  //                                       (int)found2, (size_t)pos2);
  ASSERT(found1  == found2);
  ASSERT(pos1 == pos2);
  *result = pos1;
  return found1;
}

// Check finding set/clear bits return the same results
bit_index_t _test_find_prev_bit(BIT_ARRAY *arr, bit_index_t offset,
                                bit_index_t *result)
{
  bit_index_t pos1 = 0, pos2 = 0;
  char found1, found2;
  found1 = bit_array_find_prev_set_bit(arr, offset, &pos1);
  bit_array_toggle_all(arr);
  found2 = bit_array_find_prev_clear_bit(arr, offset, &pos2);
  bit_array_toggle_all(arr);

  // printf("prev (%i,%zu) vs (%i,%zu)\n", (int)found1, (size_t)pos1,
  //                                       (int)found2, (size_t)pos2);
  ASSERT(found1  == found2);
  ASSERT(pos1 == pos2);
  *result = pos1;
  return found1;
}

void test_next_prev_bit_set()
{
  SUITE_START("next/prev bit set");

  BIT_ARRAY *arr = bit_array_create(100);
  bit_index_t pos = 0;
  char found;

  // Simple test
  // Nothing set
  found = _test_find_next_bit(arr, 0, &pos);
  ASSERT(!found && pos == 0);
  found = _test_find_next_bit(arr, 50, &pos);
  ASSERT(!found && pos == 0);
  found = _test_find_next_bit(arr, 99, &pos);
  ASSERT(!found && pos == 0);
  found = _test_find_prev_bit(arr, 0, &pos);
  ASSERT(!found && pos == 0);
  found = _test_find_prev_bit(arr, 50, &pos);
  ASSERT(!found && pos == 0);
  found = _test_find_prev_bit(arr, 100, &pos);
  ASSERT(!found && pos == 0);

  bit_array_set_bit(arr, 0);
  found = _test_find_prev_bit(arr, 0, &pos);
  ASSERT(!found && pos == 0);
  found = _test_find_next_bit(arr, 0, &pos);
  ASSERT(found && pos == 0);

  bit_array_set_bit(arr, 99);
  found = _test_find_prev_bit(arr, 99, &pos);
  ASSERT(found && pos == 0);
  found = _test_find_next_bit(arr, 99, &pos);
  ASSERT(found && pos == 99);

  bit_array_set_bits(arr, 3, 10, 20, 64);
  found = _test_find_prev_bit(arr, 99, &pos);
  ASSERT(found && pos == 64);
  found = _test_find_prev_bit(arr, 64, &pos);
  ASSERT(found && pos == 20);
  found = _test_find_next_bit(arr, 1, &pos);
  ASSERT(found && pos == 10);
  found = _test_find_next_bit(arr, 11, &pos);
  ASSERT(found && pos == 20);

  // Automated test
  bit_index_t indices[] = {0, 1, 2, 5, 24, 50, 51, 64, 80, 99};
  size_t i, n, num_idx = sizeof(indices)/sizeof(indices[0]);

  // Loop over setting diff numbers of bits - check next_bit_set
  for(n = 0; n <= num_idx; n++) {
    bit_array_clear_all(arr);
    for(i = 0; i < n; i++) bit_array_set_bit(arr, indices[i]);

    for(i = 0; i < n; i++) {
      found = _test_find_next_bit(arr, indices[i], &pos);
      ASSERT(found == 1);
      ASSERT(pos == indices[i]);

      // Check from bit after index if in range
      if(indices[i]+1 < arr->num_of_bits) {
        found = _test_find_next_bit(arr, indices[i]+1, &pos);
        if(i+1 < n) {
          ASSERT(found == 1);
          ASSERT(pos == indices[i+1]);
        } else {
          ASSERT(found == 0);
        }
      }
    }
  }

  // Loop over setting diff numbers of bits - check prev_bit_set
  for(n = 0; n <= num_idx; n++) {
    bit_array_clear_all(arr);
    for(i = 0; i < n; i++) bit_array_set_bit(arr, indices[num_idx-i-1]);

    for(i = 0; i < n; i++) {
      found = _test_find_prev_bit(arr, indices[num_idx-i-1], &pos);
      if(i+1 < n) {
        ASSERT(found == 1);
        ASSERT(pos == indices[num_idx-i-2]);
      } else {
        ASSERT(found == 0);
      }
    }
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
  char *result_str = (char*)malloc((result_len+1) * sizeof(char));
  bit_array_to_str(result, result_str);

  size_t len1 = bit_array_length(arr1);
  size_t len2 = bit_array_length(arr2);

  char *correct = (char*)malloc(result_len+1);
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

int cmp_strings(const char *str1, const char *str2, char rev)
{
  size_t len1 = strlen(str1);
  size_t len2 = strlen(str2);
  size_t max_len = MAX(len1, len2);
  
  if(rev)
  {
    size_t i;
    for(i = 0; i < max_len; i++)
    {
      char a = (i < len1 ? str1[i] : '0');
      char b = (i < len2 ? str2[i] : '0');

      if(a != b)
      {
        return a > b ? 1 : -1;
      }
    }
  }
  else
  {
    size_t i;
    for(i = max_len; i > 0; i--)
    {
      char a = (i < len1 ? str1[i] : '0');
      char b = (i < len2 ? str2[i] : '0');

      if(a != b)
      {
        return str1[i] > str2[i] ? 1 : -1;
      }
      else if(i == 0)
      {
        break;
      }
    }
  }

  return len1 > len2 ? 1 : (len1 < len2 ? -1 : 0);
}

void _test_cmp(const char *str1, const char *str2, char rev)
{
  // Get correct answer
  int cmp_correct = cmp_strings(str1, str2, rev);

  BIT_ARRAY* arr1 = bit_array_create(0);
  BIT_ARRAY* arr2 = bit_array_create(0);

  bit_array_from_str(arr1, str1);
  bit_array_from_str(arr2, str2);

  int cmp1, cmp2;

  if(rev)
  {
    cmp1 = bit_array_cmp_big_endian(arr1, arr2);
    cmp2 = bit_array_cmp_big_endian(arr2, arr1);
  }
  else
  {
    cmp1 = bit_array_cmp(arr1, arr2);
    cmp2 = bit_array_cmp(arr2, arr1);
  }

  cmp1 = (cmp1 < 0 ? -1 : (cmp1 > 0 ? 1 : 0));
  ASSERT(cmp1 == cmp_correct);

  cmp2 = (cmp2 < 0 ? -1 : (cmp2 > 0 ? 1 : 0));
  ASSERT(cmp2 == -cmp_correct);

  bit_array_free(arr1);
  bit_array_free(arr2);
}

void _test_cmps(char rev)
{
  // Remember right hand side is msb
  _test_cmp("011010100", "001101010", rev);
  _test_cmp("0", "00", rev);
  _test_cmp("", "", rev);
  _test_cmp("100000", "10", rev);
  _test_cmp("11000000000000000000000000000000000000000000010000000000000000"
                "00000000000000000000000000000000001000000000000000000000000000"
                "00000000000000000000000100000000000000000000000000000000000000"
                "000000000000100000", "1000000000000000000000000000000000000000"
                "00000001000000000000000000000000000000000000000000000000001000"
                "00000000000000000000000000000000000000000000000100000000000000"
                "00000000000000000000000000000000000010000000000000000000000000"
                "0000000000000000000000000100000", 1);

  /*
  // Some random tests
  size_t i, j;
  char str1[1000], str2[1000];

  for(i = 0; i < 20; i++)
  {
    size_t len1 = RAND(600);
    size_t len2 = RAND(600);

    for(j = 0; j < len1; j++)
    {
      str1[j] = rand() > RAND_MAX / 2 ? '1' : '0';
    }
    str1[len1] = '\0';
    
    for(j = 0; j < len2; j++)
    {
      str2[j] = rand() > RAND_MAX / 2 ? '1' : '0';
    }
    str2[len2] = '\0';

    _test_cmp(str1, str2, rev);
  }*/
}

void test_compare()
{
  SUITE_START("compare");

  _test_cmps(0);

  SUITE_END();
}

void test_compare2()
{
  SUITE_START("compare (rev endian)");

  _test_cmps(1);

  SUITE_END();
}

/*
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
*/

void _test_reverse(int len)
{
  char str[1000], str2[1000];

  BIT_ARRAY* arr = bit_array_create(len);

  bit_array_random(arr, 0.5f);

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
  char *str1 = (char*)malloc((len+1) * sizeof(char));
  bit_array_to_str(arr, str1);

  // Toggle string
  size_t i;
  for(i = start; i < start+region_len; i++)
  {
    str1[i] = (str1[i] == '1' ? '0' : '1');
  }

  // Toggle array
  char *str2 = (char*)malloc((len+1) * sizeof(char));
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
    bit_array_random(arr, 0.5f);
    _test_toggle_region(arr, i / 2, i - i / 2);
  }

  bit_array_free(arr);

  SUITE_END();
}

void _test_random_and_shuffle()
{
  int i, j, k, maxi_sum = 0;
  BIT_ARRAY *arr = bit_array_create(0);

  int max_len = 500;
  int num_hits = 50;
  int num_repetitions = 50;
  float prob = 0.5f;

  int *counts = (int*)malloc(max_len*sizeof(int));
  int *hist = (int*)malloc((num_hits+1)*sizeof(int));

  for(k = 0; k < num_repetitions; k++)
  {
    int len = RAND(max_len-5)+5;
    ASSERT(len <= max_len);

    bit_array_resize(arr, len);
    bit_array_random(arr, prob);

    bit_index_t bitset = bit_array_num_bits_set(arr);

    memset(counts, 0, len*sizeof(int));
    memset(hist, 0, (num_hits+1)*sizeof(int));

    for(i = 0; i < num_hits; i++)
    {
      bit_array_shuffle(arr);

      for(j = 0; j < len; j++)
        if(bit_array_get_bit(arr, j))
          counts[j]++;
    }

    ASSERT(bitset == bit_array_num_bits_set(arr));

    // Get the most common number of times to be hit
    for(i = 0; i < len; i++) {
      hist[counts[i]]++;
    }

    int maxi = 0;
    for(i = 1; i <= num_hits; i++) {
      if(hist[i] > hist[maxi]) {
        maxi = i;
      }
    }

    maxi_sum += maxi;
  }

  double average = (double)maxi_sum / num_repetitions;
  double expected = num_hits * (double)prob;
  ASSERT(average >= expected-10 && average <= expected+10);

  free(hist);
  free(counts);
  bit_array_free(arr);
}

void test_random_and_shuffle()
{
  SUITE_START("random and shuffle");

  int i;
  for(i = 0; i < 100; i++)
    _test_random_and_shuffle();

  SUITE_END();
}

/*
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
*/

void _test_cycle(BIT_ARRAY *arr, size_t dist, char left)
{
  BIT_ARRAY *clone = bit_array_clone(arr);

  size_t len = bit_array_length(arr);
 
  if(len > 0)
    dist = dist % len;

  char *str1 = (char*)malloc((len+1) * sizeof(char));
  char *str2 = (char*)malloc((len+1) * sizeof(char));
  str1[len] = '\0';
  str2[len] = '\0';

  // cycle in str
  if(left)
  {
    // shift away from index zero
    if(len > 0)
    {
      bit_array_to_substr(arr, 0, len-dist, str1, '1', '0', 0);
      bit_array_to_substr(arr, len-dist, dist, str1+len-dist, '1', '0', 0);
    }

    bit_array_cycle_left(arr, dist);
  }
  else
  {
    // shift towards index zero
    if(len > 0)
    {
      bit_array_to_substr(arr, dist, len-dist, str1+dist, '1', '0', 0);
      bit_array_to_substr(arr, 0, dist, str1, '1', '0', 0);
    }

    bit_array_cycle_right(arr, dist);
  }

  bit_array_to_substr(arr, 0, len, str2, '1', '0', 0);

  // printf("arr : %s\n", str2);
  // printf("str : %s\n", str1);

  // Assert that string shifting is the same as bit array shifting
  ASSERT(strcmp(str1, str2) == 0);

  free(str1);
  free(str2);

  // Now test if we cycle back it is identical to its clone
  if(left)
  {
    bit_array_cycle_right(arr, dist);
  }
  else
  {
    bit_array_cycle_left(arr, dist);
  }

  ASSERT(bit_array_cmp(arr, clone) == 0);

  bit_array_free(clone);
}

// Test cyclic shift / circular shift
void test_cycle()
{
  SUITE_START("cycle");

  BIT_ARRAY* arr = bit_array_create(0);

  _test_cycle(arr, 3, 1);
  _test_cycle(arr, 0, 0);

  bit_array_resize(arr, 10);
  bit_array_set_bits(arr, 4, 2,3,6,9);

  _test_cycle(arr, 3, 1);
  _test_cycle(arr, 0, 0);
  _test_cycle(arr, 3, 1);
  _test_cycle(arr, 0, 0);
  _test_cycle(arr, 25, 1);
  _test_cycle(arr, 25, 0);

  bit_array_resize(arr, 80);
  bit_array_set_bits(arr, 8, 10, 12, 28, 32, 39, 63, 64, 79);

  _test_cycle(arr, 65, 1);
  _test_cycle(arr, 65, 0);

  // Set even bits
  int i;
  bit_array_clear_all(arr);
  for(i = 0; i < 80; i += 2)
  {
    bit_array_set_bit(arr, i);
  }

  bit_array_cycle_left(arr, 1);
  bit_array_cycle_right(arr, 1);

  _test_cycle(arr, 1, 1);
  _test_cycle(arr, 1, 0);
  _test_cycle(arr, 1, 0);
  _test_cycle(arr, 1, 1);

  // Random arrays
  for(i = 0; i < 10; i++)
  {
    size_t len = RAND(1000UL);
    size_t dist = RAND(1000UL);
    char left = rand() > RAND_MAX / 2 ? 1 : 0;

    bit_array_resize(arr, len);
    bit_array_random(arr, 0.5f);
    _test_cycle(arr, dist, left);
  }

  bit_array_resize(arr, 5);
  bit_array_clear_all(arr);
  bit_array_set_bit(arr, 1);

  bit_array_free(arr);

  SUITE_END();
}

void _test_shift(BIT_ARRAY *arr, size_t dist, char left, char fill)
{
  size_t len = bit_array_length(arr);

  // printf("dist: %i; len: %i; %s; fill: %i\n",
  //        (int)dist, (int)len, left ? "left" : "right", fill);

  char *str1 = (char*)malloc((len+1) * sizeof(char));
  char *str2 = (char*)malloc((len+1) * sizeof(char));
  memset(str1, fill ? '1' : '0', len);
  str1[len] = '\0';
  str2[len] = '\0';

  bit_array_to_substr(arr, 0, len, str2, '1', '0', 0);
  // printf("orig: %s\n", str2);

  // cycle in str
  if(left)
  {
    // left is away from index zero
    if(dist < len)
    {
      // printf("0-%i -> %i\n", (int)(len-dist), (int)dist);
      // printf("str : %s\n", str1);
      bit_array_to_substr(arr, 0, len-dist, str1, '1', '0', 0);
      // printf("str : %s\n", str1);
    }

    bit_array_shift_left(arr, dist, fill);
  }
  else
  {
    if(dist < len)
    {
      bit_array_to_substr(arr, dist, len-dist, str1+dist, '1', '0', 0);
    }

    bit_array_shift_right(arr, dist, fill);
  }

  bit_array_to_substr(arr, 0, len, str2, '1', '0', 0);

  // printf("arr : %s\n", str2);
  // printf("str : %s\n", str1);

  ASSERT(strcmp(str1, str2) == 0);

  free(str1);
  free(str2);
}

// Test shift
void test_shift()
{
  SUITE_START("shift");

  BIT_ARRAY* arr = bit_array_create(0);

  const char left  = 1;
  const char right = 0;
  const char fill_zero = 0;
  const char fill_ones = 1;

  _test_shift(arr, 3, left, fill_ones);
  _test_shift(arr, 0, right, fill_ones);

  bit_array_resize(arr, 10);
  bit_array_set_bits(arr, 4, 2,3,6,9);

  _test_shift(arr, 3, left, fill_zero);
  _test_shift(arr, 0, left, fill_zero);
  _test_shift(arr, 3, right, fill_ones);
  _test_shift(arr, 0, right, fill_ones);
  _test_shift(arr, 25, left, fill_zero);
  _test_shift(arr, 25, right, fill_zero);

  bit_array_resize(arr, 80);
  bit_array_set_bits(arr, 8, 10, 12, 28, 32, 39, 63, 64, 79);
  _test_shift(arr, 65, right, fill_zero);
  _test_shift(arr, 65, left, fill_zero);

  // Set even bits
  bit_array_clear_all(arr);

  int i;
  for(i = 0; i < 80; i += 2)
  {
    bit_array_set_bit(arr, i);
  }
  
  _test_shift(arr, 1, left, fill_ones);
  _test_shift(arr, 1, right, fill_ones);

  bit_array_free(arr);

  SUITE_END();
}

void _test_hamming(BIT_ARRAY *arr1, BIT_ARRAY *arr2)
{
  bit_index_t bits_set1 = 0, bits_set2 = 0, dist = 0;

  bit_index_t len1 = bit_array_length(arr1);
  bit_index_t len2 = bit_array_length(arr2);

  bit_index_t i, max = MAX(len1, len2);
  for(i = 0; i < max; i++)
  {
    char a = i < len1 ? bit_array_get_bit(arr1, i) : 0;
    char b = i < len2 ? bit_array_get_bit(arr2, i) : 0;

    if(a)
      bits_set1++;

    if(b)
      bits_set2++;

    if(a != b)
      dist++;
  }

  ASSERT(bits_set1 == bit_array_num_bits_set(arr1));
  ASSERT(bits_set2 == bit_array_num_bits_set(arr2));
  ASSERT(dist == bit_array_hamming_distance(arr1, arr2));
}

void test_hamming_weight()
{
  SUITE_START("hamming weight");

  BIT_ARRAY* arr1 = bit_array_create(0);
  BIT_ARRAY* arr2 = bit_array_create(0);

  _test_hamming(arr1, arr2);
  _test_hamming(arr2, arr1);

  bit_array_resize(arr1, 10);
  _test_hamming(arr1, arr2);
  _test_hamming(arr2, arr1);

  bit_array_set_bits(arr1, 3, 0, 2, 7);
  _test_hamming(arr1, arr2);
  _test_hamming(arr2, arr1);

  bit_array_resize(arr2, 10);
  _test_hamming(arr1, arr2);
  _test_hamming(arr2, arr1);

  bit_array_set_bits(arr2, 3, 0, 2, 7);
  _test_hamming(arr1, arr2);
  _test_hamming(arr2, arr1);

  bit_array_resize(arr1, 80);
  bit_array_clear_bit(arr1, 2);
  bit_array_set_region(arr1, 50, 20);
  _test_hamming(arr1, arr2);
  _test_hamming(arr2, arr1);

  // random
  int i;
  for(i = 0; i < 10; i++)
  {
    size_t len1 = RAND(1000UL);
    size_t len2 = RAND(1000UL);
    bit_array_resize(arr1, len1);
    bit_array_resize(arr2, len2);
    bit_array_random(arr1, 0.5f);
    bit_array_random(arr2, 0.5f);
    _test_hamming(arr1, arr2);
    _test_hamming(arr2, arr1);
  }

  bit_array_free(arr1);
  bit_array_free(arr2);

  SUITE_END();
}

// Saves arr1 to file, then reloads it into arr2 and compares them
void _test_save_load(BIT_ARRAY *arr1, BIT_ARRAY *arr2)
{
  FILE *f = fopen(test_filename, "w");

  if(f == NULL)
  {
    die("Couldn't open file to write: '%s'", test_filename);
  }

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
    char *tmp = (char*)malloc((len+1) * sizeof(char));

    printf("out: %s\n", bit_array_to_str(arr1, tmp));
    printf("in : %s\n", bit_array_to_str(arr2, tmp));

    free(tmp);
  }
}

void test_save_load()
{
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
  for(i = 0; i < 10; i++)
  {
    bit_array_resize(arr1, RAND(5000UL));
    bit_array_random(arr1, 0.5f);
    _test_save_load(arr1, arr2);
  }

  bit_array_free(arr1);
  bit_array_free(arr2);

  SUITE_END();
}

//
// Aggregate testing
//
/*
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

  // Test clone
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

  // End of clone
  
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
*/

void _test_string_functions(BIT_ARRAY *arr)
{
  size_t len = bit_array_length(arr);

  char *str = (char*)malloc((len+1) * sizeof(char));
  BIT_ARRAY *arr2 = bit_array_create(0);

  // to/from string
  bit_array_to_str(arr, str);
  bit_array_from_str(arr2, str);

  // compare
  ASSERT(bit_array_cmp(arr, arr2) == 0);

  bit_array_resize(arr2, 0);

  // to/from substring
  size_t len1 = len / 2;
  size_t len2 = len - len1;
  bit_array_to_substr(arr, 0, len1, str, '1', '0', 1);
  bit_array_to_substr(arr, len1, len2, str+len1, '1', '0', 1);
  bit_array_from_substr(arr2, 0, str, len1, "1", "0", 1);
  bit_array_from_substr(arr2, len1, str+len1, len2, "1", "0", 1);

  // compare
  ASSERT(bit_array_cmp(arr, arr2) == 0);

  free(str);
  bit_array_free(arr2);
}

// bit_array_from_str(), bit_array_from_substr()
// bit_array_to_str(), bit_array_to_substr()
// bit_array_print(), bit_array_print_substr()
void test_string_functions()
{
  SUITE_START("string functions");

  BIT_ARRAY *arr = bit_array_create(10);
  bit_array_set_bits(arr, 3, 1,5,7);
  _test_string_functions(arr);

  int i;
  for(i = 0; i < 10; i++)
  {
    bit_array_resize(arr, RAND(500UL));
    bit_array_random(arr, 0.5f);
    _test_string_functions(arr);
  }

  bit_array_free(arr);

  SUITE_END();
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
  bit_array_from_hex(arr, offset, hex, strlen(hex));
  bit_array_to_hex(arr, offset, bit_array_length(arr)-offset, tmp, upper);

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
  SUITE_START("hex functions");

  BIT_ARRAY *arr = bit_array_create(0);

  // lowercase then uppercase
  _test_hex_functions(arr, "123456789ABcDeF0", 0, 0, "123456789abcdef0");
  _test_hex_functions(arr, "123456789ABcDeF0", 0, 1, "123456789ABCDEF0");

  _test_hex_functions(arr, "0x123456789ABcDeF0", 0, 0, "0x123456789abcdef0");
  _test_hex_functions(arr, "0x123456789ABcDeF0", 0, 1, "0x123456789ABCDEF0");

  _test_hex_functions(arr, "0x123456789Agadsfasdf", 0, 0, "0x123456789a");

  _test_hex_functions(arr, "0x123456789ABcDeF0", 1, 0, "0x123456789abcdef0");
  _test_hex_functions(arr, "0x123456789ABcDeF0", 40, 0, "0x123456789abcdef0");

  bit_array_free(arr);

  SUITE_END();
}

void _next_permutation_str(char *str, size_t len)
{
  if(len == 0)
    return;

  size_t i, j;

  for(i = 1; i < len; i++)
  {
    if(str[i-1] == '1' && str[i] == '0')
    {
      str[i-1] = '0';
      str[i]   = '1';
      break;
    }
  }

  // Set 0..i (inclusive) to 0
  int bits_set = 0;

  for(j = 0; j < i; j++)
  {
    if(str[j] == '1')
    {
      bits_set++;
    }
  }

  memset(str, '1', bits_set);
  memset(str+bits_set, '0', i - bits_set);
}

void test_next_permutation()
{
  SUITE_START("next permutation");

  BIT_ARRAY *arr = bit_array_create(0);
  char tmp[2000], correct[2000];
  size_t len;

  #define TMPN 12
  size_t lens[TMPN] = {0, 1, 2, 2, 2, 5, 5, 10, 101, 302, 512, 600};
  size_t bset[TMPN] = {0, 1, 0, 1, 2, 0, 1,  1,   1,   2,   2,   0};

  int i, j;
  for(i = 0; i < TMPN; i++)
  {
    len = lens[i];

    bit_array_resize(arr, lens[i]);
    bit_array_clear_all(arr);
    bit_array_set_region(arr, 0, bset[i]);

    for(j = 0; j < 100; j++)
    {
      bit_array_to_str(arr, correct);
      _next_permutation_str(correct, len);
      bit_array_next_permutation(arr);
      bit_array_to_str(arr, tmp);
      ASSERT(strcmp(tmp, correct) == 0);
    }
  }

  bit_array_free(arr);

  SUITE_END();
}

void _test_num_cmp(BIT_ARRAY *arr, uint64_t value, uint64_t num)
{
  int cmp1, cmp2;

  cmp1 = bit_array_cmp_uint64(arr, num);
  cmp1 = cmp1 > 0 ? 1 : (cmp1 < 0 ? -1 : 0);
  cmp2 = value > num ? 1 : (value < num ? -1 : 0);

  ASSERT(cmp1 == cmp2);
}

void _test_as_num_cmp_num(uint64_t value)
{
  BIT_ARRAY *arr = bit_array_create(0);

  bit_array_add_uint64(arr, value);

  // as_num returns 1 on success
  uint64_t num = 0;
  ASSERT(bit_array_as_num(arr, &num) == 1);
  ASSERT(value == num);

  _test_num_cmp(arr, value, value-1);
  _test_num_cmp(arr, value, value);
  _test_num_cmp(arr, value, value+1);

  bit_array_free(arr);
}

// Test bit_array_as_num(), bit_array_cmp_uint64()
void test_as_num_cmp_num()
{
  SUITE_START("as_num and num_cmp");

  _test_as_num_cmp_num(0);
  _test_as_num_cmp_num(10);
  _test_as_num_cmp_num(100000000);
  _test_as_num_cmp_num(ULONG_MAX);

  int i;
  for(i = 0; i < 10; i++)
  {
    _test_as_num_cmp_num(rand());
  }

  SUITE_END();
}

void _test_add_single_word_small(unsigned long init, unsigned long add, int offset)
{
  BIT_ARRAY *arr1 = bit_array_create(0);
  BIT_ARRAY *arr2 = bit_array_create(0);

  uint64_t a, b, result;

  bit_array_add_uint64(arr1, init);
  bit_array_as_num(arr1, &a);

  bit_array_add_uint64(arr2, add);
  if(offset > 0)
  {
    bit_array_resize(arr2, bit_array_length(arr2)+offset);
    bit_array_shift_left(arr2, offset, 0);
  }

  bit_array_as_num(arr2, &b);

  if(b != add << offset)
  {
    fprintf(stderr, "Warning: b != add << offset\n");
  }

  bit_array_add_word(arr1, offset, add);
  bit_array_as_num(arr1, &result);

  ASSERT(a + b == result);

  // printf("  %lu + %lu = %lu [%s]\n",
  //        (unsigned long)a, (unsigned long)b, (unsigned long)result,
  //        (a + b == result ? "Pass" : "Fail"));

  bit_array_free(arr1);
  bit_array_free(arr2);
}

void test_add_single_word()
{
  SUITE_START("add single word");

  _test_add_single_word_small(0, 3, 0);
  _test_add_single_word_small(0, 3, 1);
  _test_add_single_word_small(3, 3, 0);
  _test_add_single_word_small(3, 3, 1);
  _test_add_single_word_small(0, 0, 3);

  //   0111010 [58]
  // + 1010000 [5 << 4]
  _test_add_single_word_small(58, 5, 4);

  _test_add_single_word_small(ULONG_MAX-(1<<4), 1, 4);

  SUITE_END();
}

void _test_minus_single_word()
{
  uint64_t word = rand();
  // char tmp[1000];

  int shift = RAND(1000);
  // int shift = 5;
  shift = MAX(shift, 5);

  BIT_ARRAY *arr = bit_array_create(0);

  // Add word twice, shift left (i.e. * 2^shift)
  bit_array_add_uint64(arr, word);
  bit_array_add_uint64(arr, word);
  bit_array_resize(arr, bit_array_length(arr)+shift);
  bit_array_shift_left(arr, shift, 0);

  // Subtract word shifted left (i.e. word * 2^shift)
  ASSERT(bit_array_sub_word(arr, shift, word) == 1);

  // Shift to the right again (should now be equal to word again)
  bit_array_shift_right(arr, shift, 0);
  
  BIT_ARRAY *arr2 = bit_array_create(0);
  bit_array_add_uint64(arr2, word);

  ASSERT(bit_array_cmp_words(arr, 0, arr2) == 0);

  // Shift to the left and compare with word << shift
  bit_array_shift_left(arr, shift, 0);
  ASSERT(bit_array_cmp_words(arr, shift, arr2) == 0);

  bit_array_free(arr);
  bit_array_free(arr2);
}

void test_minus_single_word()
{
  SUITE_START("minus single word");

  int i;
  for(i = 0; i < 100; i++)
  {
    _test_minus_single_word();
  }

  SUITE_END();
}


void _test_add_words()
{
  BIT_ARRAY *arr1 = bit_array_create(RAND(10));
  BIT_ARRAY *arr2 = bit_array_create(RAND(10));

  bit_array_random(arr1, 0.5f);
  bit_array_random(arr2, 0.5f);

  int shift1 = RAND(10);
  int shift2 = RAND(10);

  BIT_ARRAY *shifted_sum = bit_array_create(0);

  bit_array_add_words(shifted_sum, shift1, arr1);
  bit_array_add_words(shifted_sum, shift2, arr2);

  bit_array_resize(arr1, bit_array_length(arr1)+shift1);
  bit_array_shift_left(arr1, shift1, 0);
  bit_array_resize(arr2, bit_array_length(arr2)+shift2);
  bit_array_shift_left(arr2, shift2, 0);
  bit_array_add(arr1, arr1, arr2); // arr1 = arr1 + arr2

  ASSERT(bit_array_cmp_words(arr1, 0, shifted_sum) == 0);

  bit_array_free(arr1);
  bit_array_free(arr2);
  bit_array_free(shifted_sum);
}

void test_add_words()
{
  SUITE_START("add multiple words");

  int i;
  for(i = 0; i < 20; i++)
    _test_add_words();

  SUITE_END();
}

void _test_minus_words()
{
  // Check that: A<<x + 1<<y - a<<x == 1<<y
  BIT_ARRAY *arr = bit_array_create(RAND(100)+5);

  int shift1 = RAND(100)+5;
  int shift2 = RAND(100)+5;

  BIT_ARRAY *accum = bit_array_clone(arr);
  bit_array_resize(accum, bit_array_length(accum)+MAX(shift1, shift2));
  bit_array_shift_left(accum, shift1, 0);
  // set shift2 to the pos of a bit NOT set
  while(shift2 > 0 && bit_array_get_bit(accum, shift2)) shift2--;
  bit_array_set_bit(accum, shift2);

  bit_array_sub_words(accum, shift1, arr);

  ASSERT(bit_array_get_bit(accum, shift2));
  bit_array_clear_bit(accum, shift2);
  ASSERT(bit_array_cmp_uint64(accum, 0) == 0);

  bit_array_free(accum);
  bit_array_free(arr);
}

void test_minus_words()
{
  SUITE_START("minus multiple words");

  int i;
  for(i = 0; i < 1000; i++)
    _test_minus_words();

  // Check minus word of length 0
  BIT_ARRAY *arr1 = bit_array_create(RAND(100));
  bit_array_random(arr1, 0.5f);
  BIT_ARRAY *arr2 = bit_array_clone(arr1);
  BIT_ARRAY *zero = bit_array_create(0);

  for(i = 0; i < 100; i++)
  {
    bit_array_sub_words(arr1, 0, zero);
    bit_array_sub_words(arr1, RAND(bit_array_length(arr1)), zero);
  }

  ASSERT(bit_array_cmp(arr1, arr2) == 0);
  bit_array_free(arr1);
  bit_array_free(arr2);
  bit_array_free(zero);

  SUITE_END();
}

void _test_multiply_small(uint64_t a, uint64_t b)
{
  BIT_ARRAY *arr = bit_array_create(0);
  uint64_t product;

  bit_array_add_uint64(arr, a);
  bit_array_mul_uint64(arr, b);
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
    bit_array_add_uint64(arr1, a);
    bit_array_add_uint64(arr2, b);

    BIT_ARRAY *result = (i == 0 ? arr1 : arr2);

    bit_array_multiply(result, arr1, arr2);

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
  bit_array_add_uint64(nom_arr, nom);

  uint64_t quotient, rem;

  bit_array_div_uint64(nom_arr, denom, &rem);
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

  bit_array_free(bitarr);
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
      bit_array_random(arr, (float)((double)rand() / RAND_MAX));
    } while(bit_array_num_bits_set(arr) == 0);

    do
    {
      bit_array_random(divisor, (float)((double)rand() / RAND_MAX));
    } while(bit_array_num_bits_set(divisor) == 0);

    while(bit_array_cmp(arr, divisor) < 0)
    {
      bit_array_shift_right(divisor, 1, 0);
    }

    if(bit_array_cmp_uint64(divisor, 1) > 0 && rand() < RAND_MAX / 2)
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
  bit_array_multiply(quotient, quotient, divisor);
  bit_array_add(tmp, tmp, quotient);

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

void _test_add_and_minus_multiple_words()
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

  int offset = RAND(num_digits-1);

  while(bit_array_num_bits_set(small) == 0)
  {
    do
    {
      bit_array_random(big, (float)((double)rand() / RAND_MAX));
    } while(bit_array_num_bits_set(big) == 0);

    do
    {
      bit_array_random(small, (float)((double)rand() / RAND_MAX));
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
  bit_array_sub_words(tmp, offset, small);

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

void test_add_and_minus_multiple_words()
{
  SUITE_START("add/minus multiple words");

  int i;
  for(i = 0; i < 10; i++)
  {
    _test_add_and_minus_multiple_words();
  }

  SUITE_END();
}

void _test_add_and_minus_single_word()
{
  // Rand number between 0-511 inclusive
  int num_digits;

  do
  {
    num_digits = rand() & 0x1ff;
  } while(num_digits == 0);

  // Set
  BIT_ARRAY *arr = bit_array_create(num_digits);

  int offset = RAND(num_digits+2);

  do
  {
    bit_array_random(arr, (float)((double)rand() / RAND_MAX));
  } while(bit_array_num_bits_set(arr) == 0);

  word_t word = rand();

  // Copy
  // tmp = arr
  BIT_ARRAY *tmp = bit_array_clone(arr);

  // tmp += word
  bit_array_add_word(tmp, offset, word);

  // tmp -= word
  bit_array_sub_word(tmp, offset, word);

  // Compare tmp and arr
  ASSERT(bit_array_cmp_words(tmp, 0, arr) == 0);

  bit_array_free(tmp);
  bit_array_free(arr);
}

void test_add_and_minus_single_word()
{
  SUITE_START("add/minus single word");

  int i;
  for(i = 0; i < 10; i++)
  {
    _test_add_and_minus_single_word();
  }

  SUITE_END();
}

void test_get_set_bytes()
{
  SUITE_START("get/set byte");

  #define NBYTES 8
  uint8_t bytes[NBYTES] = {0x55,0x31,0x5A,0x12,0x02,0x31,0xC3,0x1E};
  size_t s, i;

  BIT_ARRAY *arr = bit_array_create(NBYTES*8);

  // Set bytes using different offsets
  for(s = 0; s < 8; s++) {
    bit_array_clear_all(arr);
    for(i = 0; i < NBYTES-1; i++) {
      bit_array_set_word8(arr, i*8+s, bytes[i]);
      ASSERT(bit_array_get_word8(arr, i*8+s) == bytes[i]);
    }
    // Setting bytes doesn't extend the array - check last byte with masking
    i = NBYTES-1;
    bit_array_set_word8(arr, i*8+s, bytes[i]);
    ASSERT(bit_array_get_word8(arr, i*8+s) == (bytes[i] & (0xff>>s)));

    // Check bits
    for(i = 0; i < s; i++) ASSERT(bit_array_get(arr, i) == 0);
    for(i = s; i < NBYTES*8; i++)
      ASSERT(bit_array_get(arr, i) == bitset_get(bytes, i-s));
  }

  bit_array_free(arr);
  #undef NBYTES

  SUITE_END();
}

// test new features of bitarr library.
//   - resize from 0
//   - get/set/clear/toggle/assign auto-resize
//   - copy auto-resize
//   - copy_all function (with auto-resize)
//   - extended shift left
//   - find clear bit
// exercise short names. (some of them anyway)
#include "bar.h"

void test_bar_wrapper()
{
  SUITE_START("testing bar wrapper");

  bar bars[7];
  bar foo;
  int i, j;
  uint64_t res, w;
  bit_index_t tb, rv, k; // test bit.

  // make sure this works, too.
  w = 0x00000FFFFFFFFF00;
  rv = leading_zeros(w);
  // printf("clz=%d\n", rv);
  ASSERT(rv == 20);
  rv = trailing_zeros(w);
  ASSERT(rv == 8);
  rv = __builtin_ctzll(w);
  ASSERT(rv == 8);
  rv = __builtin_clzll(w);
  ASSERT(rv == 20);
  w = 0xFFFFFFFFFFFFFFFF;
  w >>= 5;
  rv = __builtin_clzll(w);
  ASSERT(rv == 5);

  // initialize to zeros.
  memset((void *)bars, 0, sizeof(bars));
  memset((void *)&foo, 0, sizeof(bar));

  // test resize from nothing.
  ASSERT(barlen(&foo) == 0);
  rv = bit_array_resize(&foo, 23);
  ASSERT(rv == 1);
  ASSERT(barlen(&foo) == 23);
  barfree(&foo);
  ASSERT(barlen(&foo) == 0);

  // Auto-resizing array turned off, so these tests fail

  // run through get/set/clear/toggle/assign.
  // start with 0, increase size each time.

  tb = 7;
  ASSERT(barlen(&foo) == 0);
  rv = barrget(&foo, tb);
  ASSERT(rv == 0);
  ASSERT(barlen(&foo) >= tb);
  tb *= 4;
  barrset(&foo, tb);
  ASSERT(barlen(&foo) >= tb);
  ASSERT(barrget(&foo, tb) == 1);
  tb *= 4;
  barrclr(&foo, tb);
  ASSERT(barlen(&foo) >= tb);
  ASSERT(barrget(&foo, tb) == 0);
  tb *= 4;
  barrflip(&foo, tb);
  ASSERT(barlen(&foo) >= tb);
  ASSERT(barrget(&foo, tb) == 1);
  tb *= 4;
  barrmake(&foo, tb, 1);
  ASSERT(barlen(&foo) >= tb);
  ASSERT(barrget(&foo, tb) == 1);
  tb *= 4;

  for (i = 1; i < 5; i++) {
    barcpy(&(bars[i]), &foo);
    ASSERT(barlen(&(bars[i])) == barlen(&foo));
    rv = barlen(&(bars[i]));
    rv += 57;
    barrset(&(bars[i]), rv);
    ASSERT(barlen(&(bars[i])) >= rv);
    barcpy(&foo, &(bars[i]));
    ASSERT(barlen(&foo) >= rv);
  }

  for (i = 1; i < 5; i++) {
    barfree(&(bars[i]));
  }

  barfree(&foo);
  ASSERT(barlen(&foo) == 0);

  barrset(&foo, 100);
  barfill(&foo);
  rv = barffz(&foo, &res);
  ASSERT(rv == 0);

  barrclr(&foo, 50);
  rv = barffz(&foo, &res);
  ASSERT(rv == 1);
  ASSERT(res == 50);
  barclr(&foo, 77);
  rv = barfnz(&foo, res+1, &res);
  ASSERT(rv == 1);
  ASSERT(res == 77);

  barfree(&foo);
  ASSERT(barlen(&foo) == 0);

  barrset(&foo, 1);
  barrset(&foo, 3);
  barrset(&foo, 15);
  ASSERT(barlen(&foo) >= 15);

  j = 9;
  k = barlen(&foo);
  for (i = 0; i < 10; i++) {
    bareshl(&foo, j, 0);
    k += j;
    j *= 2;
  }
  ASSERT(barlen(&foo) >= k);

  barfree(&foo);

  SUITE_END();
}

int main(int argc, char* argv[])
{
  if(argc != 1)
  {
    printf("  Unused args '%s..'\n", argv[1]);
    printf("Usage: ./bit_array_test\n");
    exit(EXIT_FAILURE);
  }

  // Initialise random number generator
  srand((unsigned int)time(NULL) + getpid());

  printf("  Test bit_array C library:\n\n");

  // Test functions
  test_copy();
  test_get_set_bytes();

  test_get_bits();
  test_parity();
  test_interleave();
  test_reverse();
  test_toggle();
  test_cycle();
  test_shift();

  test_compare();
  test_compare2();
  test_first_last_bit_set();
  test_next_prev_bit_set();
  test_hamming_weight();
  test_save_load();

  test_hex_functions();
  test_string_functions();
  test_to_from_decimal();

  test_as_num_cmp_num();

  test_add_single_word();
  test_minus_single_word();

  test_add_words();
  test_minus_words();

  test_add_and_minus_single_word();
  test_add_and_minus_multiple_words();

  test_multiply();
  test_div();
  test_small_products();
  test_product_divide();

  test_bar_wrapper();

  // slooow
  test_next_permutation();
  test_random_and_shuffle();

  // Tests that need re-writing
  // test_hash();

  // To do
  // test_crc();
  // test_multiple_actions();
  // test_zero_length_arrays();
  // test_arithmetic();

  printf("\n");
  printf(" %i / %i suites failed\n", suites_failed, suites_run);
  printf(" %i / %i suites empty\n", suites_empty, suites_run);
  printf(" %i / %i tests failed\n", tests_failed, tests_run);

  printf("\n THE END.\n");
  
  return tests_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
