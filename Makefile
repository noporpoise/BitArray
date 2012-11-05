ifndef CC
	CC = gcc
endif

ifdef DEBUG
	CFLAGS := -DDEBUG=1 --debug -g
else
	CFLAGS := -O3
endif

CFLAGS := $(CFLAGS) -Wall -Wextra -I.
LIBINC = -L.

all: clean
	$(CC) $(CFLAGS) -o lookup3.o -c lookup3.c
	$(CC) $(CFLAGS) -o bit_array.o -c bit_array.c
	ar -csru libbitarr.a bit_array.o lookup3.o
	$(CC) $(CFLAGS) $(LIBINC) -o bit_array_test bit_array_test.c -lbitarr
	$(CC) $(CFLAGS) -o bit_array_generate bit_array_generate.c

clean:
	rm -rf  *.o libbitarr.a bit_array_test bit_array_generate *.dSYM *.greg
