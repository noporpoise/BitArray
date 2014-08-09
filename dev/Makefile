ifndef CC
	CC = gcc
endif

ifdef DEBUG
	OPT = -DDEBUG=1 --debug -g
else
	OPT = -O3
endif

CFLAGS = -Wall -Wextra -Wc++-compat

all: bit_array_test bitlock_test bitlock_try_test bit_array_generate bartest

bit_array_test: bit_array_test.c ../libbitarr.a
	$(CC) $(OPT) $(CFLAGS) -I.. -L.. -o bit_array_test bit_array_test.c -lbitarr

bartest:	bartest.c  ../libbitarr.a ../bar.h
	$(CC) $(OPT) $(CFLAGS) -I.. -L.. -o bartest bartest.c -lbitarr

bitlock_test: bitlock_test.c ../bit_macros.h
	$(CC) $(OPT) $(CFLAGS) -I.. -o bitlock_test bitlock_test.c -lpthread

bitlock_try_test: bitlock_try_test.c ../bit_macros.h
	$(CC) $(OPT) $(CFLAGS) -I.. -o bitlock_try_test bitlock_try_test.c -lpthread

bit_array_generate:
	$(CC) $(OPT) $(CFLAGS) -o bit_array_generate bit_array_generate.c


../libbitarr.a:
	cd .. && make

test: bit_array_test bitlock_test bartest
	./bit_array_test && ./bitlock_test
	./bartest

clean:
	rm -rf  bit_array_test bitlock_test bit_array_generate
	rm -rf bitarr_example.dump *.o *.dSYM *.greg

.PHONY: all clean test
