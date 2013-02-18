ifndef CC
	CC = gcc
endif

ifdef DEBUG
	CFLAGS := -DDEBUG=1 --debug -g
else
	CFLAGS := -O3
endif

CFLAGS := $(CFLAGS) -Wall -Wextra

all: bit_array_test bit_matrix_test bit_array_generate

bit_array_test: bit_array_test.c
	$(CC) $(CFLAGS) -I.. -L.. -o bit_array_test bit_array_test.c -lbitarr

bit_array_generate:
	$(CC) $(CFLAGS) -o bit_array_generate bit_array_generate.c

clean:
	rm -rf  bit_array_test bit_array_generate bitarr_example.dump *.o *.dSYM *.greg

.PHONY: all clean
