/*
 bit_macros.h
 project: bit array C library
 url: https://github.com/noporpoise/BitArray/
 author: Isaac Turner <turner.isaac@gmail.com>
 license: Public Domain, no warranty
 date: Dec 2013
*/

#ifndef BITSET_H_
#define BITSET_H_

#include <inttypes.h>
#include <sched.h>

// trailing_zeros is number of least significant zeros
// leading_zeros is number of most significant zeros
#if defined(_WIN32)
  #define trailing_zeros(x) ({ __typeof(x) _r; _BitScanReverse64(&_r, x); _r; })
  #define leading_zeros(x) ({ __typeof(x) _r; _BitScanForward64(&_r, x); _r; })
#else
  #define trailing_zeros(x) ((x) ? (__typeof(x))__builtin_ctzl(x) : (__typeof(x))sizeof(x)*8)
  #define leading_zeros(x) ((x) ? (__typeof(x))__builtin_clzl(x) : (__typeof(x))sizeof(x)*8)
#endif

// Get index of top set bit. If x is 0 return nbits
#define top_set_bit(x) ((x) ? sizeof(x)*8-leading_zeros(x)-1 : sizeof(x)*8)

#define roundup_bits2bytes(bits)   (((bits)+7)/8)
#define roundup_bits2words32(bits) (((bits)+31)/32)
#define roundup_bits2words64(bits) (((bits)+63)/64)

// Round a number up to the nearest number that is a power of two
#define roundup2pow(x) (1UL << (64 - leading_zeros(x)))

#define rot32(x,r) (((x)<<(r)) | ((x)>>(32-(r))))
#define rot64(x,r) (((x)<<(r)) | ((x)>>(64-(r))))

// need to check for length == 0, undefined behaviour if uint64_t >> 64 etc
#define bitmask(nbits,type) ((nbits) ? ~(type)0 >> (sizeof(type)*8-(nbits)): (type)0)
#define bitmask32(nbits) bitmask(nbits,uint32_t)
#define bitmask64(nbits) bitmask(nbits,uint64_t)

// A possibly faster way to combine two words with a mask
//#define bitmask_merge(a,b,abits) ((a & abits) | (b & ~abits))
#define bitmask_merge(a,b,abits) (b ^ ((a ^ b) & abits))

// Swap lowest four bits. A nibble is 4 bits (i.e. half a byte)
#define rev_nibble(x) ((((x)&1)<<3)|(((x)&2)<<1)|(((x)&4)>>1)|(((x)&8)>>3))

//
// Bit array (bitset)
//
// bitsetX_wrd(): get word for a given position
// bitsetX_idx(): get index within word for a given position
#define _TYP(x) (__typeof(*(x)))
#define _VTYPTR(x) (volatile __typeof(*(x)) *)

#define bitsetX_wrd(wrdbits,pos) ((pos) / (wrdbits))
#define bitsetX_idx(wrdbits,pos) ((pos) % (wrdbits))

#define bitset32_wrd(pos) ((pos) >> 5)
#define bitset32_idx(pos) ((pos) & 31)

#define bitset64_wrd(pos) ((pos) >> 6)
#define bitset64_idx(pos) ((pos) & 63)

#define bitset_wrd(arr,pos) ((pos) / (sizeof(*(arr))*8))
#define bitset_idx(arr,pos) ((pos) % (sizeof(*(arr))*8))

#define bitset2_get(arr,wrd,idx)     (((arr)[wrd] >> (idx)) & 0x1)
#define bitset2_set(arr,wrd,idx)     ((arr)[wrd] |= (_TYP(arr)1 << (idx)))
#define bitset2_del(arr,wrd,idx)     ((arr)[wrd] &=~(_TYP(arr)1 << (idx)))
#define bitset2_tgl(arr,wrd,idx)     ((arr)[wrd] ^=~(_TYP(arr)1 << (idx)))
#define bitset2_cpy(arr,wrd,idx,bit) \
   ((arr)[wrd] = (((arr)[wrd] &~ _TYP(arr)(_TYP(arr)1 << (idx))) | \
                 _TYP(arr)(_TYP(arr)(bit) << (idx))))

