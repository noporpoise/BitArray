/*
 bar.h
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 maintainer: Isaac Turner <turner.isaac@gmail.com>
 license: Public Domain, no warranty
 date: Sept 2014
*/

// shorten the names of some of the bit_array functions to be more
// like the str* function names.  The prefix "bar" is used to represent
// bit_array and is analogous to "str".

#ifndef BAR_HEADER_SEEN
#define BAR_HEADER_SEEN

#include "bit_array.h"

#define bar BIT_ARRAY

#define barcreate  bit_array_create
#define bardestroy bit_array_free
#define baralloc   bit_array_alloc
#define barfree    bit_array_dealloc
#define barlen     bit_array_length

#define barsize    bit_array_resize
#define barcap     bit_array_ensure_size

// These five are MACROs
#define barget     bit_array_get
#define barset     bit_array_set
#define barclr     bit_array_clear
#define barflip    bit_array_toggle
#define barmake    bit_array_assign

/* Functions instead of macros bars* => s for safe */
#define barsget    bit_array_get_bit
#define barsset    bit_array_set_bit
#define barsclr    bit_array_clear_bit
#define barsflip   bit_array_toggle_bit
#define barsmake   bit_array_assign_bit

/* "resize" functions barr*: automatically enlarge array if needed */
#define barrget    bit_array_rget
#define barrset    bit_array_rset
#define barrclr    bit_array_rclear
#define barrflip   bit_array_rtoggle
#define barrmake   bit_array_rassign

#define barsetn    bit_array_set_bits
#define barclrn    bit_array_clear_bits
#define barflipn   bit_array_toggle_bits

#define barsetr    bit_array_set_region
#define barclrr    bit_array_clear_region
#define barflipr   bit_array_toggle_region

#define barfill    bit_array_set_all
#define barzero    bit_array_clear_all
#define bartogl    bit_array_toggle_all

/* gw "get word" */
#define bargw64    bit_array_get_word64
#define bargw32    bit_array_get_word32
#define bargw16    bit_array_get_word16
#define bargw8     bit_array_get_word8
#define bargwn     bit_array_get_wordn

/* sw "set word" */
#define barsw64    bit_array_set_word64
#define barsw32    bit_array_set_word32
#define barsw16    bit_array_set_word16
#define barswn     bit_array_set_wordn

#define barncpy    bit_array_copy
#define barcpy     bit_array_copy_all
#define bardup     bit_array_clone

#define barpopc    bit_array_num_bits_set
#define barzeros   bit_array_num_bits_cleared
#define bardist    bit_array_hamming_distance
#define barparity  bit_array_parity

#define barfns     bit_array_find_next_set_bit
#define barfps     bit_array_find_prev_set_bit
#define barffs     bit_array_find_first_set_bit
#define barfls     bit_array_find_last_set_bit

#define barfnz     bit_array_find_next_clear_bit
#define barfpz     bit_array_find_prev_clear_bit
#define barffz     bit_array_find_first_clear_bit
#define barflz     bit_array_find_last_clear_bit

#define barsort    bit_array_sort_bits
#define barsortr   bit_array_sort_bits_rev

#define barand     bit_array_and
#define baror      bit_array_or
#define barxor     bit_array_xor
#define barnot     bit_array_not

#define barcmp     bit_array_cmp
#define barcmpbe   bit_array_cmp_big_endian
#define barcmpw    bit_array_cmp_words
#define barcmp64   bit_array_cmp_uint64

#define barshr     bit_array_shift_right
#define barshl     bit_array_shift_left
#define bareshl    bit_array_shift_left_extend

#define barcycr    bit_array_cycle_right
#define barcycl    bit_array_cycle_left

#define barmix     bit_array_interleave

#define barrev     bit_array_reverse
#define barrevr    bit_array_reverse_region

#define bar2num    bit_array_as_num

/* Add/sub/mult/div a bit array with: */
/*   _i unsigned integer, _si shifted integer, _sb shifted bitarray */
#define baraddi    bit_array_add_uint64
#define baraddsi   bit_array_add_word
#define baraddsb   bit_array_add_words
#define barsubi    bit_array_sub_uint64
#define barsubsi   bit_array_sub_word
#define barsubsb   bit_array_sub_words
#define barmuli    bit_array_mul_uint64
#define bardivi    bit_array_div_uint64

/* arguments are both bit arrays */
#define baradd     bit_array_add
#define barsub     bit_array_subtract
#define barmul     bit_array_multiply
#define bardiv     bit_array_divide

#define barsave    bit_array_save
#define barload    bit_array_load

#define barhash    bit_array_hash

#define barrand    bit_array_random
#define barshfl    bit_array_shuffle
#define barperm    bit_array_next_permutation

#endif /* BAR_HEADER_SEEN */
