ifndef CC
	CC = gcc
endif

ifdef DEBUG
	OPT = -DDEBUG=1 --debug -g
else
	OPT = -O3 -flto
endif

CFLAGS = -Wall -Wextra -Wc++-compat -I.

all: libbitarr.a dev/bit_array_test examples

bit_array.o: bit_array.c bit_array.h bit_macros.h

libbitarr.a: bit_array.o
	ar -csru libbitarr.a bit_array.o

%.o: %.c %.h
	$(CC) $(OPT) $(CFLAGS) -fPIC -c $< -o $@

dev/bit_array_test: libbitarr.a
	cd dev; make

examples: libbitarr.a
	cd examples; make

test: dev/bit_array_test
	./dev/bit_array_test

clean:
	rm -rf libbitarr.a *.o *.dSYM *.greg
	cd dev; make clean
	cd examples; make clean

# Comment this line out to keep .o files
.INTERMEDIATE: $(OBJS)
.PHONY: all clean test examples
