/*
 bar.h
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 maintainer: Isaac Turner <turner.isaac@gmail.com>
 license: Public Domain, no warranty
 date: Aug 2014
*/

// shorten the names of some of the bit_array functions to be more
// like the str* function names.  The prefix "bar" is used to represent
// bit_array and is analogous to "str".

#define bar BIT_ARRAY

#define barcreate  bit_array_create
#define bardestroy bit_array_free
#define baralloc   bit_array_alloc
#define barfree    bit_array_dealloc
#define barlen     bit_array_length
#define barset     bit_array_set_bit
#define barclr     bit_array_clear_bit
#define barget     bit_array_get_bit
#define barflip    bit_array_toggle_bit
#define barmake    bit_array_assign_bit

#define barncpy   bit_array_copy
#define barcpy    bit_array_copy_all
#define bardup    bit_array_clone

#define barshr    bit_array_shift_right
#define barshl    bit_array_shift_left
#define bareshl   bit_array_shift_left_extend

#define barpopc   bit_array_num_bits_set

#define barfns    bit_array_find_next_set_bit
#define barfps    bit_array_find_prev_set_bit
#define barffs    bit_array_find_first_set_bit
#define barfls    bit_array_find_last_set_bit

#define barfnz    bit_array_find_next_clear_bit
#define barfpz    bit_array_find_prev_clear_bit
#define barffz    bit_array_find_first_clear_bit
#define barflz    bit_array_find_last_clear_bit

#define barcmp    bit_array_cmp
#define barand    bit_array_and
#define baror     bit_array_or
#define barxor    bit_array_xor
#define barnot    bit_array_not

#define barzero   bit_array_clear_all
#define barfill   bit_array_set_all

