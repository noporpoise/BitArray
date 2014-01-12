ifndef CC
	CC = gcc
endif

ifdef DEBUG
	OPT = -DDEBUG=1 --debug -g
else
	OPT = -O3
endif

CFLAGS = -Wall -Wextra -Wc++-compat

all: bit_array_test bit_array_generate

bit_array_test: bit_array_test.c ../libbitarr.a
	$(CC) $(OPT) $(CFLAGS) -I.. -L.. -o bit_array_test bit_array_test.c -lbitarr

bit_array_generate:
	$(CC) $(OPT) $(CFLAGS) -o bit_array_generate bit_array_generate.c

clean:
	rm -rf  bit_array_test bit_array_generate bitarr_example.dump *.o *.dSYM *.greg

.PHONY: all clean
