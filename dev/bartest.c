
/*

  bartest.c
	test new features of bitarr library.
		- resize from 0
		- get/set/clear/toggle/assign auto-resize
		- copy auto-resize
		- copy_all function (with auto-resize)
		- extended shift left
		- find clear bit
	exercise short names. (some of them anyway)
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bit_array.h"
#include "bar.h"


// these are the globals for testing.
bar bars[7];
bar foo;
bar bar1, bar2, bar3;
bar *barp = NULL;
bar *barps[12];

int main(int argc, char* argv[])
{
	int i, j, k;
	void *rp;
	int rv;
	uint64_t res, w;
	int tb;	// test bit.

	// happy little test of memset.  Expect dire consequences.
	rp = memset(NULL, 1, 0);

	// make sure this works, too.
	w = 0x00000FFFFFFFFF00;
	rv = leading_zeros(w);
// printf("clz=%d\n", rv);
	assert(rv == 20);
	rv = trailing_zeros(w);
	assert(rv == 8);
	rv = __builtin_ctzll(w);
	assert(rv == 8);
	rv = __builtin_clzll(w);
	assert(rv == 20);
	w = 0xFFFFFFFFFFFFFFFF;
	w >>= 5;
	rv = __builtin_clzll(w);
	assert(rv == 5);

	// initialize to zeros.
	memset((void *)bars, 0, sizeof(bars));
	memset((void *)&foo, 0, sizeof(bar));

	// test resize from nothing.
	assert(barlen(&foo) == 0);
	rv = bit_array_resize(&foo, 23);
	assert(rv == 1);
	assert(barlen(&foo) == 23);
	barfree(&foo);
	assert(barlen(&foo) == 0);

	// run through get/set/clear/toggle/assign.
	// start with 0, increase size each time.

	tb = 7;
	assert(barlen(&foo) == 0);
	rv = barget(&foo, tb);
	assert(rv == 0);
	assert(barlen(&foo) >= tb);
	tb *= 4;
	barset(&foo, tb);
	assert(barlen(&foo) >= tb);
	assert(barget(&foo, tb) == 1);
	tb *= 4;
	barclr(&foo, tb);
	assert(barlen(&foo) >= tb);
	assert(barget(&foo, tb) == 0);
	tb *= 4;
	barflip(&foo, tb);
	assert(barlen(&foo) >= tb);
	assert(barget(&foo, tb) == 1);
	tb *= 4;
	barmake(&foo, tb, 1);
	assert(barlen(&foo) >= tb);
	assert(barget(&foo, tb) == 1);
	tb *= 4;

	for (i = 1; i < 5; i++) {
		barcpy(&(bars[i]), &foo);
		assert(barlen(&(bars[i])) == barlen(&foo));
		rv = barlen( &(bars[i]));
		rv += 57;
		barset( &(bars[i]), rv);
		assert(barlen(&(bars[i])) >= rv);
		barcpy(&foo, &(bars[i]));
		assert(barlen(&foo) >= rv);
	}

	barfree(&foo);
	assert(barlen(&foo) == 0);

	barset(&foo, 100);
	barfill(&foo);
	rv = barffz(&foo, &res);
	assert(rv == 0);

	barclr(&foo, 50);
	rv = barffz(&foo, &res);
	assert(rv == 1);
	assert(res == 50);
	barclr(&foo, 77);
	rv = barfnz(&foo, res+1, &res);
	assert(rv == 1);
	assert(res == 77);

	barfree(&foo);
	assert(barlen(&foo) == 0);

	barset(&foo, 1);
	barset(&foo, 3);
	barset(&foo, 15);
	assert(barlen(&foo) >= 15);

	j = 9;
	k = barlen(&foo);
	for (i = 0; i < 10; i++) {
		bareshl(&foo, j, 0);
		k += j;
		j *= 2;
	}
	assert(barlen(&foo) >= k);

	printf("bartest passed\n");
	return 0;
}