#define bitset_op(func,arr,pos) func(arr, bitset_wrd(arr,pos), bitset_idx(arr,pos))

#define bitset_get(arr,pos) bitset_op(bitset2_get, arr, pos)
#define bitset_set(arr,pos) bitset_op(bitset2_set, arr, pos)
#define bitset_del(arr,pos) bitset_op(bitset2_del, arr, pos)
#define bitset_tgl(arr,pos) bitset_op(bitset2_tgl, arr, pos)
#define bitset_cpy(arr,pos,bit) \
        bitset2_cpy(arr, bitset_wrd(arr,pos), bitset_idx(arr,pos), (bit))

#define bitset_clear_word(arr,pos) ((arr)[bitset_wrd(arr,pos)] = 0)

//
// Thread safe versions
//
#define bitset2_set_mt(arr,wrd,idx) \
  __sync_or_and_fetch(&(arr)[wrd], _TYP(arr)(_TYP(arr)1<<(idx)))
#define bitset2_del_mt(arr,wrd,idx) \
  __sync_and_and_fetch(&(arr)[wrd], ~_TYP(arr)(_TYP(arr)1<<(idx)))
#define bitset2_tgl_mt(arr,wrd,idx) \
  __sync_xor_and_fetch(&(arr)[wrd], ~_TYP(arr)(_TYP(arr)1<<(idx)))
#define bitset2_cpy_mt(arr,wrd,idx,bit) \
        ((bit) ? bitset2_set_mt(arr,wrd,idx) : bitset2_del_mt(arr,wrd,idx))

#define bitset_set_mt(arr,pos) bitset_op(bitset2_set_mt, arr, pos)
#define bitset_del_mt(arr,pos) bitset_op(bitset2_del_mt, arr, pos)
#define bitset_tgl_mt(arr,pos) bitset_op(bitset2_tgl_mt, arr, pos)
#define bitset_cpy_mt(arr,pos,bit) \
        bitset2_cpy_mt(arr, bitset_wrd(arr,pos), bitset_idx(arr,pos), (bit))

// The following do not need atomics
#define bitset_get_mt(arr,pos)        bitset_get(arr,pos)
#define bitset2_get_mt(arr,wrd,idx)   bitset2_get(arr,wrd,idx)
#define bitset_clear_word_mt(arr,pos) bitset_clear_word(arr,pos)

//
// Compact bit array of spin locks
// These are most effecient when arr is of type: volatile char*
//
// Acquire a lock
#define bitlock_acquire_block(arr,pos,wait) {                                  \
  size_t _w = bitset_wrd(arr,pos);                                             \
  __typeof(*(arr)) _o, _n, _b = _TYP(arr)(_TYP(arr)1 << bitset_idx(arr,pos));  \
  do {                                                                         \
    while((arr)[_w] & _b) { wait }                                             \
    _o = (arr)[_w] & ~_b; _n = (arr)[_w] | _b;                                 \
  } while(!__sync_bool_compare_and_swap(_VTYPTR(arr)&(arr)[_w], _o, _n));     \
  __sync_synchronize(); /* Must not move commands to before acquiring lock */  \
}

// Undefined behaviour if you do not already hold the lock
#define bitlock_release(arr,pos) {                                             \
  size_t _w = bitset_wrd(arr,pos);                                             \
  __typeof(*(arr)) _o, _b = _TYP(arr)(_TYP(arr)1 << bitset_idx(arr,pos));      \
  __sync_synchronize(); /* Must get the lock before releasing it */            \
  do { _o = (arr)[_w]; }                                                       \
  while(!__sync_bool_compare_and_swap(_VTYPTR(arr)&(arr)[_w], _o, _o & ~_b)); \
}

#define bitlock_acquire(arr,pos) bitlock_acquire_block(arr,pos,{})

// calls yield if cannot acquire the lock
#define bitlock_yield_acquire(arr,pos) bitlock_acquire_block(arr,pos,sched_yield();)

#endif /* BITLOCK_H_ */
